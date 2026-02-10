#ifndef PICKING_UTILS_HPP
#define PICKING_UTILS_HPP

#include <glm/glm.hpp>

namespace PickingUtils
{
    // Calcula Origem e Direção corretas para Perspectiva E Ortográfica
    void getRayFromMouse(
        double mouseX, double mouseY,
        int screenW, int screenH,
        const glm::mat4 &view,
        const glm::mat4 &projection,
        glm::vec3 &outOrigin,
        glm::vec3 &outDir);

    bool rayIntersectsSegment(
        glm::vec3 rayOrigin,
        glm::vec3 rayDir,
        glm::vec3 segA,
        glm::vec3 segB,
        float segRadius,
        float &outDist);
}

#endif // PICKING_UTILS_HPP