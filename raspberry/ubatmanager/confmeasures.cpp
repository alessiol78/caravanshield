#include "project.h"

ConfMeasures &ConfMeasures::operator =(const ConfMeasures &o)
{
    a_valim = o.a_valim;
    a_vbatsrv = o.a_vbatsrv;
    a_voutsol = o.a_voutsol;
    a_ialim = o.a_ialim;
    a_imotor = o.a_imotor;
    ntc1_a0 = o.ntc1_a0;
    ntc1_a1 = o.ntc1_a1;
    ntc1_a2 = o.ntc1_a2;
    ntc1_a3 = o.ntc1_a3;
    ntc2_a0 = o.ntc2_a0;
    ntc2_a1 = o.ntc2_a1;
    ntc2_a2 = o.ntc2_a2;
    ntc2_a3 = o.ntc2_a3;
    thr_vsol_h = o.thr_vsol_h;
    thr_vsol_l = o.thr_vsol_l;
    thr_vbat_h = o.thr_vbat_h;
    thr_vbat_l = o.thr_vbat_l;
    thr_valim_h = o.thr_valim_h;
    thr_valim_l = o.thr_valim_l;
    return *this;
}
