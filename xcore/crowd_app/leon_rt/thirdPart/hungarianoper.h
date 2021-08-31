#ifndef IDL_XTEAM_CROWD_LEON_RT_THIRDPART_HUNGARIANOPER_H
#define IDL_XTEAM_CROWD_LEON_RT_THIRDPART_HUNGARIANOPER_H
#include "munkres/munkres.h"
// #include "munkres/adapters/boostmatrixadapter.h"  No use
#include "datatype.h"

class HungarianOper {
public:
    static Eigen::Matrix<float, -1, 2, Eigen::RowMajor> solve(const DYNAMICM &cost_matrix);
};

#endif // VIS_FG_MOTRACK_HUNGARIANOPER_H
