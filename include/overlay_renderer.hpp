#pragma once
#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "detector.hpp"

namespace CCM {

class OverlayRenderer {
public:
    /**
     * Draws zones, detections, and alerts onto the frame.
     * @param frame The video frame (modified in place).
     * @param detections List of current object detections.
     * @param config The active application configuration (for zones).
     */
    void draw(cv::Mat& frame, const std::vector<Detection>& detections, const AppConfig& config);

private:
    // Helper to draw the dashboard header
    void drawHeader(cv::Mat& frame, float fps);
};

} // namespace CCM