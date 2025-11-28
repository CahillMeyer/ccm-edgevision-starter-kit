#include <iostream>
#include <fstream>
#include <csignal>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "detector.hpp"
#include "overlay_renderer.hpp"
#include "tracker.hpp" 

// Global flag for the main loop
std::atomic<bool> g_running(true);

std::string getAbsolutePath(const std::string& path) {
#ifdef _WIN32
    char buffer[_MAX_PATH];
    if (_fullpath(buffer, path.c_str(), _MAX_PATH) != NULL) {
        return std::string(buffer);
    }
#else
    return realpath(path.c_str(), NULL);
#endif
    // If resolution fails, return the original
    return path;
}

void signal_handler(int signum) {
    (void)signum;
    std::cout << "\n[System] Interrupt signal received. Shutting down..." << std::endl;
    g_running = false;
}

int main(int argc, char** argv) {
    // Register Signal Handler
    std::signal(SIGINT, signal_handler);

    std::cout << "[CCM Code] Initializing EdgeVision System..." << std::endl;

    // Argument Parsing
    std::string config_path = "configs/zones.yaml"; // Default
    bool test_mode = false;
    bool config_json = false;
    bool config_string = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test") {
            test_mode = true;
            std::cout << "[System] TEST MODE ENABLED." << std::endl;
        } else if (arg == "--config" && i + 1 < argc) {
            config_path = argv[++i]; // Consume next arg
        } else if (arg == "--config_string") {
            config_string = true; 
        } else if (arg == "--config_json") {
            config_json = true; 
        }
    }

    // Load Configuration
    CCM::AppConfig config = CCM::AppConfig::load(config_path);
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    std::cout << "[System] Loading configuration from:  " << getAbsolutePath(config_path.c_str()) << std::endl;

    if(config_string){
        std::cout << "[Debug] Config Loaded:\n" << config.toString() << std::endl;
        return 0;
    }

    if(config_json) {
        std::cout << config.toJSON() << std::endl;
        return 0;
    }
    // Initialize Modules
    CCM::Detector* detector = nullptr;
    if (!test_mode) {
        // Use values from config (or defaults if missing)
        std::string model = config.model_path.empty() ? "models/yolov5s.onnx" : config.model_path;
        std::string classes = config.class_names.empty() ? "models/coco.names" : config.class_names;
        
        std::ifstream f(model.c_str());
        if (!f.good()) {
            std::cerr << "[Error] Model file not found: " << model << std::endl;
            return -1;
        }
        detector = new CCM::Detector(model, classes);
    }

    // Initialize Tracker 
    CCM::Tracker tracker(5, 50.0f); 
    CCM::OverlayRenderer renderer;
    
    // Initialize Camera
    std::cout << "[Camera] Opening index " << config.camera.index << "..." << std::endl;
    cv::VideoCapture cap(config.camera.index); 
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera." << std::endl;
        return -1;
    }

    // Apply Camera Settings
    if (config.camera.force_mjpg) {
        std::cout << "[Camera] Forcing MJPEG compression (WSL/Linux Mode)" << std::endl;
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, config.camera.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, config.camera.height);
    cap.set(cv::CAP_PROP_FPS, config.camera.fps);

    cv::Mat frame;
    
    // Main Loop 
    while (g_running) {
        cap >> frame;
        if (frame.empty()) {
             std::cerr << "[Warning] Blank frame captured (Camera disconnected?)" << std::endl;
             continue; // Don't crash, just retry
        }

        std::vector<CCM::Detection> detections;
        
        if (!test_mode && detector) {
            // Pass the full config to the detector so it knows pixel_scale/swap_rb
            detections = detector->detect(frame, config);

            if (config.debug.enabled) {
                std::cout << "[Main] detections this frame: " << detections.size() << std::endl;
                for (const auto& d : detections) {
                    std::cout << "  - class=" << d.className
                            << " conf=" << d.confidence
                            << " box=" << d.box << std::endl;
                }
            }
        }
        // Test Mode Simulation
        else if (test_mode) {
             static int x_pos = 0;
             x_pos = (x_pos + 5) % config.camera.width;
             CCM::Detection det;
             det.class_id = 0;
             det.className = "person (sim)";
             det.confidence = 0.99f;
             det.box = cv::Rect(x_pos, 100, 100, 200);
             detections.push_back(det);
        }

        // Tracker & Renderer
        std::vector<cv::Rect> boxes;
        for(const auto& d : detections) boxes.push_back(d.box);
        auto tracked_objects = tracker.update(boxes); 

        renderer.draw(frame, detections, config);

        cv::imshow("CCM EdgeVision | Professional Edition", frame);
        
        if (cv::waitKey(1) == 27) {
            std::cout << "[System] ESC pressed. Exiting..." << std::endl;
            g_running = false;
        }
    }

    if (detector) delete detector;
    cap.release();
    cv::destroyAllWindows();
    std::cout << "[System] Cleanup complete. Goodbye." << std::endl;
    return 0;
}