#ifndef IRONBOX_PROCESS_HPP
#define IRONBOX_PROCESS_HPP

#include <string>
#include <vector>

namespace ironbox
{

    int run_process(
        const std::string &rootfs,
        const std::vector<std::string> &arguments);

}

#endif