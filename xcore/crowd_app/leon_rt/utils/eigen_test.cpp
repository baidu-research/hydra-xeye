///
/// @file
/// @copyright All code copyright Movidius Ltd 2016, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     MvTensor Test application
///

// Includes
// ----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <algorithm>


#include <vector>
#include <string>
#include <sstream>
#include "stdlib.h"
#include "test.h"

#include "datatype.h"

#define INFTY_COST 1e5

Eigen::VectorXf
 iou(DETECTBOX& bbox, DETECTBOXSS& candidates)
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

DYNAMICM
 iou_cost(void)
 {
     int rows = 10;
     int cols = 11;
     DYNAMICM cost_matrix = Eigen::MatrixXf::Zero(rows, cols);
     for(int i = 0; i < rows; i++) {
         int track_idx = i;
         cost_matrix.row(i) = Eigen::RowVectorXf::Constant(cols, INFTY_COST);
         DETECTBOX bbox = DETECTBOX(1.0+i,2.0+i,3.0+3,4.0+i);
         int csize = 11;
         DETECTBOXSS candidates(csize, 4);
         for(int k = 0; k < csize; k++) candidates.row(k) = DETECTBOX(2.0+k,3.0+k,4.0+k,5.0+k);
         Eigen::RowVectorXf rowV = (1. - iou(bbox, candidates).array()).matrix().transpose();
         cost_matrix.row(i) = rowV;
     }
     return cost_matrix;
 }
void test_eigen(void)
{
    DYNAMICM ff = iou_cost();
    for (int i = 0;i<ff.rows();i++)
    {
      for (int j = 0;j<ff.cols();j++)
      {
        //printf("%f ",ff(i,j));
      }
    }
}

