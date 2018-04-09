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

#endif // PROJECT_H
