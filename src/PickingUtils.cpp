#include "PickingUtils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace PickingUtils {

    glm::vec3 getRayFromMouse(double mouseX, double mouseY, int screenW, int screenH, const glm::mat4& view, const glm::mat4& projection) {
        // 1. Converter Coordenadas de Tela (Pixels) para NDC (-1 a 1)
        // NDC: X cresce para a direita, Y cresce para CIMA.
        // mouseX/Y vêm do GLFW (Y cresce para BAIXO).
        
        float x_ndc = (2.0f * (float)mouseX) / (float)screenW - 1.0f;
        float y_ndc = 1.0f - (2.0f * (float)mouseY) / (float)screenH; // Inverte Y
        
        glm::vec4 ray_clip = glm::vec4(x_ndc, y_ndc, -1.0f, 1.0f);

        // 2. Unproject: Clip Space -> Eye Space
        glm::mat4 invProj = glm::inverse(projection);
        glm::vec4 ray_eye = invProj * ray_clip;
        
        // Em Eye Space, estamos olhando para -Z.
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f); 

        // 3. Unproject: Eye Space -> World Space
        glm::mat4 invView = glm::inverse(view);
        glm::vec4 ray_world = invView * ray_eye;
        
        glm::vec3 rayDir = glm::normalize(glm::vec3(ray_world));
        return rayDir;
    }

    bool rayIntersectsSegment(
        glm::vec3 rayOrigin,
        glm::vec3 rayDir,
        glm::vec3 segA,
        glm::vec3 segB,
        float segRadius,
        float& outDist
    ) {
        glm::vec3 v = segB - segA;
        glm::vec3 w0 = rayOrigin - segA;

        float lenSq = glm::dot(v, v);
        if (lenSq < 1e-6f) return false;

        float a = glm::dot(rayDir, rayDir);
        float b = glm::dot(rayDir, v);
        float c = lenSq;
        float d = glm::dot(rayDir, w0);
        float e = glm::dot(v, w0);

        float denom = a * c - b * b;
        float sc, tc;

        if (denom < 1e-6f) {
            sc = 0.0f;
            tc = (b > c ? d / b : e / c);
        } else {
            sc = (b * e - c * d) / denom;
            tc = (a * e - b * d) / denom;
        }
        
        // Interseção precisa ser na frente da câmera
        if (sc < 0.0f) return false;

        // O ponto mais próximo no segmento (limitado entre 0 e 1)
        float tSeg = glm::clamp(tc, 0.0f, 1.0f);
        glm::vec3 ptSeg = segA + tSeg * v;
        
        // O ponto mais próximo no raio (infinito)
        glm::vec3 ptRay = rayOrigin + sc * rayDir;

        float dist = glm::length(ptRay - ptSeg);

        if (dist <= segRadius) {
            outDist = glm::length(ptRay - rayOrigin); // Distância até a câmera
            return true;
        }

        return false;
    }
}