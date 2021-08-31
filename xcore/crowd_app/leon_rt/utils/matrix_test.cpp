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

#include "thirdPart/munkres/matrix.h"
#include "test.h"

void test_matrix(void) {
    int rows = 4;
    int cols = 4;
    Matrix<double> matrix(rows, cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%f ", matrix(i, j));
        }
    }
}
