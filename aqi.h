/**
 * @file aqi.h
 * @author Aaron Solochek <aarons@gmail.com>
 * @brief Functions for computing AQI from pollution concentrations based on
 * https://www.airnow.gov/sites/default/files/2020-05/aqi-technical-assistance-document-sept2018.pdf
 * @version 0.1
 * @date 2023-08-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <math.h>

#define BREAK_COUNT 7 // All tables need to have the same size

/**
 * @brief Breakpoint structure for AQI tables
 * 
 */
typedef struct
{
    float lo;
    float hi;
} breakpoint;

/**
 * @brief AQI breakpoints
 * 
 */
const breakpoint aqi_breaks[] = 
{
    {0, 50},
    {51, 100},
    {101, 150},
    {151, 200},
    {201, 300},
    {301, 400},
    {401, 500}
};

/**
 * @brief PM₂․₅, µg/m³ 24-hour
 * 
 */
const breakpoint pm2_5_breaks[] = 
{
    {0, 12},
    {12.1, 35.4},
    {35.5, 55.4},
    {55.5, 150.4},
    {150.5, 250.4},
    {250.5, 350.4},
    {350.5, 500.4}
};

/**
 * @brief PM₁₀, µg/m³ 24-hour
 * 
 */
const breakpoint pm10_breaks[] = 
{
    {0, 54},
    {55, 154},
    {155, 254},
    {255, 354},
    {355, 424},
    {425, 504},
    {505, 604}
};

/**
 * @brief O₃, ppm 8-hour
 * 
 */
const breakpoint o3_8h_breaks[] =
{
    {0.000, 0.054},
    {0.055, 0.070},
    {0.071, 0.085},
    {0.086, 0.105},
    {0.106, 0.200},
    {0.405, 0.504}, // undefined in spec
    {0.505, 0.604}  // undefined in spec
};

/**
 * @brief O₃, ppm 1-hour
 * 
 */
const breakpoint o3_1h_breaks[] =
{
    {0.000, 0.054}, // undefined in spec, using 8h value
    {0.055, 0.124}, // undefined in spec, using 8h values
    {0.125, 0.164},
    {0.165, 0.204},
    {0.205, 0.404},
    {0.405, 0.504},
    {0.505, 0.604}
};

/**
 * @brief CO, ppm 8-hour
 * 
 */
const breakpoint co_breaks[] =
{
    {0.0, 4.4},
    {4.5, 9.4},
    {9.5, 12.4},
    {12.5, 15.4},
    {15.5, 30.4},
    {30.5, 40.4},
    {40.5, 50.4}
};

/**
 * @brief SO₂ ppb 1-hour
 * 
 */
const breakpoint so2_breaks[] =
{
    {0, 35},
    {36, 75},
    {76, 185},
    {186, 304},
    {305, 604},
    {605, 804},
    {805, 1004}
};

/**
 * @brief NO₂, ppb 1-hour
 * 
 */
const breakpoint no2_breaks[] =
{
    {0, 53},
    {54,100},
    {101, 360},
    {361, 649},
    {650, 1249},
    {1250, 1649},
    {1650, 2049}
};

/**
 * @brief Find the index of the breakpoint less than or equal to the concentration
 * 
 * @param val The truncated concentration of the pollutant
 * @param map The breakpoint table corresponding to the pollutant
 * @return Index into the breakpoint table of the low concentration
 */
int low_index(float val, const breakpoint map[])
{
    int i = BREAK_COUNT;
    while(--i >= 0)
    {
        if(map[i].lo <= val)
            break;
    }
    return i;
}

/**
 * @brief Find the index of the breakpoint greater than or equal to the concentration
 * 
 * @param val The truncated concentration of the pollutant
 * @param map The breakpoint table corresponding to the pollutant
 * @return Index into the breakpoint table of the high concentration
 */
int high_index(float val, const breakpoint map[])
{
    int i = 0;
    for(i = 0; i < BREAK_COUNT; i++)
    {
        if(map[i].hi >= val)
            break;
    }
    return i;
}

/**
 * @brief Truncate a float value to 1 decimal place
 * 
 * @param val The value to truncate
 * @return The truncated value
 */
float trunc1dp(float val)
{
    val *= 10.0;
    return truncf(val) / 10.0;
}

/**
 * @brief Truncate a float value to 3 decimal places
 * 
 * @param val The value to truncate
 * @return The truncated value
 */
float trunc3dp(float val)
{
    val *= 1000.0;
    return truncf(val) / 1000.0;
}

/**
 * @brief  Calculate the AQI for a given concentration and breakpoint table
 * 
 * @param val Concentration of pollutant
 * @param map Breakpoint table
 * @return Calculated AQI
 */
int calculate_aqi(float val, const breakpoint map[])
{
    int high_idx = high_index(val, map);
    int low_idx = low_index(val, map);
    float conc_hi = map[high_idx].hi;
    float conc_lo = map[low_idx].lo;
    int aqi_hi = aqi_breaks[high_idx].hi;
    int aqi_lo = aqi_breaks[low_idx].lo;

    return round(((float)aqi_hi - (float)aqi_lo) / (conc_hi - conc_lo) * (val - conc_lo) + aqi_lo);
}

/**
 * @brief Get the PM₂․₅ AQI value
 * 
 * @param raw Pollutant concentration in µg/m³
 * @return AQI
 */
int get_pm2_5_aqi(float raw)
{
    float val = trunc1dp(raw);
    return calculate_aqi(val, pm2_5_breaks);
}

/**
 * @brief Get the PM₁₀ AQI value
 * 
 * @param raw Pollutant concentration in µg/m³
 * @return AQI
 */
int get_pm10_aqi(float raw)
{
    float val = truncf(raw);
    return calculate_aqi(val, pm10_breaks);
}

/**
 * @brief Get the 8h ozone AQI -- This is the generally required one
 * 
 * @param raw Pollutant concentration in ppm
 * @return AQI
 */
int get_8h_ozone_aqi(float raw)
{
    float val = trunc3dp(raw);
    if(val > 0.2)
        return calculate_aqi(val, o3_1h_breaks);
    else
        return calculate_aqi(val, o3_8h_breaks);
}

/**
 * @brief Get the 1h ozone AQI
 * 
 * @param raw Pollutant concentration in ppm
 * @return AQI
 */
int get_1h_ozone_aqi(float raw)
{
    float val = trunc3dp(raw);
    // Technically the 1h Ozone AQI isn't defined for concentrations under .125
    //return val < 0.125 ? 0 : calculate_aqi(val, o3_1h_breaks);

    //but I compute it anyway
    return calculate_aqi(val, o3_1h_breaks);
}

/**
 * @brief Get the ozone AQI
 * 
 * @param raw_8h Pollutant concentration in ppm averaged over 8 hours
 * @param raw_1h Pollutant concentration in ppm averaged over 1 hour
 * @return AQI
 */
int get_ozone_aqi(float raw_8h, float raw_1h)
{
    int aqi_8h = get_8h_ozone_aqi(raw_8h);
    int aqi_1h = get_1h_ozone_aqi(raw_1h);
    return aqi_1h > aqi_8h ? aqi_1h : aqi_8h;
}

/**
 * @brief Get the CO AQI
 * 
 * @param raw Pollutant concentration in ppm
 * @return AQI
 */
int get_co_aqi(float raw)
{
    float val = trunc1dp(raw);
    return calculate_aqi(val, co_breaks);
}

/**
 * @brief Get the SO₂ AQI
 * 
 * @param raw Pollutant concentration in ppb
 * @return AQI
 */
int get_so2_aqi(float raw)
{
    float val = truncf(raw);
    return calculate_aqi(val, so2_breaks);
}

/**
 * @brief Get the NO₂ aqi object
 * 
 * @param raw Pollutant concentration in ppb
 * @return AQI
 */
int get_no2_aqi(float raw)
{
    float val = truncf(raw);
    return calculate_aqi(val, no2_breaks);
}

/**
 * @brief Get the overall AQI given all pollutants. Enter 0 for unused sensors
 * 
 * @param raw_pm25 PM₂․₅ concentration in µg/m³
 * @param raw_pm10 PM₁₀ concentration in µg/m³
 * @param raw_o3_1h O₃ concentration in ppm over 1 hour 
 * @param raw_o3_8h O₃ concentration in ppm over 8 hour 
 * @param raw_co CO concentration in ppm
 * @param raw_so2 SO₂ concentration in ppb
 * @param raw_no2 NO₂ concentration in ppb
 * @return Final AQI
 */
int get_total_aqi(float raw_pm25, 
                  float raw_pm10, 
                  float raw_o3_1h, 
                  float raw_o3_8h, 
                  float raw_co, 
                  float raw_so2, 
                  float raw_no2)
{
    int aqis[6] = {0};

    aqis[0] = get_pm2_5_aqi(raw_pm25);
    aqis[1] = get_pm10_aqi(raw_pm10);
    aqis[2] = get_ozone_aqi(raw_o3_8h, raw_o3_1h);
    aqis[3] = get_co_aqi(raw_co);
    aqis[4] = get_so2_aqi(raw_so2);
    aqis[5] = get_no2_aqi(raw_no2);
    
    int max = 0;
    for(int i=0; i < 6; i++)
    {
        if(aqis[i] > max)
            max = aqis[i];
    }
    return max;
}