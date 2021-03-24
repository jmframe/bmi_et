#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/et.h"
#include "../include/bmi.h"
#include "../include/bmi_et.h"

#define INPUT_VAR_NAME_COUNT 7 // All the forcings? 
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
    ((et_model *) self->data)->bmi.current_time_step += ((et_model *) self->data)->bmi.time_step_size;
    return BMI_SUCCESS;
}

et_model *
new_bmi_et()
{
    et_model *data;
    data = (et_model*) malloc(sizeof(et_model));

    // READ THESE FROM CONFIG FILE:
    data->bmi.time_step_size = 3600;
    data->bmi.num_timesteps  = 1;

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

//---------------------------------------------------------------------------------------------------------------------
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
  "et_m_per_s",
};

//---------------------------------------------------------------------------------------------------------------------
static const char *output_var_types[OUTPUT_VAR_NAME_COUNT] = {
  "double",
};

//---------------------------------------------------------------------------------------------------------------------
static const int output_var_item_count[OUTPUT_VAR_NAME_COUNT] = {
  1,
};

//---------------------------------------------------------------------------------------------------------------------
static const char *output_var_units[OUTPUT_VAR_NAME_COUNT] = {
  "m s-1",
};

//---------------------------------------------------------------------------------------------------------------------
// Don't forget to update Get_value/Get_value_at_indices 
// (and setter) implementation if these are adjusted
static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
  "incoming_longwave_W_per_m2",
  "incoming_shortwave_W_per_m2",
  "surface_pressure_Pa",
  "specific_humidity_2m_kg_per_kg",
  "air_temperature_2m_K",
  "u_wind_speed_10m_m_per_s",
  "v_wind_speed_10m_m_per_s"
};

//---------------------------------------------------------------------------------------------------------------------
static const char *input_var_types[INPUT_VAR_NAME_COUNT] = {
  "double",
  "double",
  "double",
  "double",
  "double",
  "double",
  "double"
};

//---------------------------------------------------------------------------------------------------------------------
static const char *input_var_units[INPUT_VAR_NAME_COUNT] = {
  "W m-2",
  "W m-2",
  "Pa",
  "kg kg-1",
  "K",
  "m s-1",
  "m s-1"
};

//---------------------------------------------------------------------------------------------------------------------

static int Get_end_time (Bmi *self, double * time)
{
  Get_start_time(self, time);
  *time += (((et_model *) self->data)->bmi.num_timesteps * 
            ((et_model *) self->data)->bmi.time_step_size);
  return BMI_SUCCESS;
}


//---------------------------------------------------------------------------------------------------------------------

static int Get_time_step (Bmi *self, double * dt)
{
    *dt = ((et_model *) self->data)->bmi.time_step_size;
    return BMI_SUCCESS;
}

static int Get_time_units (Bmi *self, char * units)
{
    strncpy (units, "s", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}

//---------------------------------------------------------------------------------------------------------------------

static int Get_current_time (Bmi *self, double * time)
{
    Get_start_time(self, time);
#if ET_DEGUG > 1
    printf("Current model time step: '%d'\n", ((et_model *) self->data)->bmi.current_time_step);
#endif
    *time += (((et_model *) self->data)->bmi.current_time_step * 
              ((et_model *) self->data)->bmi.time_step_size);
    return BMI_SUCCESS;
} // end Get_current_time


//---------------------------------------------------------------------------------------------------------------------
/** Count the number of values in a delimited string representing an array of values. */

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
} //end count_delimited_values

//---------------------------------------------------------------------------------------------------------------------
int read_file_line_counts(const char* file_name, int* line_count, int* max_line_length)
{

    *line_count = 0;
    *max_line_length = 0;
    int current_line_length = 0;
    FILE* fp = fopen(file_name, "r");
    // Ensure exists
    if (fp == NULL) {
        return -1;
    }
    int seen_non_whitespace = 0;
    char c;
    for (c = fgetc(fp); c != EOF; c = fgetc(fp)) {
        // keep track if this line has seen any char other than space or tab
        if (c != ' ' && c != '\t' && c != '\n')
            seen_non_whitespace++;
        // Update line count, reset non-whitespace count, adjust max_line_length (if needed), and reset current line count
        if (c == '\n') {
            *line_count += 1;
            seen_non_whitespace = 0;
            if (current_line_length > *max_line_length)
                *max_line_length = current_line_length;
            current_line_length = 0;
        }
        else {
            current_line_length += 1;
        }
    }
    fclose(fp);

    // If we saw some non-whitespace char on last line, assume last line didn't have its own \n, so count needs to be
    // incremented by 1.
    if (seen_non_whitespace > 0) {
        *line_count += 1;
    }

    // Before returning, increment the max line length by 1, since the \n will be on the line also.
    *max_line_length += 1;

    return 0;
}  // end: read_file_line_counts

//---------------------------------------------------------------------------------------------------------------------
int read_init_config(const char* config_file, et_model* model,
                     bool yes_aorc,
                     bool yes_wrf,
                     int* et_method_int,
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
                     double* site_elevation_m,
                     int time_step_size,
                     int num_timesteps)
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

    char config_line[max_config_line_length + 1];

    int is_et_options_set = FALSE;
    int is_et_parms_set = FALSE;
    int is_surf_rad_parms_set = FALSE;
    int is_solar_options_set = FALSE;
    int is_solar_parms_set = FALSE;

    return BMI_SUCCESS;
} // end: read_init_config



//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
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
}  // end: tegister_bmi_et