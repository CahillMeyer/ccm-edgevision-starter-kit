#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

namespace CCM {

/**
 * @brief A persisted object track containing ID and history.
 */
struct TrackedObject {
    int id;              // Unique Persistent ID (e.g., "Person #42")
    cv::Rect rect;       // Current bounding box
    cv::Point center;    // Centroid for distance calculations
    int lost_frames;     // How many consecutive frames this object has been missing
};

/**
 * @brief A simple Euclidean distance tracker (IoU-based logic can be added later).
 * Matches new detections to existing tracks to maintain consistent IDs.
 */
class Tracker {
public:
    /**
     * @param max_lost_frames How long to keep a track alive without a matching detection.
     * @param dist_threshold  Max pixel distance to consider a detection a "match" for a track.
     */
    Tracker(int max_lost_frames = 5, float dist_threshold = 50.0f);

    /**
     * @brief Main tracking loop.
     * @param detections List of fresh bounding boxes from the Detector.
     * @return List of active tracks with stable IDs.
     */
    std::vector<TrackedObject> update(const std::vector<cv::Rect>& detections);

private:
    void register_object(const cv::Rect& rect);

    std::vector<TrackedObject> tracks_;
    int next_id_;
    int max_lost_frames_;
    float dist_threshold_;
};

} // namespace CCM