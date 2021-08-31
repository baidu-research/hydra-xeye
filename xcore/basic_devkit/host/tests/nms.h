#pragma once
#include <opencv2/opencv.hpp>
#include <assert.h>

/**
 * @brief nms
 * Non maximum suppression
 * @param src_rects
 * @param res_rects
 * @param thresh
 * @param neighbors
 */
inline void nms(
        const std::vector<cv::Rect2f>& src_rects,
        std::vector<cv::Rect2f>& res_rects,
        float thresh,
        int neighbors = 0) {
    res_rects.clear();

    const size_t size = src_rects.size();
    if (!size) {
        return;
    }

    // Sort the bounding boxes by the bottom - right y - coordinate of the bounding box
    std::multimap<int, size_t> idxs;
    for (size_t i = 0; i < size; ++i) {
        idxs.insert(std::pair<int, size_t>(src_rects[i].br().y, i));
    }

    // keep looping while some indexes still remain in the indexes list
    while (idxs.size() > 0) {
        // grab the last rectangle
        auto last_elem = --std::end(idxs);
        const cv::Rect2f& rect1 = src_rects[last_elem->second];

        int neighbors_count = 0;

        idxs.erase(last_elem);

        for (auto pos = std::begin(idxs); pos != std::end(idxs); ) {
            // grab the current rectangle
            const cv::Rect2f& rect2 = src_rects[pos->second];

            float init_area = (rect1 & rect2).area();
            float union_area = rect1.area() + rect2.area() - init_area;
            float overlap = init_area / union_area;

            // if there is sufficient overlap, suppress the current bounding box
            if (overlap > thresh) {
                pos = idxs.erase(pos);
                ++neighbors_count;
            } else {
                ++pos;
            }
        }
        if (neighbors_count >= neighbors) {
            res_rects.push_back(rect1);
        }
    }
}

/**
 * @brief nms2
 * Non maximum suppression with detection scores
 * @param src_rects
 * @param src_scores
 * @param res_rects
 * @param thresh
 * @param neighbors
 */
inline void nms2(
        const std::vector<cv::Rect2f>& src_rects,
        const std::vector<float>& src_scores,
        std::vector<cv::Rect2f>& res_rects,
        std::vector<float>& res_scores,
        float thresh,
        int neighbors = 0,
        float min_scores_sum = 0.f) {
    res_rects.clear();
    res_scores.clear();

    const size_t size = src_rects.size();
    if (!size) {
        return;
    }

    assert(src_rects.size() == src_scores.size());

    // Sort the bounding boxes by the detection score
    std::multimap<float, size_t> idxs;
    for (size_t i = 0; i < size; ++i) {
        idxs.insert(std::pair<float, size_t>(src_scores[i], i));
    }

    // keep looping while some indexes still remain in the indexes list
    while (idxs.size() > 0) {
        // grab the last rectangle
        auto last_elem = --std::end(idxs);
        const cv::Rect2f& rect1 = src_rects[last_elem->second];
        const float& score1 = src_scores[last_elem->second];

        int neighbors_count = 0;
        float scores_sum = last_elem->first;

        idxs.erase(last_elem);

        for (auto pos = std::begin(idxs); pos != std::end(idxs); ) {
            // grab the current rectangle
            const cv::Rect2f& rect2 = src_rects[pos->second];

            float init_area = (rect1 & rect2).area();
            float union_area = rect1.area() + rect2.area() - init_area;
            float overlap = init_area / union_area;

            // if there is sufficient overlap, suppress the current bounding box
            if (overlap > thresh) {
                scores_sum += pos->first;
                pos = idxs.erase(pos);
                ++neighbors_count;
            } else {
                ++pos;
            }
        }
        if (neighbors_count >= neighbors &&
                scores_sum >= min_scores_sum) {
            res_rects.push_back(rect1);
            res_scores.push_back(score1);
        }
    }
}
