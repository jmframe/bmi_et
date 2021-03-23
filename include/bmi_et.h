#ifndef BMI_ET_H
#define BMI_ET_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "bmi.h"
#include "et.h"

Bmi* register_bmi_et(Bmi *model);

et_model * new_bmi_et();

#if defined(__cplusplus)
}
#endif

#endif