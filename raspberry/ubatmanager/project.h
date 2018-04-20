#ifndef PROJECT_H
#define PROJECT_H

typedef struct {
    float v_bat_serv;
    float v_out_sol;
    float i_alim;
    float i_motor;
    float t_board;
    float ntc_1;
    float ntc_2;
    float v_alim;
} gmeasures;

class ConfMeasures {
public:
    //-- params for adc conversion
    float a_valim;
    float a_vbatsrv;
    float a_voutsol;
    float a_ialim;
    float a_imotor;
    float ntc1_a0;
    float ntc1_a1;
    float ntc1_a2;
    float ntc1_a3;
    float ntc2_a0;
    float ntc2_a1;
    float ntc2_a2;
    float ntc2_a3;
    //---- threashold for switch strategy
    float thr_vsol_h;   // min V out solar controller good
    float thr_vsol_l;   // under voltage level out solar protection
    float thr_vbat_h;   // V service battery
    float thr_vbat_l;   // V service battery
    float thr_valim_h;  // V motor recharger
    float thr_valim_l;  // V motor recharger

    ConfMeasures& operator=(const ConfMeasures &o);
};

#ifdef __arm__
#include "bcm2835/bcm2835.h"
const int pin_relay = RPI_V3_GPIO_P1_29; // 5
const int pin_aux1 = RPI_V3_GPIO_P1_36;  // 16
const int pin_aux2 = RPI_V3_GPIO_P1_35;  // 19
const int pin_buzz = RPI_V2_GPIO_P1_22;  // 25
const int pin_btn1 = RPI_V3_GPIO_P1_38;  // 20
const int pin_btn2 = RPI_V3_GPIO_P1_40;  // 21
const int pin_a0 = RPI_V2_GPIO_P1_18;    // 24
#endif

#endif // PROJECT_H
