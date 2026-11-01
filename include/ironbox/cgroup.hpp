#ifndef IRONBOX_CGROUP_HPP
#define IRONBOX_CGROUP_HPP

#include <cstddef>
#include <string>

#include <sys/types.h>

namespace ironbox
{

int create_cgroup(
    const std::string& name,
    pid_t pid
);

int set_memory_limit(
    const std::string& name,
    std::size_t limit_bytes
);

int set_pids_limit(
    const std::string& name,
    std::size_t max_pids
);

int set_cpu_limit(
    const std::string& name,
    std::size_t quota_us,
    std::size_t period_us
);

int remove_cgroup(const std::string& name);

}

#endif