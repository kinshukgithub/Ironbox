#include "ironbox/process.hpp"
#include "ironbox/namespace.hpp"
#include "ironbox/filesystem.hpp"
#include "ironbox/cgroup.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ironbox
{

    int run_process(
        const std::string &rootfs,
        const std::vector<std::string> &arguments)
    {
        if (arguments.empty())
        {
            std::cerr << "IronBox: no command provided\n";
            return 1;
        }

        pid_t setup_pid = fork();

        if (setup_pid < 0)
        {
            std::cerr << "IronBox: first fork failed: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (setup_pid == 0)
        {
            if (create_uts_namespace("ironbox") != 0)
            {
                _exit(1);
            }

            if (create_mount_namespace() != 0)
            {
                _exit(1);
            }

            if (create_pid_namespace() != 0)
            {
                _exit(1);
            }

            int sync_pipe[2];

            if (pipe(sync_pipe) != 0)
            {
                std::cerr << "IronBox: failed to create synchronization pipe: "
                          << std::strerror(errno)
                          << '\n';

                _exit(1);
            }

            pid_t container_pid = fork();

            if (container_pid < 0)
            {
                std::cerr << "IronBox: second fork failed: "
                          << std::strerror(errno)
                          << '\n';

                _exit(1);
            }

            if (container_pid == 0)
            {
                close(sync_pipe[1]); // I dont want the write end of the pipe in the child process, it shold only read from the pipe

                char signal; // Variable to hold the signal from the parent process

                ssize_t bytes_read = read(
                    sync_pipe[0],
                    &signal,
                    1);

                close(sync_pipe[0]); // Close the read end of the pipe after reading

                if (bytes_read != 1)
                {
                    std::cerr << "IronBox: failed waiting for cgroup setup\n";
                    _exit(1);
                }

                // This process is PID 1 inside the new PID namespace.

                if (setup_root_filesystem(rootfs) != 0)
                {
                    _exit(1);
                }

                if (mount_proc_filesystem() != 0)
                {
                    _exit(1);
                }

                std::vector<char *> argv;

                for (const std::string &argument : arguments)
                {
                    argv.push_back(
                        const_cast<char *>(argument.c_str()));
                }

                argv.push_back(nullptr);

                execvp(argv[0], argv.data());

                std::cerr << "IronBox: exec failed: "
                          << std::strerror(errno)
                          << '\n';

                _exit(127);
            }

            close(sync_pipe[0]); // Close the read end of the pipe in the parent process

            std::string cgroup_name = "ironbox-" + std::to_string(container_pid);

            if (create_cgroup(cgroup_name, container_pid) != 0)
            {
                close(sync_pipe[1]);

                kill(container_pid, SIGKILL);
                waitpid(container_pid, nullptr, 0);

                _exit(1);
            }

            const std::size_t memory_limit =
                256ULL * 1024ULL * 1024ULL;

            if (set_memory_limit(cgroup_name, memory_limit) != 0)
            {
                close(sync_pipe[1]);

                kill(container_pid, SIGKILL);
                waitpid(container_pid, nullptr, 0);

                _exit(1);
            }

            const std::size_t pids_limit = 32;

            if (set_pids_limit(cgroup_name, pids_limit) != 0)
            {
                close(sync_pipe[1]);

                kill(container_pid, SIGKILL);
                waitpid(container_pid, nullptr, 0);

                _exit(1);
            }

            const std::size_t cpu_quota_us = 50000;
            const std::size_t cpu_period_us = 100000;

            if (set_cpu_limit(
                    cgroup_name,
                    cpu_quota_us,
                    cpu_period_us) != 0)
            {
                close(sync_pipe[1]);

                kill(container_pid, SIGKILL);
                waitpid(container_pid, nullptr, 0);

                _exit(1);
            }

            char go_signal = 'G';

            ssize_t bytes_written = write(
                sync_pipe[1],
                &go_signal,
                1);

            close(sync_pipe[1]);

            int container_status = 0;

            if (waitpid(container_pid, &container_status, 0) < 0)
            {
                std::cerr << "IronBox: failed to wait for container process: "
                          << std::strerror(errno)
                          << '\n';

                _exit(1);
            }

            if (remove_cgroup(cgroup_name) != 0)
            {
                std::cerr << "IronBox: warning: cgroup cleanup failed\n";
            }

            if (WIFEXITED(container_status))
            {
                _exit(WEXITSTATUS(container_status));
            }

            if (WIFSIGNALED(container_status))
            {
                _exit(128 + WTERMSIG(container_status));
            }

            _exit(1);
        }

        int setup_status = 0;

        if (waitpid(setup_pid, &setup_status, 0) < 0)
        {
            std::cerr << "IronBox: failed to wait for setup process: "
                      << std::strerror(errno)
                      << '\n';

            return 1;
        }

        if (WIFEXITED(setup_status))
        {
            return WEXITSTATUS(setup_status);
        }

        if (WIFSIGNALED(setup_status))
        {
            return 128 + WTERMSIG(setup_status);
        }

        return 1;
    }

}