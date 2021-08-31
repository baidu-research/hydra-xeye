/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "matching/model.h"
#include <algorithm>


enum DETECTBOX_IDX {IDX_X = 0, IDX_Y, IDX_W, IDX_H };
ModelDetection *ModelDetection::instance = NULL;

ModelDetection *ModelDetection::getInstance()
{
    if (instance == NULL) {
        instance = new ModelDetection();
    }
    return instance;
}

void ModelDetection::dataMoreConf(float min_confidence, DETECTIONS &d)
{
    DETECTIONS::iterator it;
    for(it = d.begin(); it != d.end();) {
        if((*it).confidence < min_confidence) it = d.erase(it);
        else ++it;
    }
}

void ModelDetection::dataPreprocessing(float max_bbox_overlap, DETECTIONS &d)
{
    int size = int(d.size());
    if(size == 0) return;

    //generate idx:
    std::vector<int> idxs;
    idxs.reserve(size);

    std::vector<bool> idx_status;
    idx_status.reserve(size);
    for(size_t i = 0; i < size; ++i) {
        idxs.push_back(int(i));
        idx_status.push_back(false);
    }

    //get areas:
    std::vector<double> areas;
    areas.reserve(size);
    for(size_t i = 0; i < size; ++i) {
        double tmp = (d[i].tlwh(IDX_W)+1)*(d[i].tlwh(IDX_H)+1);
        areas.push_back(tmp);
    }

    //sort idxs by scores in ascending order ==>quick sort:
    _Qsort(d, idxs, 0, size-1);

    //get delete detections:
    std::vector<int> delIdxs;
    while(true) {//get compare idx;
        int i = -1;
        for(int j = size-1; j > 0; --j) {
            if(idx_status[j] == false) {
                i = j;
                idx_status[i] = true;
                break;
            }
        }
        if(i == -1) break; //end circle

        int x1 = d[idxs[i]].tlwh(IDX_X);
        int y1 = d[idxs[i]].tlwh(IDX_Y);
        int x2 = d[idxs[i]].tlwh(IDX_X) + d[idxs[i]].tlwh(IDX_W);
        int y2 = d[idxs[i]].tlwh(IDX_Y) + d[idxs[i]].tlwh(IDX_H);
        for(size_t j = 0; j < i; j++) {
            if(idx_status[j] == true) continue;
            int xx1 = int(d[idxs[j]].tlwh(IDX_X) > x1?d[idxs[j]].tlwh(IDX_X):x1);
            int yy1 = int(d[idxs[j]].tlwh(IDX_Y) > y1?d[idxs[j]].tlwh(IDX_Y):y1);
            int xx2 = d[idxs[j]].tlwh(IDX_X) + d[idxs[j]].tlwh(IDX_W);
            int yy2 = d[idxs[j]].tlwh(IDX_Y) + d[idxs[j]].tlwh(IDX_H);
            xx2 = (xx2 < x2?xx2:x2);
            yy2 = (yy2 < y2?yy2:y2);
            //standard area = w*h;
            int w = xx2-xx1+1;
            w = (w > 0?w:0);
            int h = yy2-yy1+1;
            h = (h > 0?h:0);
            //get delIdx;
            double tmp_overlap = w*h*1.0/areas[idxs[j]];
            if(tmp_overlap > max_bbox_overlap) {
                delIdxs.push_back(idxs[j]);
                idx_status[j] = true;
            }
        }
    }
    //delete from detections:
    for(size_t i = 0; i < delIdxs.size(); ++i) {
        DETECTIONS::iterator it = d.begin() + delIdxs[i];
        d.erase(it);
    }
}

ModelDetection::ModelDetection() {}

void ModelDetection::_Qsort(DETECTIONS d, std::vector<int>& a, int low, int high)
{
    if(low >= high) return;
    int first = low;
    int last = high;

    int key_idx = a[first];
    while(first < last) {
        while(first < last && d[a[last]].confidence >= d[key_idx].confidence) --last;
        a[first] = a[last];
        while(first < last && d[a[first]].confidence <= d[key_idx].confidence) ++first;
        a[last] = a[first];
    }
    a[first] = key_idx;
    _Qsort(d, a, low, first-1);
    _Qsort(d, a, first+1, high);
}

DETECTBOX DETECTION_ROW::to_xyah() const
{   //(centerx, centery, ration, h)
    DETECTBOX ret = tlwh;
    ret(0,IDX_X) += (ret(0, IDX_W)*0.5);
    ret(0, IDX_Y) += (ret(0, IDX_H)*0.5);
    ret(0, IDX_W) /= ret(0, IDX_H);
    return ret;
}

DETECTBOX DETECTION_ROW::to_tlbr() const
{   //(x,y,xx,yy)
    DETECTBOX ret = tlwh;
    ret(0, IDX_X) += ret(0, IDX_W);
    ret(0, IDX_Y) += ret(0, IDX_H);
    return ret;
}

