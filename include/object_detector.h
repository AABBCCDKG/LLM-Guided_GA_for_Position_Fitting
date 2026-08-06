#ifndef SMARTGA_OBJECT_DETECTOR_H
#define SMARTGA_OBJECT_DETECTOR_H

#include <map>
#include <string>
#include <utility>
#include <vector>

class DetectObject {
public:
    DetectObject();

    static std::vector<std::vector<std::vector<float>>> transformData(const std::vector<std::vector<std::pair<int, int>>>& input_data);

    std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> detect(const std::vector<std::string>& image_paths, const std::vector<std::string>& shape_types);

private:
    using DetectorFunction = std::vector<std::pair<int, int>> (DetectObject::*)(const std::string&);
    std::map<std::string, DetectorFunction> detectors;

    std::map<std::string, std::vector<std::pair<int, int>>> detectShapesInImage(const std::string& image_path, const std::vector<std::string>& shape_types);
    std::vector<std::pair<int, int>> detectCircles(const std::string& image_path);
};

std::vector<std::string> getImagePaths(const std::string& folder_path);

#endif  // SMARTGA_OBJECT_DETECTOR_H
