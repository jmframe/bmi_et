#include <stdio.h>
#include <stdlib.h>
#include "../include/et.h"
#include "../include/bmi.h"
#include "../include/bmi_et.h"

#define INPUT_VAR_NAME_COUNT 5 // 
#define OUTPUT_VAR_NAME_COUNT 1 // et_m_per_s; 

int 
main(int argc, const char *argv[])
{
  if(argc<=1){
    printf("make sure to include an ET Type between 1 - 5\n");
    exit(1);
  }

  Bmi *model = (Bmi *) malloc(sizeof(Bmi));

  register_bmi_et(model);

  int et_method_int = 1*atoi(argv[1]);
  
  model->initialize(model);
  
  et_setup(model->data, et_method_int);

  model->update(model);
  return 0;
}

static int 
Initialize (Bmi *self)
{
    et_model *et;

    et = (et_model *) self->data;

    return BMI_SUCCESS;
}

static int 
Update (Bmi *self)
{
    run(((et_model *) self->data));
    return BMI_SUCCESS;
}

et_model *
new_bmi_et()
{
    et_model *data;
    data = (et_model*) malloc(sizeof(et_model));
    data->bmi.time_step_size = 3600;
    return data;
}

static int 
Finalize (Bmi *self)
{
  if (self){
    et_model* model = (et_model *)(self->data);
    self->data = (void*)new_bmi_et();
  }
}

//---------------------------------------------------------------------------------------------------------------------
static int 
Get_start_time (Bmi *self, double * time)
{
    *time = 0.0;
    return BMI_SUCCESS;
}

Bmi* 
register_bmi_et(Bmi *model)
{
    if (model) {
        model->data = (void*)new_bmi_et();
        model->initialize = Initialize;
        model->update = Update;
        model->finalize = Finalize;
        model->get_start_time = Get_start_time;
    }

    return model;
}

//---------------------------------------------------------------------------------------------------------------------
/*
static int Get_end_time (Bmi *self, double * time)
{
  Get_start_time(self, time);
  *time += (((et_model *) self->data)->num_timesteps * ((et_model *) self->data)->time_step_size);
  return BMI_SUCCESS;
}
*/

//---------------------------------------------------------------------------------------------------------------------
/*
static int Get_time_step (Bmi *self, double * dt)
{
    *dt = ((et_model *) self->data)->time_step_size;
    return BMI_SUCCESS;
}

static int Get_time_units (Bmi *self, char * units)
{
    strncpy (units, "s", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}
*/
//---------------------------------------------------------------------------------------------------------------------
/*
static int Get_current_time (Bmi *self, double * time)
{
    Get_start_time(self, time);
#if ET_DEGUG > 1
    printf("Current model time step: '%d'\n", ((et_model *) self->data)->current_time_step);
#endif
    *time += (((et_model *) self->data)->current_time_step * ((et_model *) self->data)->time_step_size);
    return BMI_SUCCESS;
} end Get_current_time
*/

//---------------------------------------------------------------------------------------------------------------------
/** Count the number of values in a delimited string representing an array of values. */
/*
static int count_delimited_values(char* string_val, char* delimiter)
{
    char *copy, *copy_to_free, *value;
    int count = 0;

    // Make duplicate to avoid changing original string
    // Then work on copy, but keep 2nd pointer to copy so that memory can be freed
    copy_to_free = copy = strdup(string_val);
    while ((value = strsep(&copy, delimiter)) != NULL)
        count++;
    free(copy_to_free);
    return count;
} end count_delimited_values
*/
//---------------------------------------------------------------------------------------------------------------------
/*
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
{
    int config_line_count, max_config_line_length;
    // Note that this determines max line length including the ending return character, if present
    int count_result = read_file_line_counts(config_file, &config_line_count, &max_config_line_length);
    if (count_result == -1) {
        printf("Invalid config file '%s'", config_file);
        return BMI_FAILURE;
    }

    FILE* fp = fopen(config_file, "r");
    if (fp == NULL)
        return BMI_FAILURE;

    // TODO: document config file format (<param_key>=<param_val>, where array values are comma delim strings)

    // TODO: things needed in config file:
    //  - forcing file name
    //  - et_options
    //      - yes_aorc
    //      - yes_wrf
    //      - use_energy_balance_method
    //      - use_aerodynamic_method
    //      - use_combination_method
    //      - use_priestley_taylor_method
    //      - use_penman_monteith_method
    //  - et_parms
    //      - wind_speed_measurement_height_m
    //      - humidity_measurement_height_m=
    //      - vegetation_height_m
    //      - zero_plane_displacement_height_m
    //      - momentum_transfer_roughness_length
    //      - heat_transfer_roughness_length_m
    //  - surf_rad_params.
    //      - surface_longwave_emissivity
    //      - surface_shortwave_albedo
    //  - solar_options
    //      - cloud_base_height_known
    //  -solar_parms
    //      - latitude_degrees
    //      - longitude_degrees
    //      - site_elevation_m

    char config_line[max_config_line_length + 1];

    // TODO: may need to add other variables to track that everything that was required was properly set

    // Keep track of whether required values were set in config
    // TODO: do something more efficient, maybe using bitwise operations
    int is_et_options_set = FALSE;
    int is_et_parms_set = FALSE;
    int is_surf_rad_parms_set = FALSE;
    int is_solar_options_set = FALSE;
    int is_solar_parms_set = FALSE;

    // Additionally,
    // ...
    // ...

    return BMI_SUCCESS;
} // end: read_init_config
*/
