/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "matching/track.h"


Track::Track(DETECTBOX& location, int track_id, int n_init,
             int max_age, int dis_sign, int count_age)
{
    this->location = location;
    this->track_id = track_id;
    this->hits = 1;
    this->age = 1;
    this->time_since_update = 0;
    this->state = TrackState::Tentative;

    this->_n_init = n_init;
    this->_max_age = max_age;
    this->_dis_sign = dis_sign;
    this->_cnt_age = count_age;
}

void Track::predit()
{
    this->age += 1;
    this->time_since_update += 1;
}

void Track::update(const DETECTION_ROW& detection)
{
    this->location = detection.tlwh;

    this->hits += 1;
    this->time_since_update = 0;
    if(this->state == TrackState::Tentative && this->hits >= this->_n_init) {
        this->state = TrackState::Confirmed;
    }
}

void Track::mark_missed()
{
    if(this->state == TrackState::Tentative) {
        this->state = TrackState::Deleted;
    } else if(this->time_since_update > this->_max_age) {
        this->state = TrackState::Deleted;
    }
}

bool Track::is_confirmed()
{
    return this->state == TrackState::Confirmed;
}

bool Track::is_deleted()
{
    return this->state == TrackState::Deleted;
}

bool Track::is_tentative()
{
    return this->state == TrackState::Tentative;
}

DETECTBOX Track::to_tlwh()
{
    DETECTBOX ret = location;
    return ret;
}

