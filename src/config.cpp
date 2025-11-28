#include "config.hpp"
#include <iostream>

namespace CCM {
std::string ZoneConfig::toString() const {
    std::ostringstream oss;
    oss << "ZoneConfig { name=\"" << name << "\", rect=("
        << rect.x << "," << rect.y << "," << rect.width << "," << rect.height
        << ") }";
    return oss.str();
}

std::string ZoneConfig::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"name\":\"" << name << "\","
        << "\"rect\":{"
            << "\"x\":" << rect.x << ","
            << "\"y\":" << rect.y << ","
            << "\"width\":" << rect.width << ","
            << "\"height\":" << rect.height
        << "}"
        << "}";
    return oss.str();
}

std::string CameraConfig::toString() const {
    std::ostringstream oss;
    oss << "CameraConfig { index=" << index
        << ", width=" << width
        << ", height=" << height
        << ", fps=" << fps
        << ", force_mjpg=" << (force_mjpg ? "true" : "false")
        << " }";
    return oss.str();
}

std::string CameraConfig::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"index\":" << index << ","
        << "\"width\":" << width << ","
        << "\"height\":" << height << ","
        << "\"fps\":" << fps << ","
        << "\"force_mjpg\":" << (force_mjpg ? "true" : "false")
        << "}";
    return oss.str();
}

std::string ModelConfig::toString() const {
    std::ostringstream oss;
    oss << "ModelConfig { "
        << "input_width=" << input_width
        << ", input_height=" << input_height
        << " }";
    return oss.str();
}

std::string ModelConfig::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"input_width\":" << input_width << ","
        << "\"input_height\":" << input_height << ","
        << "}";
    return oss.str();
}

std::string DebugConfig::toString() const {
    std::ostringstream oss;
    oss << "DebugConfig { enabled=" << (enabled ? "true" : "false")
        << ", threshold=" << threshold << " }";
    return oss.str();
}

std::string DebugConfig::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"enabled\":" << (enabled ? "true" : "false") << ","
        << "\"threshold\":" << threshold
        << "}";
    return oss.str();
}

AppConfig AppConfig::load(const std::string& filepath) {
    AppConfig config;
    cv::FileStorage fs(filepath, cv::FileStorage::READ);
    
    if (!fs.isOpened()) {
        std::cerr << "[Config] Warning: Could not open " << filepath << ". Using defaults." << std::endl;
        return config; 
    }

    // AI Settings
    if (!fs["model_path"].empty()) fs["model_path"] >> config.model_path;

    // Model Parameters
    cv::FileNode model_node = fs["model_parameters"];
    if (!model_node.empty()) {
        if (!model_node["pixel_scale"].empty()) model_node["pixel_scale"] >> config.pixel_scale;
        if (!model_node["input_width"].empty()) model_node["input_width"] >> config.input_width;
        if (!model_node["input_height"].empty()) model_node["input_height"] >> config.input_height;
        if (!model_node["swap_rb"].empty()) model_node["swap_rb"] >> config.swap_rb;
    }

    cv::FileNode yolo_node = fs["model"];
    if (!yolo_node.empty()) {
        if (!yolo_node["input_width"].empty())
            yolo_node["input_width"] >> config.model.input_width;
        if (!yolo_node["input_height"].empty())
            yolo_node["input_height"] >> config.model.input_height;
    }

    // AI Settings
    if (!fs["model_path"].empty()) fs["model_path"] >> config.model_path;
    if (!fs["class_names"].empty()) fs["class_names"] >> config.class_names;
    if (!fs["confidence_threshold"].empty()) fs["confidence_threshold"] >> config.confidence_threshold;
    if (!fs["nms_threshold"].empty()) fs["nms_threshold"] >> config.nms_threshold;

    // Camera Settings
    cv::FileNode cam_node = fs["camera"];
    if (!cam_node.empty()) {
        if (!cam_node["index"].empty()) cam_node["index"] >> config.camera.index;
        if (!cam_node["width"].empty()) cam_node["width"] >> config.camera.width;
        if (!cam_node["height"].empty()) cam_node["height"] >> config.camera.height;
        if (!cam_node["fps"].empty()) cam_node["fps"] >> config.camera.fps;
        if (!cam_node["force_mjpg"].empty()) cam_node["force_mjpg"] >> config.camera.force_mjpg;
    }

    // Zones
    cv::FileNode zones_node = fs["zones"];
    if (!zones_node.empty()) {
        for (auto it = zones_node.begin(); it != zones_node.end(); ++it) {
            ZoneConfig z;
            (*it)["name"] >> z.name;
            std::vector<int> r;
            (*it)["rect"] >> r;
            if (r.size() == 4) z.rect = cv::Rect(r[0], r[1], r[2], r[3]);
            std::vector<int> c;
            (*it)["color"] >> c;
            if (c.size() == 3) z.color = cv::Scalar(c[0], c[1], c[2]);
            (*it)["trigger_class"] >> z.trigger_class;
            config.zones.push_back(z);
        }
    }
    
    // Load Active Search Zones (List)
    if (!fs["active_search_zones"].empty()) {
        cv::FileNode search_node = fs["active_search_zones"];
        // Check if it's a sequence (list) or a single item
        if (search_node.type() == cv::FileNode::SEQ) {
            search_node >> config.active_search_zones;
        } 
        // Fallback: If user accidentally types a single string without brackets
        else if (search_node.type() == cv::FileNode::STRING) {
            std::string z;
            search_node >> z;
            config.active_search_zones.push_back(z);
        }
    }

    // Parse Debug Settings
    cv::FileNode debug_node = fs["debug"];
    if (!debug_node.empty()) {
        if (!debug_node["enabled"].empty()) debug_node["enabled"] >> config.debug.enabled;
        if (!debug_node["threshold"].empty()) debug_node["threshold"] >> config.debug.threshold;
    }
    
    // Log that we loaded it
    if (config.debug.enabled) {
        std::cout << "[Config] Debugging ENABLED (Threshold: " << config.debug.threshold << ")" << std::endl;
    }
    
    std::cout << "[Config] Loaded settings from " << filepath << std::endl;
    return config;
}

std::string AppConfig::toString() const {
    std::ostringstream oss;
    oss << "AppConfig:\n"
        << "  " << "\"model_path\":\"" << model_path << "\"," << "\n"
        << "  " << "\"class_names\":\"" << class_names << "\"," << "\n"
        << "  " << "\"confidence_threshold\":" << confidence_threshold << "," << "\n"
        << "  " << "\"nms_threshold\":" << nms_threshold << "," << "\n"
        << "  " << "\"pixel_scale\":" << pixel_scale << "," << "\n"
        << "  " << "\"input_width\":" << input_width << "," << "\n"
        << "  " << "\"input_height\":" << input_height << "," << "\n"
        << "  " << "\"swap_rb\":" << (swap_rb ? "true" : "false") << "," << "\n"
        << "  " << model.toString() << "\n"
        << "  " << camera.toString() << "\n"
        << "  " << debug.toString() << "\n"
        << "  Zones (" << zones.size() << "):\n";

    for (const auto& z : zones)
        oss << "    - " << z.toString() << "\n";

    return oss.str();
}

std::string AppConfig::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"model_path\":\"" << model_path << "\","
        << "\"class_names\":\"" << class_names << "\","
        << "\"confidence_threshold\":" << confidence_threshold << ","
        << "\"nms_threshold\":" << nms_threshold << ","
        << "\"pixel_scale\":" << pixel_scale << ","
        << "\"input_width\":" << input_width << ","
        << "\"input_height\":" << input_height << ","
        << "\"swap_rb\":" << (swap_rb ? "true" : "false") << ","
        << "\"model\":" << model.toJSON() << ","
        << "\"camera\":" << camera.toJSON() << ","
        << "\"debug\":" << debug.toJSON() << ","
        << "\"zones\":[";
    for (size_t i = 0; i < zones.size(); i++) {
        oss << zones[i].toJSON();
        if (i + 1 < zones.size()) oss << ",";
    }
    oss << "]}";
    return oss.str();
}

} // namespace CCM