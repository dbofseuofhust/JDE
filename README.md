# JDE
Towards Real-Time Multi-Object Tracking

# 1. 训练步骤

<del>以下操作步骤均以MOT16为例</del>

<del>## 1.1 准备数据集</del>

<del>* 从MOT挑战赛官网下载数据集并解压 <br></del>
<del>wget https://motchallenge.net/data/MOT16.zip -P /data/tseng/dataset/jde <br></del>
<del>cd /data/tseng/dataset/jde <br></del>
<del>unzip MOT16.zip -d MOT16 <br></del>

<del>* 创建MOT16任务的工作区, 并将MOT格式标注文件转换为需要格式的标注文件 <br></del>
<del>git clone https://github.com/CnybTseng/JDE.git <br></del>
<del>cd JDE <br></del>
<del>mkdir -p workspace/mot16-2020-5-29 <br></del>
<del>./tools/split_dataset.sh ./workspace/mot16-2020-5-29 <br></del>
<del>此时workspace/mot16-2020-5-29目录下会生成train.txt <br></del>

## 1.2 从预训练模型导出参数生成JDE初始模型

* 从darknet官网下载darknet53预训练模型 <br>
wget https://pjreddie.com/media/files/darknet53.conv.74 -P ./workspace <br>
python darknet2pytorch.py -pm ./workspace/mot16-2020-5-29/jde.pth \ <br>
    --dataset ./workspace/mot16-2020-5-29 -dm ./workspace/darknet53.conv.74 -lbo <br>
此时workspace/mot16-2020-5-29目录下会生成初始模型jde.pth, 其骨干网已初始化为darnet53的参数 <br>

## 1.3 执行训练脚本

cp ./tools/train.sh ./train.sh <br>
根据需要修改, 然后运行训练脚本 <br>
./train.sh <br>

# 2. 测试
运行类似如下命令执行多目标跟踪 <br>
python tracker.py --img-path /data/tseng/dataset/jde/MOT16/test/MOT16-03/img1 \ <br>
    --model workspace/mot16-2020-5-29/checkpoint/jde-ckpt-049.pth

# 3. 和原始版本的JDE的性能对比(MOT16 train)

# 3.1 darknet based JDE
         MOTA  MOTP  IDF1  IDP   IDR   Rcll  Prcn  TP    FP   FN    MTR   PTR   MLR   MT  PT  ML FAR  FM   FMR   IDSW IDSWR
MOT16-02 44.81 76.14 43.28 54.39 35.94 56.04 84.80  9993 1791  7840 24.07 57.41 18.52  13  31 10 2.98  423  7.55  211  3.8
MOT16-04 73.80 78.82 75.16 83.37 68.43 78.08 95.13 37131 1902 10426 50.60 38.55 10.84  42  32  9 1.81  331  4.24  131  1.7
MOT16-05 71.15 78.66 73.29 79.40 68.06 79.54 92.80  5423  421  1395 48.80 44.80  6.40  61  56  8 0.50  181  2.28  151  1.9
MOT16-09 64.64 79.55 58.42 64.43 53.43 74.95 90.37  3940  420  1317 48.00 52.00  0.00  12  13  0 0.80  152  2.03  122  1.6
MOT16-10 54.70 76.28 55.47 65.29 48.21 65.50 88.69  8068 1029  4250 25.93 68.52  5.56  14  37  3 1.57  618  9.44  301  4.6
MOT16-11 78.22 82.19 70.92 73.91 68.17 85.75 92.97  7867  595  1307 55.07 39.13  5.80  38  27  4 0.66  172  2.01   96  1.1
MOT16-13 43.85 75.14 50.05 65.77 40.39 54.15 88.17  6200  832  5250 26.17 51.40 22.43  28  55 24 1.11  720 13.30  347  6.4
OVERALL  63.65 78.29 64.50 73.84 57.26 71.21 91.84 78622 6990 31785 40.23 48.55 11.22 208 251 58 1.31 2597 36.47 1359 19.1

# 3.2 lighted JDE
         MOTA  MOTP  IDF1  IDP   IDR   Rcll  Prcn  TP    FP   FN    MTR   PTR   MLR   MT  PT  ML FAR  FM   FMR   IDSW IDSWR
MOT16-02 33.81 72.50 36.08 46.89 29.32 48.90 78.21  8721 2430  9112 14.81 61.11 24.07   8  33 13 4.05  478  9.77  261  5.3
MOT16-04 68.80 76.74 70.87 79.55 63.89 74.79 93.13 35568 2625 11989 44.58 38.55 16.87  37  32 14 2.50  453  6.06  223  3.0
MOT16-05 63.96 76.19 71.82 80.51 64.83 73.36 91.11  5002  488  1816 37.60 52.80  9.60  47  66 12 0.58  211  2.88  153  2.1
MOT16-09 61.92 77.66 53.55 62.68 46.74 69.30 92.93  3643  277  1614 36.00 60.00  4.00   9  15  1 0.53  138  1.99  111  1.6
MOT16-10 47.69 74.29 50.69 63.38 42.23 58.69 88.08  7230  978  5088 25.93 57.41 16.67  14  31  9 1.50  560  9.54  378  6.4
MOT16-11 69.13 79.87 70.07 75.36 65.48 78.68 90.55  7218  753  1956 42.03 42.03 15.94  29  29 11 0.84  205  2.61  123  1.6
MOT16-13 31.52 72.19 42.17 55.77 33.90 47.98 78.93  5494 1467  5956 14.95 57.01 28.04  16  61 30 1.96  723 15.07  418  8.7
OVERALL  56.33 75.97 59.89 70.31 52.15 66.01 88.99 72876 9018 37531 30.95 51.64 17.41 160 267 90 1.70 2768 41.94 1667 25.3

......