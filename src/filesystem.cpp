#include "ironbox/filesystem.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace ironbox
{

    int setup_root_filesystem(const std::string &rootfs)
    {
        namespace fs = std::filesystem;

        fs::path root_path;

        try
        {
            root_path = fs::canonical(rootfs);
        }
        catch (const fs::filesystem_error &error)
        {
            std::cerr << "IronBox: invalid root filesystem path: "
                      << error.what()
                      << '\n';

            return 1;
        }

        // Safety guard: never allow "/" to become the supplied rootfs.
        if (root_path == "/")
        {
            std::cerr << "IronBox: refusing to use host root '/' as rootfs\n";
            return 1;
        }

        if (!fs::is_directory(root_path))
        {
            std::cerr << "IronBox: rootfs is not a directory: "
                      << root_path
                      << '\n';

            return 1;
        }

        const fs::path old_root_path = root_path / ".old_root";

        std::error_code error_code;

        fs::create_directory(old_root_path, error_code);

        if (error_code)
        {
            std::cerr << "IronBox: failed to create .old_root: "
                      << error_code.message()
                      << '\n';

            return 1;
        }

        // Make rootfs itself a mount point.
        if (mount(
                root_path.c_str(),
                root_path.c_str(),
                nullptr,
                MS_BIND | MS_REC,
                nullptr) != 0)
        {
            std::cerr << "IronBox: failed to bind-mount rootfs: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (syscall(
                SYS_pivot_root,
                root_path.c_str(),
                old_root_path.c_str()) != 0)
        {
            std::cerr << "IronBox: pivot_root failed: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (chdir("/") != 0)
        {
            std::cerr << "IronBox: chdir('/') failed: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (umount2("/.old_root", MNT_DETACH) != 0)
        {
            std::cerr << "IronBox: failed to detach old root: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (rmdir("/.old_root") != 0)
        {
            std::cerr << "IronBox: failed to remove /.old_root: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        return 0;
    }

    int mount_proc_filesystem()
    {
        if (mount("proc", "/proc", "proc", 0, nullptr) != 0)
        {
            std::cerr << "IronBox: failed to mount /proc: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        return 0;
    }

}