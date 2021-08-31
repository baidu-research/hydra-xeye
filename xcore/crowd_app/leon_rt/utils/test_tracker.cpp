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
#include <UnitTestApi.h>
#include <VcsHooksApi.h>


using std::vector;


char uttxt[200*1024];

#ifdef __cplusplus
extern "C" {
#endif
float getTime(void);
#ifdef __cplusplus
}
#endif
extern vector<vector<float>> person_bbox;
extern vector<RESULT_DATA> track_result;
vector<vector<float>> result_bbox;

void parsetxt(void);

extern int run_tracker(void);

void testTracker(void)
{
    int offset = 0;
    char *line = uttxt;
    char *end;
    char linebuf[1024];
    int lnum;
    int lframe = 0;
    int curframe = 0;
    int trackid;
    float f1,f2,f3,f4,s0;
    int input = 1;
    person_bbox.clear();
    loadMemFromFile("ut.txt", 0, 0, 141054, uttxt);
    uttxt[141054] = 0;
    while(true){

        line = uttxt+offset;
        end = strstr(line,"\n");
        if (NULL != end){
            memcpy(linebuf, line,end - line);
            linebuf[end-line] = 0;
            offset += (end+1-line);
            if (NULL != strstr(linebuf,"fftest")){
                sscanf(linebuf,"%d:fftest: %d %f %f %f %f %f",&lnum,&lframe,&f1,&f2,&f3,&f4,&s0);
                input = 1;
                
              if (lframe == curframe)
              {
            
                vector<float> bbox_tmp;
                bbox_tmp.push_back(f1);
                bbox_tmp.push_back(f2);
                bbox_tmp.push_back(f3);
                bbox_tmp.push_back(f4);
                bbox_tmp.push_back(s0);

                person_bbox.push_back(bbox_tmp);
                continue;
              }
                
                
            }
            else if(NULL != strstr(linebuf,"ffresult")){
                sscanf(linebuf,"%d:ffresult %d %d %f %f %f %f",&lnum,&lframe,&trackid,&f1,&f2,&f3,&f4);
                input  = 0;
            //    printf("trackid %d\n",trackid);
                s0 = float(trackid);
              if (lframe == curframe)
              {
            
                vector<float> bbox_tmp;
                bbox_tmp.push_back(f1);
                bbox_tmp.push_back(f2);
                bbox_tmp.push_back(f3);
                bbox_tmp.push_back(f4);
                bbox_tmp.push_back(s0);
                result_bbox.push_back(bbox_tmp);
                continue;
              }
            }
          }
           for (int i = curframe;i < lframe;i++)
           {
               // printf("fftest frame %d \n",i);
               float time1 = getTime();
               run_tracker();
               float time2 = getTime();
               printf("fftracker %d person cost %f\n",person_bbox.size(),time2-time1);
               person_bbox.clear();
           }

            if (track_result.size() != result_bbox.size())
            {
              printf("track size error %d,%d,%d\n",curframe,track_result.size(),result_bbox.size());
              return;            
            }
            if (track_result.size() != 0) {
               for (unsigned int k = 0; k < track_result.size(); k++) {

                   DETECTBOX tmp_det = track_result[k].second;
                   int track_id = track_result[k].first;
                   if (track_id != int(result_bbox[k][4]))
                   {
                     printf("track id error:frame %d, %d %d\n",curframe,track_id, int(result_bbox[k][4]));
                     return;
                   }
                   if (result_bbox[k][0] != tmp_det(0) ||
                      result_bbox[k][1] != tmp_det(1) ||
                      result_bbox[k][2] != tmp_det(2) ||
                      result_bbox[k][3] != tmp_det(3))
                   {
                     printf("track bbox error: frame %d,%f %f %f %f\n",curframe,tmp_det(0),tmp_det(1),tmp_det(2),tmp_det(3));
                     return;
                   }
               }
            }
               result_bbox.clear();
               if (NULL == end)
               {
                  printf("fftest end\n");
                  break;
               }
               curframe = lframe;
               if (input == 1)
               {
               
                vector<float> bbox_tmp;
                bbox_tmp.push_back(f1);
                bbox_tmp.push_back(f2);
                bbox_tmp.push_back(f3);
                bbox_tmp.push_back(f4);
                bbox_tmp.push_back(s0);

                person_bbox.push_back(bbox_tmp);
               }
               else{
               
                vector<float> bbox_tmp;
                bbox_tmp.push_back(f1);
                bbox_tmp.push_back(f2);
                bbox_tmp.push_back(f3);
                bbox_tmp.push_back(f4);
                bbox_tmp.push_back(s0);
                result_bbox.push_back(bbox_tmp);
               }
    }

}
