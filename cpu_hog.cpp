#include <cstdint>
#include <iostream>

int main()
{
    volatile std::uint64_t counter = 0;

    while (true)
    {
        ++counter;
    }

    return 0;
}
