/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: ScreenshotUtils.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa utilitário para capturar e salvar screenshots da janela OpenGL.
 */

#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include "lodepng.hpp"
#include "ScreenshotUtils.hpp"

void ScreenshotUtils::saveScreenshot(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Inverter verticalmente
    for (int y = 0; y < height / 2; ++y)
    {
        for (int x = 0; x < width * 4; ++x)
        {
            std::swap(pixels[y * width * 4 + x], pixels[(height - 1 - y) * width * 4 + x]);
        }
    }

    // Nome de arquivo com timestamp
    std::time_t t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << "screenshot_"
        << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
        << ".png";
    std::string filename = oss.str();

    unsigned error = lodepng::encode(filename, pixels, width, height);
    if (error)
    {
        std::cerr << "Erro ao salvar screenshot: " << lodepng_error_text(error) << std::endl;
    }
}
