#include "Diagnostics.hpp"

void Diagnostics::threadTask(){
    console->info("thread started!");
    while(!abortThread_)
    {
        stat_gpu = percent_gpu();
        stat_cpu = percent_cpu();
        stat_socTemp = soc_temp();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}