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
#include "stdlib.h"
#include "test.h"

#include "datatype.h"
#include "thirdPart/munkres/munkres.h"

void test_munkres(void) {
    DYNAMICM cost_matrix = Eigen::MatrixXf::Constant(7, 7, 0.70001);
    cost_matrix(0, 6) = 0.343278;
    cost_matrix(1, 5) = 0.164497;
    cost_matrix(2, 4) = 0.468738;
    cost_matrix(3, 0) = 0.214564;
    cost_matrix(4, 1) = 0.180936;
    cost_matrix(5, 3) = 0.38458;
    cost_matrix(6, 2) = 0.113495;

    int rows = cost_matrix.rows();
    int cols = cost_matrix.cols();
    Matrix<double> matrix(rows, cols);

    for (unsigned int i = 0; i < rows; i++) {
        for (unsigned int j = 0; j < cols; j++) {
            matrix(i, j) = cost_matrix(i, j);
        }
    }

    Munkres<double> m;
    m.solve(matrix);
    printf("munkres matrix:\n");

    for (unsigned int i = 0; i < matrix.rows(); i++) {
        for (unsigned int j = 0; j < matrix.columns(); j++) {
            printf("%f ", matrix(i, j));
        }
    }
}

