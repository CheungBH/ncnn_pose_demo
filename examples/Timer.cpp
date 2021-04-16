#include "Timer.h"

Timer::Timer()
{
    this->time = std::chrono::high_resolution_clock::now();
}

float Timer::report()
{
    auto temp = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(temp - time);
    this->time = temp;
    return duration.count();
}