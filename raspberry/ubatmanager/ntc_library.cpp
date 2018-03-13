#include "ntc_library.h"
#include <math.h>

/**********
* Defines *
**********/
/** Absolute Zero. */
#define TABS (-273.15)

/************
* Variables *
************/
/** Coefficients of Steinhart-Hart polynom. */
static double a_def[] = {
    4.524024725919526e-004,
    3.934722516618191e-004,
    -7.642331765196044e-006,
    4.048572707661904e-007,
};

NTC_library::NTC_library()
{
    for(int i=0;i<4;i++)
        a[i] = a_def[i];
}

void NTC_library::calibrate(double a0, double a1, double a2, double a3)
{
    a[0] = a0;
    a[1] = a1;
    a[2] = a2;
    a[3] = a3;
}

float NTC_library::fromAdc(int value)
{
    if(value>=1024) return 1e38f; // == infinite
    float v = (value * 5) / 1024;
    float r = (3300.0f * v) / (5.0f-v); // in ohm
    return (float)ttor((double)r);
}

double NTC_library::ttor(double t)
{
    double r;
    double u, v, p, q, b, c, d;

    t = t - TABS;
    d = (a[0] - 1.0 / t) / a[3];
    c = a[1] / a[3];
    b = a[2] / a[3];
    q = 2.0 / 27.0 * b * b * b - 1.0 / 3.0 * b * c + d;
    p = c - 1.0 / 3.0 * b * b;
    v = - pow(q / 2.0 + sqrt(q * q / 4.0 + p * p * p / 27.0), 1.0 / 3.0);
    u =   pow(-q / 2.0 + sqrt(q * q / 4.0 + p * p * p / 27.0), 1.0 / 3.0);
    r  = exp(u + v - b / 3.0);
    return r;
}
