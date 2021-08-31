#ifndef VIS_FG_MOTRACK_LINEAR_ASSIGNMENT_H
#define VIS_FG_MOTRACK_LINEAR_ASSIGNMENT_H
#include "datatype.h"
#include "tracker.h"

#define INFTY_COST 1e5

class tracker;
//for matching;
class linear_assignment
{
    linear_assignment();
    linear_assignment(const linear_assignment& );
    linear_assignment& operator=(const linear_assignment&);
    static linear_assignment* instance;

public:
    static linear_assignment* getInstance();
    TRACKER_MATCHED matching_cascade(tracker* distance_metric,
                                    tracker::GATED_METRIC_FUNC distance_metric_func,
                                    float max_distance,
                                    int cascade_depth,
                                    std::vector<Track>& tracks,
                                    const DETECTIONS& detections,
                                    std::vector<int> &track_indices,
                                    std::vector<int> detection_indices = std::vector<int>());
    TRACKER_MATCHED min_cost_matching(
        tracker* distance_metric,
        tracker::GATED_METRIC_FUNC distance_metric_func,
        float max_distance,
        std::vector<Track>& tracks,
        const DETECTIONS& detections,
        std::vector<int>& track_indices,
        std::vector<int>& detection_indices);
};

#endif // VIS_FG_MOTRACK_LINEAR_ASSIGNMENT_H
