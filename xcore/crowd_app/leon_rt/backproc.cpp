#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pubdef.h"
#include "Fp16Convert.h"
#include "backproc.h"
#include "define.h"
#include <vector>
#include <mvLog.h>
using std::vector;

// this score line will be initialized at the beginning, when configuration file is loaded.
int g_score_line = 0;

static int very_large_confidence_count = 0;

int detect_backupproc(uint16_t *modelOut,  uint32_t modellen,
                    int picwidth, int pichigh, vector<BBox> &detectBBoxs) {

    int num = (int)f16Tof32(modelOut[0]);
    int iType = (int)f16Tof32(modelOut[1]);
    BBox tmpInfo;
    for(int n = 0, i = 7; n < num; i = i + 7, n++)
    {
        int iIndex = (int)f16Tof32(modelOut[i + 1]);
        float score = f16Tof32(modelOut[i+2]);
        int iscore = (int)(score * 100);

        if (iscore < g_score_line) {
            continue;
        }

        if (iscore >= 100) {
            very_large_confidence_count++;
        }
        // the system default logging level is MVLOG_INFO, so MVLOG_DEBUG here won't log anything to terminal
        mvLog(MVLOG_DEBUG, "very large confidence occures %d times", very_large_confidence_count);
        int left = (int)(f16Tof32(modelOut[i+3])*picwidth);
        int top = (int)(f16Tof32(modelOut[i+4])*pichigh);
        int right = (int)(f16Tof32(modelOut[i+5])*picwidth);
        int bottom = (int)(f16Tof32(modelOut[i+6])*pichigh);

        if (left < 0) left = 0;
        if (right < 0) right = 0;
        if (top < 0) top = 0;
        if (bottom < 0) bottom = 0;
        if (left > picwidth) left = picwidth;
        if (right > picwidth) right = picwidth;
        if (top > pichigh) top = pichigh;
        if (bottom > pichigh) bottom = pichigh;

        if (left >= right || top >= bottom) {
            continue;
        }

        tmpInfo.conf = score;
        tmpInfo.left = left;
        tmpInfo.top = top;
        tmpInfo.width = right - left;
        tmpInfo.height = bottom - top;
        tmpInfo.degree = 0;

        if (tmpInfo.width > 200 || tmpInfo.height > 200) {
            continue;
        }

        detectBBoxs.push_back(tmpInfo);
    }
}


