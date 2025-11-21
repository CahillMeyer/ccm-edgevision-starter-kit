#include <iostream>
#include <fstream>
#include <csignal>  // <--- Required for Signal Handling
#include <atomic>   // <--- Required for thread-safe flags
#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "detector.hpp"
#include "overlay_renderer.hpp"
#include "tracker.hpp" 

// Global flag for the main loop
std::atomic<bool> g_running(true);

// Signal Handler: Catches Ctrl+C (SIGINT)
void signal_handler(int signum) {
    (void)signum;
    std::cout << "\n[System] Interrupt signal received. Shutting down..." << std::endl;
    g_running = false;
}

int main(int argc, char** argv) {
    // 1. Register Signal Handler
    std::signal(SIGINT, signal_handler);

    std::cout << "[CCM Code] Initializing EdgeVision System..." << std::endl;

    // 2. Argument Parsing
    bool test_mode = false;
    if (argc > 1 && std::string(argv[1]) == "--test") {
        test_mode = true;
        std::cout << "[System] TEST MODE ENABLED." << std::endl;
    }

    // 3. Load Configuration
    CCM::AppConfig config = CCM::AppConfig::load("configs/zones.yaml");
    
    // 4. Initialize Modules
    CCM::Detector* detector = nullptr;
    if (!test_mode) {
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
    
    // 5. Initialize Camera
    std::cout << "[Camera] Opening index " << config.camera.index << "..." << std::endl;
    cv::VideoCapture cap(config.camera.index); 
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera." << std::endl;
        return -1;
    }

    // Apply Camera Settings
    if (config.camera.force_mjpg) {
        std::cout << "[Camera] Forcing MJPEG compression (WSL Mode)" << std::endl;
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, config.camera.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, config.camera.height);
    cap.set(cv::CAP_PROP_FPS, config.camera.fps);

    cv::Mat frame;
    
    // 6. Main Loop 
    while (g_running) {
        cap >> frame;
        if (frame.empty()) {
             std::cerr << "[Warning] Blank frame captured (Camera disconnected?)" << std::endl;
             // In production, we might try to reconnect here. 
             // For now, we break to trigger cleanup.
             break;
        }

        std::vector<CCM::Detection> detections;if (!test_mode && detector) {
            detections = detector->detect(frame, config.confidence_threshold, config.nms_threshold);
        }

        // Convert detections to Rects for the tracker
        std::vector<cv::Rect> detection_boxes;
        for(const auto& d : detections) detection_boxes.push_back(d.box);
        
        // Run Tracker Update
        // Note: Currently we aren't visualizing the IDs in renderer.draw(), 
        // but the logic is now running and ready for future features.
        auto tracked_objects = tracker.update(detection_boxes); 
        // -------------------------

        renderer.draw(frame, detections, config);

        if (test_mode) {
            cv::putText(frame, "TEST MODE", {20, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,255,255}, 2);
        }

        cv::imshow("CCM EdgeVision | Professional Edition", frame);
        
        // Check for ESC key (local exit)
        if (cv::waitKey(1) == 27) {
            std::cout << "[System] ESC pressed. Exiting..." << std::endl;
            g_running = false;
        }
    }

    // 7. Graceful Cleanup
    if (detector) delete detector;
    cap.release();
    cv::destroyAllWindows();
    std::cout << "[System] Cleanup complete. Goodbye." << std::endl;
    return 0;
}