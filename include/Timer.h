#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
public:
    Timer();
    void setDuration(unsigned long new_duration);
    bool hasElapsed();
    void restart();
private:
    unsigned long start_time;
    unsigned long duration;
};

#endif