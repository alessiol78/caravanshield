#ifndef NTC_LIBRARY_H
#define NTC_LIBRARY_H


class NTC_library
{
public:
    NTC_library();
    void calibrate(double a0, double a1,
                   double a2, double a3);
    float fromAdc(int value);

private:
    double ttor(double t);
    double a[4];
};

#endif // NTC_LIBRARY_H
