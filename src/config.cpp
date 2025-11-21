#include "config.hpp"
#include <iostream>

namespace CCM {

AppConfig AppConfig::load(const std::string& filepath) {
    AppConfig config;
    cv::FileStorage fs(filepath, cv::FileStorage::READ);
    
    if (!fs.isOpened()) {
        std::cerr << "[Config] Warning: Could not open " << filepath << ". Using defaults." << std::endl;
        return config; 
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
            Zone z;
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
    
    std::cout << "[Config] Loaded settings from " << filepath << std::endl;
    return config;
}

} // namespace CCM