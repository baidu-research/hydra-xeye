/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/
 
#ifndef VIS_FG_MOTRACK_DATATYPE_H
#define VIS_FG_MOTRACK_DATATYPE_H

#include <vector>
#include "Eigen/Core"


typedef Eigen::Matrix<float, 1, 4, Eigen::RowMajor> DETECTBOX;
typedef Eigen::Matrix<float, -1, 4, Eigen::RowMajor> DETECTBOXSS;

//main
using RESULT_DATA = std::pair<int, DETECTBOX>;

//tracker:
using MATCH_DATA = std::pair<int, int>;
struct TRACKER_MATCHED {
    std::vector<MATCH_DATA> matches;
    std::vector<int> unmatched_tracks;
    std::vector<int> unmatched_detections;
};

//linear_assignment:
typedef Eigen::Matrix<float, -1, -1, Eigen::RowMajor> DYNAMICM;

#endif // VIS_FG_MOTRACK_DATATYPE_H
