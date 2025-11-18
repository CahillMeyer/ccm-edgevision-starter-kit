#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

int main() {
    cv::VideoCapture cap(0);  // Use 0 for default USB cam, or "nvarguscamerasrc" for Jetson CSI

    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera.\n";
        return -1;
    }

    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        frame_count++;

        // Simple overlay
        cv::putText(frame, "CCM EdgeVision", {20, 40}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,0}, 2);
        cv::imshow("Live Camera Feed", frame);

        if (cv::waitKey(1) == 27) break;  // ESC to quit
    }

    auto end_time = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end_time - start_time).count();
    std::cout << "FPS: " << frame_count / elapsed << "\n";

    return 0;
}
