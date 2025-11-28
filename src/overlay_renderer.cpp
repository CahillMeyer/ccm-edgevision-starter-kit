#include "overlay_renderer.hpp"

namespace CCM {

void OverlayRenderer::draw(cv::Mat& frame, const std::vector<Detection>& detections, const AppConfig& config) {
    
    // TEMP: raw visualization of everything that Detector spits out.
    for (const auto& det : detections) {
        cv::rectangle(frame, det.box, cv::Scalar(0, 255, 255), 2);
        cv::putText(frame,
                    det.className + " " + cv::format("%.2f", det.confidence),
                    det.box.tl() + cv::Point(0, -5),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(0, 255, 255),
                    1);
    }
    
    // 1. Draw Zones (The "Customization Package" feature)
    for (const auto& zone : config.zones) {
        cv::rectangle(frame, zone.rect, zone.color, 2);
        
        // Label background
        int baseline;
        cv::Size labelSize = cv::getTextSize(zone.name, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseline);
        cv::rectangle(frame, 
            cv::Point(zone.rect.x, zone.rect.y - labelSize.height - 5),
            cv::Point(zone.rect.x + labelSize.width, zone.rect.y),
            zone.color, cv::FILLED);
            
        cv::putText(frame, zone.name, {zone.rect.x, zone.rect.y - 5}, 
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,0), 1);
    }

    // 2. Draw Detections & Trigger Alerts
    for (const auto& det : detections) {
        cv::Scalar box_color = cv::Scalar(0, 255, 0); // Default Green
        cv::Point center = (det.box.tl() + det.box.br()) / 2;

        // Logic: Check if detection is inside a specialized zone
        bool alert = false;
        for (const auto& zone : config.zones) {
            if (zone.rect.contains(center) && det.className == zone.trigger_class) {
                box_color = cv::Scalar(0, 0, 255); // Red for alert
                alert = true;
                
                // Alert Banner
                cv::putText(frame, "ALERT: " + zone.name, {50, 50}, 
                            cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0,0,255), 3);
            }
        }

        cv::rectangle(frame, det.box, box_color, 2);
        
        std::string label = det.className + " " + std::to_string(int(det.confidence * 100)) + "%";
        cv::putText(frame, label, {det.box.x, det.box.y - 10}, 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, box_color, 2);
    }
}

} // namespace CCM