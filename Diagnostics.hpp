#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>

class Diagnostics {
    public:
        Diagnostics():
            abortThread_(false),
            stat_gpu(0.0),
            stat_cpu(0.0),
            stat_socTemp(0.0),
            interval(1000)
        {
            console = spdlog::stdout_color_mt("diagnostics");
            main_thread_ = std::thread(std::bind(&Diagnostics::threadTask, this));
        } 

        ~Diagnostics(){
            abortThread_ = true;
            main_thread_.join();
        } 
        
        float stat_gpu;
        float stat_cpu;
        float stat_socTemp;

        int interval;

    private:
        std::shared_ptr<spdlog::logger> console;

        bool abortThread_;
        std::thread main_thread_;

        float percent_cpu(){
            return get_cpu_util();
        }

        float percent_gpu(){
            return get_gpu_usage((GPUPlugin*)&gpuStat) * 100.0;
        }

        float soc_temp(){
            return getCPUTemperature();
        }

        void threadTask();

        unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

        double get_cpu_util(){

            double percent;
            FILE* file;
            unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

            file = fopen("/proc/stat", "r");
            fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
                &totalSys, &totalIdle);
            fclose(file);

            if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
                totalSys < lastTotalSys || totalIdle < lastTotalIdle){
                //Overflow detection. Just skip this value.
                percent = -1.0;
            }
            else{
                total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                    (totalSys - lastTotalSys);
                percent = total;
                total += (totalIdle - lastTotalIdle);
                percent /= total;
                percent *= 100;
            }

            lastTotalUser = totalUser;
            lastTotalUserLow = totalUserLow;
            lastTotalSys = totalSys;
            lastTotalIdle = totalIdle;

            return percent;
        }

        typedef struct
        {                      
            /* Timer for periodic update */
            unsigned long last_val[5];
            unsigned long long last_timestamp;
        } GPUPlugin;

        GPUPlugin gpuStat;

        float get_gpu_usage(GPUPlugin *g)
        {
            char *buf = NULL;
            size_t res = 0;
            unsigned long jobs, runtime, active;
            unsigned long long ts, timestamp, elapsed;
            float max, load[5];
            int i;

            // open the stats file
            FILE *fp = fopen ("/sys/kernel/debug/dri/0/gpu_usage", "rb");
            if (fp == NULL) return 0.0;

            // read the stats file a line at a time
            while (getline (&buf, &res, fp) > 0)
            {
                if (sscanf (buf, "timestamp;%lld;", &ts) == 1)
                {
                    // use the timestamp line to calculate time since last measurement
                    timestamp = ts;
                    elapsed = timestamp - g->last_timestamp;
                    g->last_timestamp = timestamp;
                }
                else if (sscanf (strchr (buf, ';'), ";%ld;%ld;%ld;", &jobs, &runtime, &active) == 3)
                {
                    // depending on which queue is in the line, calculate the percentage of time used since last measurement
                    // store the current time value for the next calculation
                    i = -1;
                    if (!strncmp (buf, "v3d_bin", 7)) i = 0;
                    if (!strncmp (buf, "v3d_ren", 7)) i = 1;
                    if (!strncmp (buf, "v3d_tfu", 7)) i = 2;
                    if (!strncmp (buf, "v3d_csd", 7)) i = 3;
                    if (!strncmp (buf, "v3d_cac", 7)) i = 4;

                    if (i != -1)
                    {
                        if (g->last_val[i] == 0) load[i] = 0.0;
                        else
                        {
                            load[i] = runtime;
                            load[i] -= g->last_val[i];
                            load[i] /= elapsed;
                        }
                        g->last_val[i] = runtime;
                    }
                }
            }

            // list is now filled with calculated loadings for each queue for each PID
            free (buf);
            fclose (fp);

            // calculate the max of the five queue values and store in the task array
            max = 0.0;
            for (i = 0; i < 5; i++)
                if (load[i] > max)
                    max = load[i];

            return max;
        }

        float getCPUTemperature() {
            std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
            if (!file.is_open()) {
                std::cerr << "Failed to open temperature file." << std::endl;
                return -1.0f; // Return an error value
            }

            int tempMilliDegrees;
            file >> tempMilliDegrees;

            return tempMilliDegrees / 1000.0f; // Convert millidegrees to degrees
        }
};

#endif // DIAGNOSTICS_HPP
