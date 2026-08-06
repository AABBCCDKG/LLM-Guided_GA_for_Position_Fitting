#include "object_detector.h"

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

DetectObject::DetectObject() {
    detectors["circle"] = &DetectObject::detectCircles;
}

std::vector<std::vector<std::vector<float>>> DetectObject::transformData(
    const std::vector<std::vector<std::pair<int, int>>>& input_data) {
    std::vector<std::vector<std::vector<float>>> result;
    result.reserve(input_data.size());

    for (const auto& positions : input_data) {
        std::vector<float> x_values;
        std::vector<float> y_values;
        x_values.reserve(positions.size());
        y_values.reserve(positions.size());
        for (const auto& [x, y] : positions) {
            x_values.push_back(static_cast<float>(x));
            y_values.push_back(static_cast<float>(y));
        }
        result.push_back({std::move(x_values), std::move(y_values)});
    }
    return result;
}

std::map<std::string, std::vector<std::vector<std::pair<int, int>>>>
DetectObject::detect(
    const std::vector<std::string>& image_paths,
    const std::vector<std::string>& shape_types) {
    std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> results;
    for (const auto& path : image_paths) {
        const auto image_results = detectShapesInImage(path, shape_types);
        for (const auto& shape : shape_types) {
            results[shape].push_back(image_results.at(shape));
        }
    }
    return results;
}

std::map<std::string, std::vector<std::pair<int, int>>>
DetectObject::detectShapesInImage(
    const std::string& image_path,
    const std::vector<std::string>& shape_types) {
    std::map<std::string, std::vector<std::pair<int, int>>> results;
    for (const auto& shape : shape_types) {
        const auto detector = detectors.find(shape);
        if (detector == detectors.end()) {
            throw std::invalid_argument("Unsupported shape type: " + shape);
        }
        results[shape] = (this->*detector->second)(image_path);
    }
    return results;
}

std::vector<std::pair<int, int>> DetectObject::detectCircles(
    const std::string& image_path) {
    const cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "Could not open or find the image: " << image_path << '\n';
        return {};
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(
        gray,
        circles,
        cv::HOUGH_GRADIENT,
        1.2,
        30,
        50,
        30,
        15,
        50);

    std::vector<std::pair<int, int>> circle_centers;
    circle_centers.reserve(circles.size());
    for (const auto& circle : circles) {
        circle_centers.emplace_back(
            static_cast<int>(circle[0]),
            static_cast<int>(circle[1]));
    }
    return circle_centers;
}

std::vector<std::string> getImagePaths(const std::string& folder_path) {
    if (!fs::exists(folder_path)) {
        throw std::invalid_argument("The folder path " + folder_path + " does not exist.");
    }

    std::vector<std::string> image_paths;
    for (const auto& entry : fs::directory_iterator(folder_path)) {
        if (entry.is_regular_file()) {
            auto path = entry.path().string();
            if (path.ends_with(".jpg") || path.ends_with(".png") || path.ends_with(".jpeg")) {
                image_paths.push_back(path);
            }
        }
    }
    std::sort(image_paths.begin(), image_paths.end());
    return image_paths;
}
