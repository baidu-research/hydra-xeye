#ifndef VIS_FG_MOTRACK_TRACKER_H
#define VIS_FG_MOTRACK_TRACKER_H
#include <vector>
#include "track.h"
#include "model.h"

class tracker
{
public:
    float max_iou_distance;
    int max_age;
    int n_init;
//    std::vector<std::vector<cv::Point>> _contours;
    float _in_thre;
    float _out_thre;
    int _next_idx;
public:
    std::vector<Track> tracks;
    tracker(/*NearNeighborDisMetric* metric,*/
        float max_cosine_distance, int nn_budget,
        float max_iou_distance, int max_age, int n_init,
 //       std::vector<std::vector<cv::Point>> contours, 
        float in_thre, float out_thre);
    void predict();
    void update(const DETECTIONS& detections);
    typedef DYNAMICM (tracker::* GATED_METRIC_FUNC)(
        std::vector<Track>& tracks,
        const DETECTIONS& dets,
        const std::vector<int>& track_indices,
        const std::vector<int>& detection_indices);
private:
    void _match(const DETECTIONS& detections, TRACKER_MATCHED& res);
    void _initiate_track(const DETECTION_ROW& detection);
public:
    DYNAMICM iou_cost(
        std::vector<Track>& tracks,
        const DETECTIONS& dets,
        const std::vector<int>& track_indices,
        const std::vector<int>& detection_indices);
    Eigen::VectorXf iou(DETECTBOX& bbox,
                        DETECTBOXSS &candidates);
};
#endif // VIS_FG_MOTRACK_TRACKER_H
