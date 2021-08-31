#ifndef VIS_FG_MOTRACK_MODEL_H
#define VIS_FG_MOTRACK_MODEL_H
#include "datatype.h"
#include <map>

/**
 * Each rect's data structure.
 * tlwh: topleft point & (w,h)
 * confidence: detection confidence.
 * feature: the rect's 128d feature.
 */
class DETECTION_ROW {
public:
    DETECTBOX tlwh; //np.float
    float confidence; //float
    DETECTBOX to_xyah() const;
    DETECTBOX to_tlbr() const;
};

typedef std::vector<DETECTION_ROW> DETECTIONS;

/**
 * Get each image's rects & corresponding features.
 * Method of filter conf.
 * Method of preprocessing.
 */
class ModelDetection
{

public:
    static ModelDetection* getInstance();
    void dataMoreConf(float min_confidence, DETECTIONS& d);
    void dataPreprocessing(float max_bbox_overlap, DETECTIONS& d);

private:
    ModelDetection();
    ModelDetection(const ModelDetection&);
    ModelDetection& operator =(const ModelDetection&);
    static ModelDetection* instance;

    using AREAPAIR = std::pair<int, double>;
    struct cmp {
        bool operator()(const AREAPAIR a, const AREAPAIR b) {
            return a.second < b.second;
        }
    };
    void _Qsort(DETECTIONS d, std::vector<int>& a, int low, int high);
};

#endif // VIS_FG_MOTRACK_MODEL_H
