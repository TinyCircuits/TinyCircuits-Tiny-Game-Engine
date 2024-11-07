#ifndef ENGINE_RTC_H
#define ENGINE_RTC_H

#include <time.h>   // For time struct that bm8563 library uses
#include <stdint.h>
#include <stdbool.h>

#define ENGINE_RTC_OK 1
#define ENGINE_RTC_I2C_ERROR -1

// Sets up I2C pins for RTC clock and checks if
// time has been compromised (i.e. lost power)
void engine_rtc_init();

// Checks if the RTC may have gone below voltage and lost time.
// Returns true of compromised
int engine_rtc_check_compromised();

// Get the broken down time
int engine_rtc_get_datetime(struct tm *rtc);

// Set the broken down time
int engine_rtc_set_datetime(struct tm *rtc);

#endif  // ENGINE_RTC_H