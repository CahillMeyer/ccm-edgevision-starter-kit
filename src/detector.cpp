#include "detector.hpp"
#include <fstream>
#include <iostream>

namespace CCM {

Detector::Detector(const std::string& model_path, const std::string& classes_path) {
    std::ifstream ifs(classes_path);
    std::string line;
    while (std::getline(ifs, line)) classes_.push_back(line);

    std::cout << "[Detector] Loading model: " << model_path << std::endl;
    net_ = cv::dnn::readNet(model_path);
    
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
}

std::vector<Detection> Detector::detect(const cv::Mat& frame, float conf_threshold, float nms_threshold) {
    std::vector<Detection> results;
    cv::Mat blob;
    
    // Use float literals (1.0f) to prevent double conversion warnings
    cv::dnn::blobFromImage(frame, blob, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false);
    net_.setInput(blob);

    std::vector<cv::Mat> outputs;
    net_.forward(outputs, net_.getUnconnectedOutLayersNames());

    float* data = (float*)outputs[0].data;
    
    // Safety: Explicitly cast generic YOLO constants
    const int rows = 25200; 
    const int dimensions = outputs[0].cols;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    float x_factor = static_cast<float>(frame.cols) / 640.0f;
    float y_factor = static_cast<float>(frame.rows) / 640.0f;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[4];
        if (confidence >= conf_threshold) {
            float* classes_scores = data + 5;
            cv::Mat scores(1, static_cast<int>(classes_.size()), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;
            cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

            if (max_class_score > conf_threshold) {
                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                
                // FIX: Use 0.5f (float) and static_cast<int> for safe narrowing
                int left = static_cast<int>((x - 0.5f * w) * x_factor);
                int top = static_cast<int>((y - 0.5f * h) * y_factor);
                int width = static_cast<int>(w * x_factor);
                int height = static_cast<int>(h * y_factor);

                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
                class_ids.push_back(class_id.x);
            }
        }
        data += dimensions;
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, conf_threshold, nms_threshold, nms_result);

    for (int idx : nms_result) {
        Detection det;
        det.class_id = class_ids[idx];
        // Safe size check
        if (class_ids[idx] >= 0 && static_cast<size_t>(class_ids[idx]) < classes_.size()) {
             det.className = classes_[class_ids[idx]];
        } else {
             det.className = "Unknown";
        }
        det.confidence = confidences[idx];
        det.box = boxes[idx];
        results.push_back(det);
    }

    return results;
}

} // namespace CCM