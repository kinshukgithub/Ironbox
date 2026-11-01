#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    constexpr std::size_t chunk_size = 10 * 1024 * 1024; // 10 MiB

    std::vector<std::vector<char>> allocations;

    std::size_t total_allocated = 0;

    while (true)
    {
        allocations.emplace_back(chunk_size);

        // Actually touch the memory so Linux really allocates physical pages.
        std::fill(
            allocations.back().begin(),
            allocations.back().end(),
            1
        );

        total_allocated += chunk_size;

        std::cout
            << "Allocated: "
            << total_allocated / (1024 * 1024)
            << " MiB"
            << std::endl;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(300)
        );
    }
}
