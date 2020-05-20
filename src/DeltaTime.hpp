//
// DeltaTime: Simple class to precisely measure intervals
//
// Author: A.Navatta

#ifndef DELTA_TIME_H

#define DELTA_TIME_H

#include <particle.h>

#define MAX_TIMERS      5

class DeltaTime {
public:
    DeltaTime() {
        for (int i=0; i< MAX_TIMERS; i++)
            last_ticks[i] = 0;
            
        ticks_per_micro = System.ticksPerMicrosecond();
    }
    void start(int i) {
        last_ticks[i] = System.ticks();
    }

    uint32_t delta(int i) {
        uint32_t current_ticks = System.ticks();
        
        if (last_ticks[i] != 0) {
            uint32_t _d = (current_ticks - last_ticks[i])/ticks_per_micro;
            last_ticks[i] = current_ticks;
            return _d;
        } else
            last_ticks[i] = current_ticks;

        return 0;
    }

private:
    uint32_t last_ticks[MAX_TIMERS];
    double ticks_per_micro = 0;
};

#endif