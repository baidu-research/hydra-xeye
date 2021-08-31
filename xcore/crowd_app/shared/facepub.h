#ifndef FACEPUB_H
#define FACEPUB_H


typedef struct image_{
    u8 *img;
    int cols;
    int rows;
}image_s;



typedef struct DetectedFaceInfo {
    float left;
    float top;
    float width;
    float height;
    float conf;
    float quality;
    float pose_x;
    float pose_y;
    float pose_z;
    int degree;
}DetectedFaceInfo;



#define SCORE_MAX_FACE_CACHE_SIZE  40
#define MAX_SELECTED_FACE_SIZE  20

// typedef struct{
    // DetectedFaceInfo detectedFaceInfo;
    // image_s img;
    // float score;
    // int drop;
    // void *pNextRecord;
// }FaceRecord;

typedef struct {
  int age;
  bool male;
} FaceAttributes;

typedef struct{
    DetectedFaceInfo detectedFaceInfo;
    image_s img;
    int drop;
    int faceId;
    int processed;
    int gender;
    int age;
}FaceRecord;

typedef struct{
    int faceId;
    int faceNum;
    int faceEnd;
    int faceDestoy;
    FaceRecord faceRecord[3];//0->highest score, 1->middle score, 2->lowest score
}FaceRecordTopList;

typedef struct{
    int faceId;
    int faceNum;
    int faceEnd;
    int faceDestoy;
    void *pHeadRecord;
    void *pLastRecord;
}FaceRecordLists;

const int log_buffer_size = 4 * 1024;


#endif
