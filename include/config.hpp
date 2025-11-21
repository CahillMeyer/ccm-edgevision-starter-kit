#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace CCM {

/**
 * @brief Represents a user-defined area of interest in the camera feed.
 * Loaded dynamically from zones.yaml.
 */
struct Zone {
    std::string name;
    cv::Rect rect;
    cv::Scalar color;
    std::string trigger_class; // The object class (e.g., "person") that triggers an alert here.
};

// Hardware configuration struct
struct CameraConfig {
    int index = 0;
    int width = 640;
    int height = 480;
    int fps = 30;
    bool force_mjpg = false;
};

/**
 * @brief Global application configuration settings.
 * Handles the "Customization Package" requirements without recompilation.
 */
struct AppConfig {
    // Model Settings
    std::string model_path;
    std::string class_names;
    
    // Detection Sensitivity
    float confidence_threshold; // Minimum confidence (0.0 - 1.0) to consider a detection valid.
    float nms_threshold;        // Overlap threshold for Non-Maximum Suppression (avoids duplicate boxes).

    
    CameraConfig camera; 
    
    // Custom Zones
    std::vector<Zone> zones;

    /**
     * @brief Factory method to parse the YAML configuration.
     * @param filepath Path to the .yaml file.
     * @return A fully populated AppConfig object.
     */    
     static AppConfig load(const std::string& filepath);
};

} // namespace CCM