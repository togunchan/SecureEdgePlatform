#pragma once
#include <string>
#include <vector>

namespace cli
{
    class ICommand
    {
    public:
        virtual ~ICommand() = default;
        virtual std::string name() const = 0;
        virtual void execute(const std::vector<std::string> &args) = 0;
    };
}