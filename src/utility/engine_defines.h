#ifndef ENGINE_DEFINES_H
#define ENGINE_DEFINES_H

#if defined(__unix__)
    #define ENGINE_FAST_FUNCTION(x) x
#elif defined(__arm__)
    #define ENGINE_FAST_FUNCTION(x) __time_critical_func(x) // https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#ga27ef91000958320e25ff481d16786ebf
#else
    #warning "Unknown platform for defining fast functions"
    #define ENGINE_FAST_FUNCTION(x) x
#endif

#endif

#define ADC_REF_VOLTAGE 3.3f
#define ADC_RESOLUTION (1 << 12)
#define ADC_CONV_FACTOR ADC_REF_VOLTAGE / (ADC_RESOLUTION - 1)

// Battery voltage is either 5V when charging or 4.2V to 2.75V on battery.
// The input voltage we're measuring is before the LDO. The measured voltage
// is dropped to below max readable reference voltage of 3.3V through 1/(1+1)
// voltage divider (cutting it in half):
// - PLUGGED IN (-diode forward):
//      * (5 - 0.4  [NSR0230P2T5G/https://www.onsemi.com/pdf/datasheet/nsr0230p2-d.pdf @ ~110mA]) / 2 = 2.3V                                                             <-  MAX VOLTAGE AT WORST DRAW                                                           <-  MAX VOLTAGE AT BEST DRAW
// - BATTERY (-p-mosfet Rds - battery internal resistance)
//      * (4.2 - (110mA/1000)(~95mOhm/1000) [AO3419/https://www.digikey.com/htmldatasheets/production/235766/0/0/1/ao3419.pdf Rds @ 110mA] -
//               (110mA/1000)(180mOhm/1000) [DTP401525/https://www.digikey.com/htmldatasheets/production/2065025/0/0/1/DTP401525-Specifications.pdf?#page=5]) = 2.08    <-  MAX VOLTAGE AT WORST DRAW


#define POWER_MAX_HALF_VOLTAGE 2.0f
#define POWER_MIN_HALF_VOLTAGE 1.4f // https://www.digikey.com/htmldatasheets/production/2065025/0/0/1/DTP401525-Specifications.pdf?#page=7 (release voltage)