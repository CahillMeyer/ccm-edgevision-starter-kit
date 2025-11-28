#pragma once
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "config.hpp"

namespace CCM {

struct Detection {
    int class_id;
    std::string className;
    float confidence;
    cv::Rect box;
};

class Detector {
public:
    Detector(const std::string& model_path, const std::string& classes_path);
    
    // Main inference method
    std::vector<Detection> detect(const cv::Mat& frame, const AppConfig& config);

private:
    cv::dnn::Net net_;
    std::vector<std::string> classes_;
};

} // namespace CCM