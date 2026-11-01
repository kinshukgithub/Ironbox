#include "ironbox/cgroup.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace ironbox
{
    int write_cgroup_value(
        const std::filesystem::path &file_path,
        const std::string &value)
    {
        std::ofstream stream(file_path);

        if (!stream)
        {
            std::cerr << "IronBox: failed to open "
                      << file_path
                      << '\n';

            return 1;
        }

        stream << value;

        if (!stream)
        {
            std::cerr << "IronBox: failed to write value to "
                      << file_path
                      << '\n';

            return 1;
        }

        return 0;
    }

    int set_memory_limit(
        const std::string &name,
        std::size_t limit_bytes)
    {
        namespace fs = std::filesystem;

        const fs::path memory_max =
            fs::path("/sys/fs/cgroup") /
            name /
            "memory.max";

        return write_cgroup_value(
            memory_max,
            std::to_string(limit_bytes));
    }

    int set_pids_limit(
        const std::string& name,
        std::size_t max_pids
    )
    {
        namespace fs = std::filesystem;

        const fs::path pids_max =
            fs::path("/sys/fs/cgroup") /
            name /
            "pids.max";

        return write_cgroup_value(
            pids_max,
            std::to_string(max_pids)
        );
    }

    int set_cpu_limit(
        const std::string& name,
        std::size_t quota_us,
        std::size_t period_us
    )
    {
        namespace fs = std::filesystem;

        const fs::path cpu_max =
            fs::path("/sys/fs/cgroup") /
            name /
            "cpu.max";

        const std::string value =
            std::to_string(quota_us)
            + " "
            + std::to_string(period_us);

        return write_cgroup_value(
            cpu_max,
            value
        );
    }

    int create_cgroup(
        const std::string &name,
        pid_t pid)
    {
        namespace fs = std::filesystem;

        const fs::path cgroup_root = "/sys/fs/cgroup";
        const fs::path cgroup_path = cgroup_root / name;

        std::error_code error_code;

        fs::create_directory(cgroup_path, error_code);

        if (error_code)
        {
            std::cerr << "IronBox: failed to create cgroup directory: "
                      << error_code.message()
                      << '\n';

            return 1;
        }

        const fs::path procs_file = cgroup_path / "cgroup.procs";

        std::ofstream procs_stream(procs_file);

        if (!procs_stream)
        {
            std::cerr << "IronBox: failed to open "
                      << procs_file
                      << '\n';

            return 1;
        }

        procs_stream << pid;

        if (!procs_stream)
        {
            std::cerr << "IronBox: failed to attach PID "
                      << pid
                      << " to cgroup\n";

            return 1;
        }

        return 0;
    }

    int remove_cgroup(const std::string& name)
    {
        namespace fs = std::filesystem;

        const fs::path cgroup_path =
            fs::path("/sys/fs/cgroup") / name;

        std::error_code error_code;

        bool removed = fs::remove(
            cgroup_path,
            error_code
        );

        if (error_code)
        {
            std::cerr << "IronBox: failed to remove cgroup "
                    << cgroup_path
                    << ": "
                    << error_code.message()
                    << '\n';

            return 1;
        }

        if (!removed)
        {
            std::cerr << "IronBox: cgroup does not exist: "
                    << cgroup_path
                    << '\n';

            return 1;
        }

        return 0;
    }

}