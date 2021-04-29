#pragma once

namespace engine
{
    class Instance
    {
    public:
        Instance(std::string &name,
                 std::unordered_map<const char *, bool> &instance_extension,
                 std::vector<const char *> &validation_layers,
                 bool headless,
                 uint32_t api_version);
        Instance() = delete;
        ~Instance();
    };
}