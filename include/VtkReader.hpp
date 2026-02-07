#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>

struct ArterialNode {
    glm::vec3 position;
};

struct ArterialSegment {
    int indexA; // start node index
    int indexB; // end node index
    float radius; // from SCALARS
};

struct ArterialTree {
    std::vector<ArterialNode> nodes;
    std::vector<ArterialSegment> segments;
};

class VtkReader {
public:
    static bool load(const std::string& filepath, ArterialTree& outTree);
};
