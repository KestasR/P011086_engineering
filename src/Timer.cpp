#include "Timer.h"

Timer::Timer() {
    start_time = millis();
    duration = 0;
}

void Timer::setDuration(unsigned long new_duration) {
    duration = new_duration;
}

bool Timer::hasElapsed() {
    unsigned long elapsed_time = millis() - start_time;

    if (elapsed_time >= duration) {
        return true;
    } else {
        return false;
    }
}

void Timer::restart() {
    start_time = millis();
}
