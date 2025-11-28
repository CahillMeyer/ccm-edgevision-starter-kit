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

std::vector<Detection> Detector::detect(const cv::Mat& frame, const AppConfig& config) {
    std::vector<Detection> results;

    if (frame.empty()) {
        std::cerr << "[Detector] Warning: empty frame passed to detect()." << std::endl;
        return results;
    }

    // -------------------------------------------------------------------------
    // 1. Preprocess: use config-driven blob params
    // -------------------------------------------------------------------------
    cv::Mat blob;
    cv::dnn::blobFromImage(
        frame,
        blob,
        config.pixel_scale,                                  // e.g. 1.0f or 1/255.f
        cv::Size(config.input_width, config.input_height),   // e.g. 640x640
        cv::Scalar(),                                        // no mean subtraction
        config.swap_rb,                                      // swap RB if needed
        false                                                // crop = false
    );

    net_.setInput(blob);

    // -------------------------------------------------------------------------
    // 2. Forward pass
    // -------------------------------------------------------------------------
    std::vector<cv::Mat> outputs;
    net_.forward(outputs, net_.getUnconnectedOutLayersNames());

    if (outputs.empty()) {
        std::cerr << "[Detector] Error: network returned no outputs." << std::endl;
        return results;
    }

    cv::Mat output = outputs[0];

    // -------------------------------------------------------------------------
    // 3. Flatten YOLOv5 output to [num_rows x dimensions]
    //    Typical YOLOv5 ONNX: [1, 25200, 85] -> [25200 x 85]
    // -------------------------------------------------------------------------
    int rows = 0;
    int dimensions = 0;

    if (output.dims == 2) {
        rows = output.rows;
        dimensions = output.cols;
    } else {
        int last_dim = output.size[output.dims - 1];
        int total = 1;
        for (int i = 0; i < output.dims - 1; ++i) {
            total *= output.size[i];
        }
        rows = total;
        dimensions = last_dim;
        output = output.reshape(1, rows);
    }

    if (dimensions < 5) {
        std::cerr << "[Detector] Invalid output shape: rows=" << rows
                  << " dims=" << dimensions << " (expected >= 5)" << std::endl;
        return results;
    }

    if (config.debug.enabled) {
        static bool meta_logged = false;
        if (!meta_logged) {
            std::cout << "[Detector] Output tensor flattened to "
                      << rows << " rows x " << dimensions << " dims" << std::endl;
            meta_logged = true;
        }
    }

    // -------------------------------------------------------------------------
    // 4. Decode detections from YOLO output
    //    Row format: [cx, cy, w, h, obj_conf, class0, class1, ...]
    // -------------------------------------------------------------------------
    const int num_classes = dimensions - 5;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {
        float* data = output.ptr<float>(i);

        float obj_conf = data[4];  // objectness

        // Quick gate on objectness first
        if (obj_conf < config.confidence_threshold) {
            continue;
        }

        // Find best class score
        int best_class_id = -1;
        float best_class_score = 0.0f;

        for (int c = 0; c < num_classes && c < static_cast<int>(classes_.size()); ++c) {
            float score = data[5 + c];  // assuming scores are already in [0,1] space
            if (score > best_class_score) {
                best_class_score = score;
                best_class_id = c;
            }
        }

        if (best_class_id < 0) {
            continue;
        }

        // Combined confidence = objectness * class probability
        float combined_conf = obj_conf * best_class_score;

        // Debug logging for candidates above debug.threshold
        if (config.debug.enabled && obj_conf >= config.debug.threshold) {
            std::string name = (best_class_id >= 0 && best_class_id < (int)classes_.size())
                               ? classes_[best_class_id]
                               : "unknown";

            std::cout << "[Debug] Obj: " << obj_conf
                      << " | Class Score: " << best_class_score
                      << " | Combined: " << combined_conf
                      << " | Candidate: " << name << std::endl;
        }

        // Final gate using combined confidence
        if (combined_conf < config.confidence_threshold) {
            continue;
        }

        // Raw model outputs
        float x      = data[0];
        float y      = data[1];
        float width  = data[2];
        float height = data[3];

        // Decode to pixel space
        float cx, cy, boxW, boxH;
        
        // Model gives coords in "model input" units (e.g. 640x640)
        const float x_factor = static_cast<float>(frame.cols) /
                                static_cast<float>(config.model.input_width);
        const float y_factor = static_cast<float>(frame.rows) /
                                static_cast<float>(config.model.input_height);

        cx   = x      * x_factor;
        cy   = y      * y_factor;
        boxW = width  * x_factor;
        boxH = height * y_factor;

        // Convert to top-left + size, in integer pixels
        int left     = static_cast<int>(cx - 0.5f * boxW);
        int top      = static_cast<int>(cy - 0.5f * boxH);
        int widthPx  = std::max(1, static_cast<int>(boxW));
        int heightPx = std::max(1, static_cast<int>(boxH));

        // Optional debug of *raw* values
        if (config.debug.enabled) {
            std::cout << "[DebugBoxRaw] x=" << x
                      << " y=" << y
                      << " w=" << width
                      << " h=" << height
                      << std::endl;
        }

        // Skip clearly invalid boxes
        if (widthPx <= 0 || heightPx <= 0) {
            if (config.debug.enabled) {
                std::cout << "[DebugBoxSkip] invalid box after decode: left=" << left
                          << " top=" << top
                          << " width=" << widthPx
                          << " height=" << heightPx
                          << " (frame " << frame.cols << "x" << frame.rows << ")"
                          << std::endl;
            }
            continue;
        }

        // Clamp to frame boundaries using *widthPx/heightPx*
        if (left < 0) {
            widthPx += left;   // shrink width by how far we’re off-screen
            left = 0;
        }
        if (top < 0) {
            heightPx += top;
            top = 0;
        }
        if (left + widthPx > frame.cols) {
            widthPx = frame.cols - left;
        }
        if (top + heightPx > frame.rows) {
            heightPx = frame.rows - top;
        }

        if (widthPx <= 0 || heightPx <= 0) {
            if (config.debug.enabled) {
                std::cout << "[DebugBoxSkip] clamped box has non-positive size: left=" << left
                          << " top=" << top
                          << " width=" << widthPx
                          << " height=" << heightPx
                          << " (frame " << frame.cols << "x" << frame.rows << ")"
                          << std::endl;
            }
            continue;
        }

        if (config.debug.enabled) {
            std::cout << "[DebugBoxDecoded] left=" << left
                      << " top=" << top
                      << " width=" << widthPx
                      << " height=" << heightPx
                      << " (frame " << frame.cols << "x" << frame.rows << ")"
                      << " (frame " << frame.cols << "x" << frame.rows << ")"
                      << std::endl;
        }

        // Now store the *pixel* rect
        confidences.emplace_back(combined_conf);
        boxes.emplace_back(left, top, widthPx, heightPx);
        class_ids.emplace_back(best_class_id);
    }

    std::cout << "[Detector] raw boxes: " << boxes.size();

    // -------------------------------------------------------------------------
    // 5. Non-Maximum Suppression
    // -------------------------------------------------------------------------
    std::vector<int> nms_indices;
    cv::dnn::NMSBoxes(
        boxes,
        confidences,
        config.confidence_threshold,   // score threshold
        config.nms_threshold,          // NMS IoU threshold
        nms_indices
    );

    std::cout << " | after NMS: " << nms_indices.size() << std::endl;

    // -------------------------------------------------------------------------
    // 6. Build final Detection objects + zone filtering
    // -------------------------------------------------------------------------
    const bool restrict_to_zones = !config.active_search_zones.empty();

    for (int idx : nms_indices) {
        Detection det;
        det.class_id = class_ids[idx];
        det.confidence = confidences[idx];
        det.box = boxes[idx];

        if (det.class_id >= 0 && det.class_id < static_cast<int>(classes_.size())) {
            det.className = classes_[det.class_id];
        } else {
            det.className = "unknown";
        }

        if (restrict_to_zones) {
            cv::Point center(
                det.box.x + det.box.width / 2,
                det.box.y + det.box.height / 2
            );

            bool inside_allowed_zone = false;

            // Only consider zones listed in active_search_zones
            for (const auto& zone : config.zones) {
                // Zone must be in active_search_zones
                if (std::find(
                        config.active_search_zones.begin(),
                        config.active_search_zones.end(),
                        zone.name
                    ) == config.active_search_zones.end()) {
                    continue;
                }

                if (zone.rect.contains(center)) {
                    inside_allowed_zone = true;
                    break;
                }
            }

            if (!inside_allowed_zone) {
                if (config.debug.enabled) {
                    std::cout << "[Detector] Ignored detection outside active zones: "
                              << det.className << " @ ("
                              << center.x << "," << center.y << ")" << std::endl;
                }
                continue;
            }
        }

        results.push_back(det);
    }

    std::cout << "[Detector] final detections after zone filter: "
              << results.size() << std::endl;

    return results;
}


} // namespace CCM