#!/bin/sh
tar jcvf mot-$1.tar.bz2 \
    /usr/lib/libmot* \
    /usr/lib/libtrack_merge.so \
    mot/build/install/bin/jde.trt \
    mot4j/mot4j.java \
    track_merge/TrackMerge.java \
    README.md