#pragma once

#include <string>
#include <vector>

namespace nre
{
    class NrePipeline
    {
    public:
        NrePipeline(const std::string &vertFilepath, const std::string &fragFilepath);

    private:
        static std::vector<char> readFile(const std::string &filePath);

        void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath);
    };
} // namespace nre