#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

int main() {
    cv::VideoCapture cap(0);  // 0 = default webcam

    if (!cap.isOpened()) {
        std::cerr << "❌ Failed to open camera.\n";
        return -1;
    }

    std::cout << "✅ Camera opened successfully.\n";
    cv::Mat frame;
    int frame_count = 0;
    auto start = std::chrono::steady_clock::now();

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        frame_count++;
        cv::putText(frame, "CCM EdgeVision", {20, 40}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {0, 255, 0}, 2);

        cv::imshow("Camera Feed", frame);
        if (cv::waitKey(1) == 27) break;  // ESC to exit
    }

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "FPS: " << frame_count / seconds << "\n";

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
