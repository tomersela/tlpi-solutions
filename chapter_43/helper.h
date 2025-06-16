#ifndef HELPER_H
#define HELPER_H

// show usage message for bandwidth measurement programs
void show_usage(char *prog_name);

// pin process to specific CPU core
void set_cpu_affinity(int core);

// get current time in nanoseconds using monotonic clock
long long get_current_time_ns(void);

#endif /* HELPER_H */ 
