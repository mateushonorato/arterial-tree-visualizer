/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: VtkReader.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa leitor simples de arquivos VTK contendo nós e segmentos arteriais.
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include "VtkReader.hpp"

bool VtkReader::load(const std::string &filepath, ArterialTree &outTree)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[VTKReader] Falha ao abrir arquivo: " << filepath << std::endl;
        return false;
    }
    std::string line;
    size_t numPoints = 0, numLines = 0, numLineIndices = 0;
    outTree.nodes.clear();
    outTree.segments.clear();
    std::vector<ArterialSegment> tempSegments;
    enum State
    {
        SEEK,
        POINTS,
        LINES,
        SCALARS,
        RADII
    } state = SEEK;
    size_t pointsRead = 0, linesRead = 0, radiiRead = 0;
    bool foundPoints = false, foundLines = false, foundRadii = false;
    while (std::getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty())
            continue;
        std::stringstream ss(line);
        std::string keyword;
        ss >> keyword;
        // Converte a palavra-chave para maiúsculas para correspondência robusta
        std::string keyword_upper = keyword;
        std::transform(keyword_upper.begin(), keyword_upper.end(), keyword_upper.begin(), ::toupper);
        if (state == SEEK)
        {
            if (keyword_upper == "POINTS")
            {
                ss >> numPoints;
                if (numPoints == 0)
                {
                    std::cerr << "[VTKReader] Seção POINTS não contém pontos." << std::endl;
                    return false;
                }
                state = POINTS;
                pointsRead = 0;
                continue;
            }
        }
        if (state == POINTS)
        {
            float x, y, z;
            std::stringstream pointStream(line);
            if (!(pointStream >> x >> y >> z))
            {
                std::cerr << "[VTKReader] Erro ao ler ponto no índice " << pointsRead << std::endl;
                return false;
            }
            outTree.nodes.push_back({glm::vec3(x, y, z)});
            pointsRead++;
            if (pointsRead == numPoints)
            {
                foundPoints = true;
                state = SEEK;
            }
            continue;
        }
        if (state == SEEK && keyword_upper == "LINES")
        {
            ss >> numLines >> numLineIndices;
            if (numLines == 0)
            {
                std::cerr << "[VTKReader] Seção LINES não contém linhas." << std::endl;
                return false;
            }
            tempSegments.clear();
            linesRead = 0;
            state = LINES;
            continue;
        }
        if (state == LINES)
        {
            int n, a, b;
            std::stringstream lineStream(line);
            if (!(lineStream >> n >> a >> b))
            {
                std::cerr << "[VTKReader] Erro ao ler conectividade de linha na linha " << linesRead << std::endl;
                return false;
            }
            if (n != 2)
            {
                std::cerr << "[VTKReader] Apenas segmentos (n=2) são suportados. Encontrado n=" << n << std::endl;
                return false;
            }
            tempSegments.push_back({a, b, 0.0f});
            linesRead++;
            if (linesRead == numLines)
            {
                foundLines = true;
                state = SEEK;
            }
            continue;
        }
        // Aceitar tanto SCALARS quanto scalars, e tanto radius quanto raio
        if (state == SEEK && keyword_upper == "SCALARS")
        {
            std::string name, type;
            ss >> name >> type;
            std::string name_lower = name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
            if (name_lower == "radius" || name_lower == "raio")
            {
                state = SCALARS;
            }
            continue;
        }
        if (state == SCALARS && keyword_upper == "LOOKUP_TABLE")
        {
            // Próximas linhas contém os raios (valores escalares)
            radiiRead = 0;
            state = RADII;
            continue;
        }
        if (state == RADII)
        {
            float r;
            std::stringstream scalarStream(line);
            if (!(scalarStream >> r))
            {
                std::cerr << "[VTKReader] Erro ao ler raio no índice " << radiiRead << std::endl;
                return false;
            }
            if (radiiRead >= tempSegments.size())
            {
                std::cerr << "[VTKReader] Mais raios que segmentos!" << std::endl;
                return false;
            }
            tempSegments[radiiRead].radius = r;
            radiiRead++;
            if (radiiRead == tempSegments.size())
            {
                foundRadii = true;
                state = SEEK;
            }
            continue;
        }
    }
    // Atribuição final e validação
    if (!foundPoints || !foundLines || !foundRadii)
    {
        std::cerr << "[VTKReader] Falha no parsing ou arquivo incompleto: " << filepath << std::endl;
        return false;
    }
    if (tempSegments.size() != radiiRead)
    {
        std::cerr << "[VTKReader] Número de raios (" << radiiRead << ") difere do número de segmentos (" << tempSegments.size() << ")" << std::endl;
        return false;
    }
    outTree.segments = tempSegments;
    outTree.normalize();
    for (auto &seg : outTree.segments)
    {
        glm::vec3 pA = outTree.nodes[seg.indexA].position;
        glm::vec3 pB = outTree.nodes[seg.indexB].position;
        seg.midpoint = (pA + pB) / 2.0f;
    }
    return true;
}
