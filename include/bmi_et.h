#ifndef ET_BMI_ET_H
#define ET_BMI_ET_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "bmi.h"
#include "Et_main.h"

/** Read number of lines in file and max line length, returning -1 if it does not exist or could not be read. */
int read_file_line_counts(const char* file_name, int* line_count, int* max_line_length);

int read_init_config(const char* config_file, et_model* model,
                     char* forcing_file,
                     bool* yes_aorc,
                     bool* yes_wrf,
                     bool* et_options,
                     double* wind_speed_measurement_height_m,
                     double* humidity_measurement_height_m,
                     double* vegetation_height_m,
                     double* zero_plane_displacement_height_m,
                     double* momentum_transfer_roughness_length,
                     double* heat_transfer_roughness_length_m,
                     double* surface_longwave_emissivity,
                     double* surface_shortwave_albedo,
                     bool* cloud_base_height_known,
                     double* latitude_degrees,
                     double* longitude_degrees,
                     double* site_elevation_m)

Bmi* register_bmi(Bmi *model);

Bmi* register_bmi_cfe(Bmi *model);

et_model * new_bmi_et(void);

#if defined(__cplusplus)
}
#endif

#endif