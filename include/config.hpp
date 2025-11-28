#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace CCM {

/**
 * @brief Represents a user-defined area of interest in the camera feed.
 * Loaded dynamically from zones.yaml.
 */
struct ZoneConfig {
    std::string name;
    cv::Rect rect;
    cv::Scalar color;
    std::string trigger_class; // The object class (e.g., "person") that triggers an alert here.
    
    std::string toString() const;
    std::string toJSON() const;
};

// Hardware configuration struct
struct CameraConfig {
    int index = 0;
    int width = 640;
    int height = 480;
    int fps = 30;
    bool force_mjpg = false;

    std::string toString() const;
    std::string toJSON() const;
};

struct ModelConfig {
    int input_width = 640;
    int input_height = 640;

    std::string toString() const;
    std::string toJSON() const;
};

struct DebugConfig {
    bool enabled = false;
    float threshold = 0.5f;

    std::string toString() const;
    std::string toJSON() const;
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
    
    // Model Preprocessing Params
    float pixel_scale = 1.0f; 
    int input_width = 640;
    int input_height = 640;
    bool swap_rb = true;

    CameraConfig camera; 
    ModelConfig model;
    DebugConfig debug;
    
    // Custom Zones
    std::vector<ZoneConfig> zones;

    // Vector of Active Search Zones
    std::vector<std::string> active_search_zones;

    /**
     * @brief Factory method to parse the YAML configuration.
     * @param filepath Path to the .yaml file.
     * @return A fully populated AppConfig object.
     */    
    static AppConfig load(const std::string& filepath);
    
    std::string toString() const;
    std::string toJSON() const;
};

} // namespace CCM