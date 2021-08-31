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
#include "matching/model.h"
#include "matching/linear_assignment.h"
using namespace std;


tracker::tracker(/*NearNeighborDisMetric *metric,*/
    float max_cosine_distance, int nn_budget,
    float max_iou_distance, int max_age, int n_init, 
    //std::vector<std::vector<cv::Point>> contours, 
    float in_thre, float out_thre) {
    this->max_iou_distance = max_iou_distance;
    this->max_age = max_age;
    this->n_init = n_init;

    this->tracks.clear();
    this->_next_idx = 1;
   // this->_contours = contours;
    this->_in_thre = in_thre;
    this->_out_thre = out_thre;
}

void tracker::predict()
{
    for(Track& track:tracks) {
        track.predit();
    }
}

void tracker::update(const DETECTIONS &detections)
{
    TRACKER_MATCHED res;
    _match(detections, res);
    vector<MATCH_DATA>& matches = res.matches;
    for(MATCH_DATA& data:matches) {
        int track_idx = data.first;
        int detection_idx = data.second;
        tracks[track_idx].update(detections[detection_idx]);
    }
    vector<int>& unmatched_tracks = res.unmatched_tracks;
    for(int& track_idx:unmatched_tracks) {
        this->tracks[track_idx].mark_missed();
    }
    vector<int>& unmatched_detections = res.unmatched_detections;
    for(int& detection_idx:unmatched_detections) {
        this->_initiate_track(detections[detection_idx]);
    }
    vector<Track>::iterator it;
    for(it = tracks.begin(); it != tracks.end();) {
        if((*it).is_deleted()) it = tracks.erase(it);
        else ++it;
    }
}

void tracker::_match(const DETECTIONS &detections, TRACKER_MATCHED &res)
{
    vector<int> confirmed_tracks;
    vector<int> unconfirmed_tracks;
    int idx = 0;
    for(Track& t:tracks) {
        if(t.is_confirmed()) confirmed_tracks.push_back(idx);
        else unconfirmed_tracks.push_back(idx);
        idx++;
    }
    /*
    TRACKER_MATCHED matcha = linear_assignment::getInstance()->matching_cascade(
                                this, &tracker::gated_matric,
                                this->metric->mating_threshold,
                                this->max_age,
                                this->tracks,
                                detections,
                                confirmed_tracks);
    */
    TRACKER_MATCHED matcha = linear_assignment::getInstance()->matching_cascade(
                                this, &tracker::iou_cost,
                                this->max_iou_distance,
                                this->max_age,
                                this->tracks,
                                detections,
                                confirmed_tracks);
    //printf("ffconfirm %d\n",confirmed_tracks.size());
    //printf("ffunconfirm %d\n",unconfirmed_tracks.size());
    vector<int> iou_track_candidates;
    iou_track_candidates.assign(unconfirmed_tracks.begin(), unconfirmed_tracks.end());
    vector<int>::iterator it;
    for(it = matcha.unmatched_tracks.begin(); it != matcha.unmatched_tracks.end();) {
        int idx = *it;
        if(tracks[idx].time_since_update == 1) { //push into unconfirmed
            iou_track_candidates.push_back(idx);
            it = matcha.unmatched_tracks.erase(it);
            continue;
        }
        ++it;
    }
    TRACKER_MATCHED matchb = linear_assignment::getInstance()->min_cost_matching(
                                this, &tracker::iou_cost,
                                this->max_iou_distance,
                                this->tracks,
                                detections,
                                iou_track_candidates,
                                matcha.unmatched_detections);
    // get result
    res.matches.assign(matcha.matches.begin(), matcha.matches.end());
    res.matches.insert(res.matches.end(), matchb.matches.begin(), matchb.matches.end());
    // unmatched_tracks
    res.unmatched_tracks.assign(
        matcha.unmatched_tracks.begin(),
        matcha.unmatched_tracks.end());
    res.unmatched_tracks.insert(
        res.unmatched_tracks.end(),
        matchb.unmatched_tracks.begin(),
        matchb.unmatched_tracks.end());
    res.unmatched_detections.assign(
        matchb.unmatched_detections.begin(),
        matchb.unmatched_detections.end());
}

void tracker::_initiate_track(const DETECTION_ROW &detection)
{
    DETECTBOX location = detection.tlwh;
    float x_center = detection.tlwh(0) + detection.tlwh(2)*0.5;
    float y_center = detection.tlwh(1) + detection.tlwh(3)*0.5;
    float dis_cl = 1.0;//cv::pointPolygonTest(_contours[0],
                     //                   cv::Point2f(x_center, y_center), false);
    int dis_sign = 1;
    if (dis_cl < 0) dis_sign = -1;

    this->tracks.push_back(Track(location, this->_next_idx, this->n_init,
                                 this->max_age, dis_sign, 0));
    _next_idx += 1;
}

DYNAMICM
tracker::iou_cost(
    std::vector<Track> &tracks,
    const DETECTIONS &dets,
    const std::vector<int>& track_indices,
    const std::vector<int>& detection_indices)
{
    int rows = track_indices.size();
    int cols = detection_indices.size();
    DYNAMICM cost_matrix = Eigen::MatrixXf::Zero(rows, cols);
    for(int i = 0; i < rows; i++) {
        int track_idx = track_indices[i];
        /*
        if(tracks[track_idx].time_since_update > 1) {
            cost_matrix.row(i) = Eigen::RowVectorXf::Constant(cols, INFTY_COST);
            continue;
        }*/
        DETECTBOX bbox = tracks[track_idx].to_tlwh();
        int csize = detection_indices.size();
        DETECTBOXSS candidates(csize, 4);
        for(int k = 0; k < csize; k++) candidates.row(k) = dets[detection_indices[k]].tlwh;
        Eigen::RowVectorXf rowV = (1. - iou(bbox, candidates).array()).matrix().transpose();
        cost_matrix.row(i) = rowV;
    }
    return cost_matrix;
}

Eigen::VectorXf
tracker::iou(DETECTBOX& bbox, DETECTBOXSS& candidates)
{
    float bbox_tl_1 = bbox[0];
    float bbox_tl_2 = bbox[1];
    float bbox_br_1 = bbox[0] + bbox[2];
    float bbox_br_2 = bbox[1] + bbox[3];
    float area_bbox = bbox[2] * bbox[3];

    Eigen::Matrix<float, -1, 2> candidates_tl;
    Eigen::Matrix<float, -1, 2> candidates_br;

    candidates_tl = candidates.leftCols(2) ;
    candidates_br = candidates.rightCols(2) + candidates_tl;

    int size = int(candidates.rows());
    Eigen::VectorXf res(size);
    for(int i = 0; i < size; i++) {
        float tl_1 = std::max(bbox_tl_1, candidates_tl(i, 0));
        float tl_2 = std::max(bbox_tl_2, candidates_tl(i, 1));
        float br_1 = std::min(bbox_br_1, candidates_br(i, 0));
        float br_2 = std::min(bbox_br_2, candidates_br(i, 1));

        float w = br_1 - tl_1;
        w = (w < 0? 0: w);
        float h = br_2 - tl_2;
        h = (h < 0? 0: h);
        float area_intersection = w * h;
        float area_candidates = candidates(i, 2) * candidates(i, 3);
        res[i] = area_intersection/(area_bbox + area_candidates - area_intersection);
    }
    return res;
}

