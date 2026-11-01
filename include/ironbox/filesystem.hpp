#ifndef IRONBOX_FILESYSTEM_HPP
#define IRONBOX_FILESYSTEM_HPP

#include <string>

namespace ironbox
{

    int setup_root_filesystem(const std::string &rootfs);

    int mount_proc_filesystem();

}

#endif