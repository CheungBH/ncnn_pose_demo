#include <chrono>
#include <iostream>
#include <string>

class Timer{
public:
    Timer();
    float report();
private:
    std::chrono::high_resolution_clock::time_point time;
};