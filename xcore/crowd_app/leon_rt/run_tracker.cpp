/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "matching/tracker.h"
#include "datatype.h"
#include <algorithm>
#include "time.h"
#include <unistd.h>
#include <vector>
#include <string>
#include <math.h>
#include "define.h"
using std::string;
using std::vector;

tracker* _this_tracker = NULL;
vector<vector<float>> person_bbox;
vector<RESULT_DATA> track_result;

vector<tracker_detail> g_tracker_detail_info;

int run_tracker() {
    // params for tracking
    float _max_iou_dis = 0.8;
    int _max_age = 10;
    int _num_init = 3;
    int _nn_budget = 20;
    float _max_cos_dis = 0.1;
    float _min_conf = 0.3;
    float _nms_max_overlap = 0.45;
    float _cnt_in_thre = 20;
    float _cnt_out_thre = 15;
    float _detection_thre = 0.1;
    if (_this_tracker == NULL){ 
         _this_tracker = new tracker(_max_cos_dis, _nn_budget, _max_iou_dis, _max_age,
                               _num_init, _cnt_in_thre, _cnt_out_thre);
    }
    //    while (1) {// read frame
    // Detect the human
    // vector<vector<int>> person_bbox  [[x,y,w,h,conf],...]
    DETECTIONS detections;
    for (unsigned int idx_bbox = 0; idx_bbox < person_bbox.size(); ++idx_bbox) {
        vector<float> bbox_tmp = person_bbox[idx_bbox];
        DETECTION_ROW tmp_det_row;
        tmp_det_row.tlwh = DETECTBOX(bbox_tmp[0], bbox_tmp[1],
                                     bbox_tmp[2], bbox_tmp[3]);
        tmp_det_row.confidence = bbox_tmp[4]; //float
        detections.push_back(tmp_det_row);
    }
    //if (_min_conf > _detection_thre) {
   //     ModelDetection::getInstance()->dataMoreConf(_min_conf, detections);
   // }
    // NMS
    if (_nms_max_overlap < 1) {
        ModelDetection::getInstance()->dataPreprocessing(_nms_max_overlap, detections);
    }

    // predict the object location through Kalman filter
    _this_tracker->predict();
    // match and update the tracks
    _this_tracker->update(detections);
    track_result.clear();
    g_tracker_detail_info.clear();
    // get the tracking result
    for (Track & track : _this_tracker->tracks) {
        if (!track.is_confirmed() || track.time_since_update >= 1) {
            continue;
        }

        // calculate the distance to the counting line
        DETECTBOX bbox = track.to_tlwh();
        track_result.push_back(std::make_pair(track.track_id, track.to_tlwh()));
    }
 //   }// end while

    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
