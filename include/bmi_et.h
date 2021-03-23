#ifndef ET_BMI_ET_H
#define ET_BMI_ET_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "bmi.h"

int et_method_int;

Bmi* register_bmi_et(Bmi *model);

et_model * new_bmi_et();

#if defined(__cplusplus)
}
#endif

#endif