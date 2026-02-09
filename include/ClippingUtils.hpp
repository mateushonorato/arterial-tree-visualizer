#pragma once
#include <glm/glm.hpp>

class ClippingUtils {
public:
    // Liang-Barsky line clipping for axis-aligned box
    // Returns true if segment is inside (possibly clipped), false if outside
    static bool clipSegment(glm::vec3& p0, glm::vec3& p1, const glm::vec3& boxMin, const glm::vec3& boxMax);
};
#pragma once