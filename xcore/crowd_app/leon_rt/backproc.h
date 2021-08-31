#ifndef BACKPROC_H
#define BACKPROC_H
#include <vector>
#include "define.h"
using std::vector;

int detect_backupproc(uint16_t *modelOut,  uint32_t modellen,
                    int picwidth, int pichigh , vector<BBox> &detectBBoxs);


#endif
