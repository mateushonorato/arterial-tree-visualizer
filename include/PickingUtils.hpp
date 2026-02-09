#ifndef PICKING_UTILS_HPP
#define PICKING_UTILS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace PickingUtils {
    // Returns a ray in world space from the mouse position
    glm::vec3 getRayFromMouse(double mouseX, double mouseY, int screenW, int screenH, const glm::mat4& view, const glm::mat4& projection);

    // Returns true if the ray intersects the segment (cylinder), outputs the closest distance to ray origin
    bool rayIntersectsSegment(
        glm::vec3 rayOrigin,
        glm::vec3 rayDir,
        glm::vec3 segA,
        glm::vec3 segB,
        float segRadius,
        float& outDist
    );
}

#endif // PICKING_UTILS_HPP
