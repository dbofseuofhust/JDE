#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <map>
#include <list>
#include <thread>
#include <vector>
#include <ostream>
#include <opencv2/opencv.hpp>

namespace mot {

typedef enum
{
    New = 0,
    Tracked = 1,
    Lost = 2,
    Removed = 3
} TrajectoryState;

class Trajectory;
typedef std::vector<Trajectory> TrajectoryPool;
typedef std::vector<Trajectory>::iterator TrajectoryPoolIterator;
typedef std::vector<Trajectory *>TrajectoryPtrPool;
typedef std::vector<Trajectory *>::iterator TrajectoryPtrPoolIterator;

class TKalmanFilter : public cv::KalmanFilter
{
public:
    TKalmanFilter(void);
    virtual ~TKalmanFilter(void) {}
    virtual void init(const cv::Mat &measurement);
    virtual const cv::Mat &predict();
    virtual const cv::Mat &correct(const cv::Mat &measurement);
    virtual void project(cv::Mat &mean, cv::Mat &covariance) const;
private:
    float std_weight_position;
    float std_weight_velocity;
};

inline TKalmanFilter::TKalmanFilter(void) : cv::KalmanFilter(8, 4, 0, CV_32F)
{
    cv::KalmanFilter::transitionMatrix = cv::Mat::eye(8, 8, CV_32F);
    for (int i = 0; i < 4; ++i)
        cv::KalmanFilter::transitionMatrix.at<float>(i, i + 4) = 1;
    cv::KalmanFilter::measurementMatrix = cv::Mat::eye(4, 8, CV_32F);
    std_weight_position = 1/20.f;
    std_weight_velocity = 1/160.f;
}

class Trajectory : public TKalmanFilter
{
public:
    Trajectory();
    Trajectory(cv::Vec4f &ltrb, float score, const cv::Mat &embedding);
    Trajectory(const Trajectory &other);
    Trajectory &operator=(const Trajectory &rhs);
    virtual ~Trajectory(void) {};
    static int next_id();
    virtual const cv::Mat &predict(void);
    virtual void update(Trajectory &traj, int timestamp, bool update_embedding=true);
    virtual void activate(int timestamp);
    virtual void reactivate(Trajectory &traj, int timestamp, bool newid=false);
    virtual void mark_lost(void);
    virtual void mark_removed(void);
    friend TrajectoryPool operator+(const TrajectoryPool &a, const TrajectoryPool &b);
    friend TrajectoryPool operator+(const TrajectoryPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPool &operator+=(TrajectoryPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPool &operator+=(TrajectoryPool &a, const TrajectoryPool &b);
    friend TrajectoryPtrPool &operator+=(TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPool operator-(const TrajectoryPool &a, const TrajectoryPool &b);
    friend TrajectoryPool &operator-=(TrajectoryPool &a, const TrajectoryPool &b);
    friend TrajectoryPool &operator-=(TrajectoryPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPtrPool &operator-=(TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPtrPool operator+(const TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend TrajectoryPtrPool operator+(const TrajectoryPtrPool &a, TrajectoryPool &b);
    friend TrajectoryPtrPool operator-(const TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend cv::Mat embedding_distance(const TrajectoryPool &a, const TrajectoryPool &b);
    friend cv::Mat embedding_distance(const TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend cv::Mat embedding_distance(const TrajectoryPtrPool &a, const TrajectoryPool &b);
    friend cv::Mat mahalanobis_distance(const TrajectoryPool &a, const TrajectoryPool &b);
    friend cv::Mat mahalanobis_distance(const TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend cv::Mat mahalanobis_distance(const TrajectoryPtrPool &a, const TrajectoryPool &b);
    friend cv::Mat iou_distance(const TrajectoryPool &a, const TrajectoryPool &b);
    friend cv::Mat iou_distance(const TrajectoryPtrPool &a, const TrajectoryPtrPool &b);
    friend cv::Mat iou_distance(const TrajectoryPtrPool &a, const TrajectoryPool &b);
    friend std::ostream& operator<<(std::ostream& os, Trajectory& trajectory);
private:   
    void update_embedding(const cv::Mat &embedding);
public:
    TrajectoryState state;
    cv::Vec4f ltrb;
    cv::Mat smooth_embedding;
    int id;
    bool is_activated;
    int timestamp;
    int starttime;
private:
    static std::map<std::thread::id, int> count;
    static std::map<std::thread::id, std::list<int>> reused_id;
    cv::Vec4f xyah;
    float score;
    cv::Mat current_embedding;
    float eta;
    int length;
};

inline cv::Vec4f ltrb2xyah(cv::Vec4f &ltrb)
{
    cv::Vec4f xyah;
    xyah[0] = (ltrb[0] + ltrb[2]) * 0.5f;
    xyah[1] = (ltrb[1] + ltrb[3]) * 0.5f;
    xyah[3] =  ltrb[3] - ltrb[1];
    xyah[2] = (ltrb[2] - ltrb[0]) / xyah[3];
    return xyah;
}

inline cv::Vec4f xyah2ltrb(cv::Vec4f &xyah)
{
    cv::Vec4f xywh(xyah);
    xywh[2] *= xywh[3];
    cv::Vec4f ltrb;
    ltrb[0] = xywh[0] - xywh[2] / 2;
    ltrb[1] = xywh[1] - xywh[3] / 2;
    ltrb[2] = xywh[0] + xywh[2] / 2;
    ltrb[3] = xywh[1] + xywh[3] / 2;
    return ltrb;
}

inline Trajectory::Trajectory() :
    state(New), ltrb(cv::Vec4f()), smooth_embedding(cv::Mat()), id(0),
    is_activated(false), timestamp(0), starttime(0), score(0), eta(0.9), length(0)
{
}

inline Trajectory::Trajectory(cv::Vec4f &ltrb_, float score_, const cv::Mat &embedding) :
    state(New), ltrb(ltrb_), smooth_embedding(cv::Mat()), id(0),
    is_activated(false), timestamp(0), starttime(0), score(score_), eta(0.9), length(0)
{
    xyah = ltrb2xyah(ltrb);
    update_embedding(embedding);
}

inline Trajectory::Trajectory(const Trajectory &other):
    state(other.state), ltrb(other.ltrb), id(other.id), is_activated(other.is_activated),
    timestamp(other.timestamp), starttime(other.starttime), xyah(other.xyah),
    score(other.score), eta(other.eta), length(other.length)
{
    if (!other.smooth_embedding.empty())
    {
        other.smooth_embedding.copyTo(smooth_embedding);
    }
    else
    {
        smooth_embedding.release();
    }
    if (!other.current_embedding.empty())
    {
        other.current_embedding.copyTo(current_embedding);
    }
    else
    {
        current_embedding.release();
    }
}

inline Trajectory &Trajectory::operator=(const Trajectory &rhs)
{
    this->state = rhs.state;
    this->ltrb = rhs.ltrb;
    rhs.smooth_embedding.copyTo(this->smooth_embedding);
    this->id = rhs.id;
    this->is_activated = rhs.is_activated;
    this->timestamp = rhs.timestamp;
    this->starttime = rhs.starttime; 
    this->xyah = rhs.xyah;
    this->score = rhs.score;
    rhs.current_embedding.copyTo(this->current_embedding);            
    this->eta = rhs.eta;    
    this->length = rhs.length;
    
    return *this;
}

inline int Trajectory::next_id()
{
    std::thread::id tid = std::this_thread::get_id();
    if (reused_id[tid].size() > 0) {
        int rid = reused_id[tid].front();
        reused_id[tid].pop_front();
        return rid;
    }
    ++count[tid];
    return count[tid];
}

inline void Trajectory::mark_lost(void)
{
    state = Lost;
}

inline void Trajectory::mark_removed(void)
{
    state = Removed;
    // std::thread::id tid = std::this_thread::get_id();
    // reused_id[tid].push_back(id);
}

}   // namespace mot

#endif  // TRAJECTORY_H