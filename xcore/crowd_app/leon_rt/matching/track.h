#ifndef VIS_FG_MOTRACK_TRACK_H
#define VIS_FG_MOTRACK_TRACK_H

#include "datatype.h"
#include "model.h"


class Track
{
    /*"""
    A single target track with state space `(x, y, a, h)` and associated
    velocities, where `(x, y)` is the center of the bounding box, `a` is the
    aspect ratio and `h` is the height.

    Parameters
    ----------
    mean : ndarray
        Mean vector of the initial state distribution.
    covariance : ndarray
        Covariance matrix of the initial state distribution.
    track_id : int
        A unique track identifier.
    n_init : int
        Number of consecutive detections before the track is confirmed. The
        track state is set to `Deleted` if a miss occurs within the first
        `n_init` frames.
    max_age : int
        The maximum number of consecutive misses before the track state is
        set to `Deleted`.
    feature : Optional[ndarray]
        Feature vector of the detection this track originates from. If not None,
        this feature is added to the `features` cache.

    Attributes
    ----------
    mean : ndarray
        Mean vector of the initial state distribution.
    covariance : ndarray
        Covariance matrix of the initial state distribution.
    track_id : int
        A unique track identifier.
    hits : int
        Total number of measurement updates.
    age : int
        Total number of frames since first occurance.
    time_since_update : int
        Total number of frames since last measurement update.
    state : TrackState
        The current track state.
    features : List[ndarray]
        A cache of features. On each measurement update, the associated feature
        vector is added to this list.

    """*/
public:
    enum TrackState {Tentative = 1, Confirmed, Deleted};
    Track(DETECTBOX &location, int track_id, int n_init,
          int max_age, int dis_sign, int count_age);
    void predit();
    void update(const DETECTION_ROW &detection);
    void mark_missed();
    bool is_confirmed();
    bool is_deleted();
    bool is_tentative();
    DETECTBOX to_tlwh();
    int time_since_update;
    int track_id;
    DETECTBOX location;

    int hits;
    int age;
    int _n_init;
    int _max_age;
    TrackState state;
    int _dis_sign;
    int _cnt_age;
};

#endif // VIS_FG_MOTRACK_TRACK_H
