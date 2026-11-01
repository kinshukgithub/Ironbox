#include "ironbox/process.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: ironbox <command> [arguments...]\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "run")
    {
        if (argc < 4)
        {
            std::cerr
                << "Usage: ironbox run <rootfs> <program> [arguments...]\n";
            return 1;
        }

        std::string rootfs = argv[2];

        std::vector<std::string> arguments;

        for (int i = 3; i < argc; ++i)
        {
            arguments.emplace_back(argv[i]);
        }

        return ironbox::run_process(rootfs, arguments);
    }

    std::cerr << "Unknown command: " << command << '\n';
    return 1;
}