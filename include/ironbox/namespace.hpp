#ifndef IRONBOX_NAMESPACE_HPP
#define IRONBOX_NAMESPACE_HPP

#include <string>

namespace ironbox
{

    int create_uts_namespace(const std::string &hostname);

    int create_pid_namespace();

    int create_mount_namespace();

}

#endif