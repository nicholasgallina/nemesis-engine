#include "nre_pipeline.hpp"

// std
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace nre
{

    NrePipeline::NrePipeline(const std::string &vertFilepath, const std::string &fragFilepath)
    {
        createGraphicsPipeline(vertFilepath, fragFilepath);
    }

    std::vector<char> NrePipeline::readFile(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void NrePipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath)
    {
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        std::cout << "Vertex Shader Code Size: " << vertCode.size() << '\n';
        std::cout << "Fragment Shader Code Size: " << vertCode.size() << '\n';
    }

} // namespace nre