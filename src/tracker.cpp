#include "tracker.hpp"
#include <cmath>
#include <limits>
#include <algorithm>

namespace CCM {

// Simple Euclidean distance helper
// Return explicit float to avoid C4244
static float calculateDistance(const cv::Point& a, const cv::Point& b) {
    double dist_sq = std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2);
    return static_cast<float>(std::sqrt(dist_sq));
}

Tracker::Tracker(int max_lost_frames, float dist_threshold) 
    : next_id_(1), max_lost_frames_(max_lost_frames), dist_threshold_(dist_threshold) {}

std::vector<TrackedObject> Tracker::update(const std::vector<cv::Rect>& detections) {
    std::vector<TrackedObject> result_tracks;
    
    if (tracks_.empty()) {
        for (const auto& rect : detections) {
            register_object(rect);
        }
    } else {
        std::vector<bool> detection_used(detections.size(), false);
        
        for (auto& track : tracks_) {
            float min_dist = std::numeric_limits<float>::max();
            int best_idx = -1;

            for (size_t i = 0; i < detections.size(); ++i) {
                if (detection_used[i]) continue;

                cv::Point center_det = (detections[i].tl() + detections[i].br()) / 2;
                float dist = calculateDistance(track.center, center_det);

                if (dist < min_dist && dist < dist_threshold_) {
                    min_dist = dist;
                    // FIX: Explicit cast from size_t (i) to int
                    best_idx = static_cast<int>(i);
                }
            }

            if (best_idx != -1) {
                track.rect = detections[best_idx];
                track.center = (detections[best_idx].tl() + detections[best_idx].br()) / 2;
                track.lost_frames = 0;
                detection_used[best_idx] = true;
            } else {
                track.lost_frames++;
            }
        }

        for (size_t i = 0; i < detections.size(); ++i) {
            if (!detection_used[i]) {
                register_object(detections[i]);
            }
        }
    }

    auto it = tracks_.begin();
    while (it != tracks_.end()) {
        if (it->lost_frames > max_lost_frames_) {
            it = tracks_.erase(it);
        } else {
            result_tracks.push_back(*it);
            ++it;
        }
    }

    return result_tracks;
}

void Tracker::register_object(const cv::Rect& rect) {
    TrackedObject new_obj;
    new_obj.id = next_id_++;
    new_obj.rect = rect;
    new_obj.center = (rect.tl() + rect.br()) / 2;
    new_obj.lost_frames = 0;
    tracks_.push_back(new_obj);
}

} // namespace CCM