#include "ironbox/namespace.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>

namespace ironbox
{

    int create_mount_namespace()
    {
        if (unshare(CLONE_NEWNS) != 0)
        {
            std::cerr << "IronBox: failed to create mount namespace: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0)
        {
            std::cerr << "IronBox: failed to make mount tree private: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        return 0;
    }

    int create_pid_namespace()
    {
        if (unshare(CLONE_NEWPID) != 0)
        {
            std::cerr << "IronBox: failed to create PID namespace: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        return 0;
    }

    int create_uts_namespace(const std::string &hostname)
    {
        if (unshare(CLONE_NEWUTS) != 0)
        {
            std::cerr << "IronBox: failed to create UTS namespace: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (sethostname(hostname.c_str(), hostname.size()) != 0)
        {
            std::cerr << "IronBox: failed to set hostname: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        return 0;
    }

}