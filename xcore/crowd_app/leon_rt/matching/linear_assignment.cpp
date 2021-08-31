/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "matching/linear_assignment.h"
#include "thirdPart/hungarianoper.h"


linear_assignment *linear_assignment::instance = NULL;
linear_assignment::linear_assignment()
{
}

linear_assignment *linear_assignment::getInstance()
{
    if(instance == NULL) instance = new linear_assignment();
    return instance;
}

TRACKER_MATCHED
linear_assignment::matching_cascade(
    tracker *distance_metric,
    tracker::GATED_METRIC_FUNC distance_metric_func,
    float max_distance,
    int cascade_depth,
    std::vector<Track> &tracks,
    const DETECTIONS &detections,
    std::vector<int>& track_indices,
    std::vector<int> detection_indices)
{
    TRACKER_MATCHED res;
    for(size_t i = 0; i < detections.size(); i++) {
        detection_indices.push_back(int(i));
    }

    std::vector<int> unmatched_detections;
    unmatched_detections.assign(detection_indices.begin(), detection_indices.end());
    res.matches.clear();
    std::vector<int> track_indices_l;

    std::map<int, int> matches_trackid;
    for(int level = 0; level < cascade_depth; level++) {
        if(unmatched_detections.size() == 0) break; //No detections left;

        track_indices_l.clear();
        for(int k:track_indices) {
            if(tracks[k].time_since_update == 1+level)
                track_indices_l.push_back(k);
        }
        if(track_indices_l.size() == 0) continue; //Nothing to match at this level.

        TRACKER_MATCHED tmp = min_cost_matching(
                                 distance_metric, distance_metric_func,
                                 max_distance, tracks, detections, track_indices_l,
                                 unmatched_detections);
        unmatched_detections.assign(tmp.unmatched_detections.begin(), tmp.unmatched_detections.end());
        for(size_t i = 0; i < tmp.matches.size(); i++) {
            MATCH_DATA pa = tmp.matches[i];
            res.matches.push_back(pa);
            matches_trackid.insert(pa);
        }
    }
    res.unmatched_detections.assign(unmatched_detections.begin(), unmatched_detections.end());
    for(size_t i = 0; i < track_indices.size(); i++) {
        int tid = track_indices[i];
        if(matches_trackid.find(tid) == matches_trackid.end())
            res.unmatched_tracks.push_back(tid);
    }
    return res;
}

TRACKER_MATCHED
linear_assignment::min_cost_matching(tracker *distance_metric,
                                     tracker::GATED_METRIC_FUNC distance_metric_func,
                                     float max_distance,
                                     std::vector<Track> &tracks,
                                     const DETECTIONS &detections,
                                     std::vector<int> &track_indices,
                                     std::vector<int> &detection_indices) {
    TRACKER_MATCHED res;
    if((detection_indices.size() == 0) || (track_indices.size() == 0)) {
        res.matches.clear();
        res.unmatched_tracks.assign(track_indices.begin(), track_indices.end());
        res.unmatched_detections.assign(detection_indices.begin(), detection_indices.end());
        return res;
    }
    DYNAMICM cost_matrix = (distance_metric->*(distance_metric_func))(
                               tracks, detections, track_indices, detection_indices);
    for(int i = 0; i < cost_matrix.rows(); i++) {
        for(int j = 0; j < cost_matrix.cols(); j++) {
            float tmp = cost_matrix(i,j);
            if(tmp > max_distance) cost_matrix(i,j) = max_distance + 1e-5;
        }
    }
    Eigen::Matrix<float, -1, 2, Eigen::RowMajor> indices = HungarianOper::solve(cost_matrix);
    res.matches.clear();
    res.unmatched_tracks.clear();
    res.unmatched_detections.clear();
    for(size_t col = 0; col < detection_indices.size(); col++) {
        bool flag = false;
        for(int i = 0; i < indices.rows(); i++)
            if(indices(i, 1) == col) {
                flag = true;
                break;
            }
        if(flag == false)res.unmatched_detections.push_back(detection_indices[col]);
    }
    for(size_t row = 0; row < track_indices.size(); row++) {
        bool flag = false;
        for(int i = 0; i < indices.rows(); i++)
            if(indices(i, 0) == row) {
                flag = true;
                break;
            }
        if(flag == false) res.unmatched_tracks.push_back(track_indices[row]);
    }
    for(int i = 0; i < indices.rows(); i++) {
        int row = indices(i, 0);
        int col = indices(i, 1);

        int track_idx = track_indices[row];
        int detection_idx = detection_indices[col];
        if(cost_matrix(row, col) > max_distance) {
            res.unmatched_tracks.push_back(track_idx);
            res.unmatched_detections.push_back(detection_idx);
        } else res.matches.push_back(std::make_pair(track_idx, detection_idx));
    }
    return res;
}

