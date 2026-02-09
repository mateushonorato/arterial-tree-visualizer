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

    void normalize() {
        if (nodes.empty()) return;
        // 1. Compute bounding box
        glm::vec3 minPos( std::numeric_limits<float>::max() );
        glm::vec3 maxPos(-std::numeric_limits<float>::max() );
        for (const auto& node : nodes) {
            minPos = glm::min(minPos, node.position);
            maxPos = glm::max(maxPos, node.position);
        }
        // 2. Compute max dimension
        float maxDim = glm::max(glm::max(maxPos.x - minPos.x, maxPos.y - minPos.y), maxPos.z - minPos.z);
        if (maxDim < 1e-6f) maxDim = 1.0f;
        // 3. Compute scale factor
        float scaleFactor = 2.0f / maxDim;
        // 4. Centering
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        for (auto& node : nodes) node.position = (node.position - center) * scaleFactor;
        // 5. Find max radius
        float maxRadius = 0.0f;
        for (const auto& seg : segments) maxRadius = std::max(maxRadius, seg.radius);
        float maxRadiusScaled = maxRadius * scaleFactor;
        float fixFactor = 1.0f;
        // 6. Heuristic: if maxRadiusScaled > 0.2, downscale radii
        if (maxRadiusScaled > 0.2f) {
            fixFactor = 0.05f / maxRadiusScaled;
            std::cout << "Detected Unit Mismatch! Downscaling radius by factor: " << fixFactor << std::endl;
        }
        // 7. Apply scale and fix to radii
        for (auto& seg : segments) seg.radius = seg.radius * scaleFactor * fixFactor;
    }
};

class VtkReader {
public:
    static bool load(const std::string& filepath, ArterialTree& outTree);
};
