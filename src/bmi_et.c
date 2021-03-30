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
  const char *cfg_file = "/glade/work/jframe/alt-modular/modules/et_fred_five/bmi_et_config.txt";
  model->initialize(model, et_method_int, cfg_file);
  
  model->update(model);
  
  model->finalize(model);
  
  return 0;
}

static int 
Initialize (Bmi *self, int et_method_int, const char *cfg_file)
{
    et_model *et;
    et = (et_model *) self->data;

    //int config_read_result = read_init_config(cfg_file, self->data);
    int config_read_result = read_init_config(cfg_file, et);
    if (config_read_result == BMI_FAILURE)
        return BMI_FAILURE;

    //et_setup(self->data, et_method_int);
    et_setup(et, et_method_int);
    return BMI_SUCCESS;
}

static int 
Update (Bmi *self)
{
    et_model *et = (et_model *) self->data;
    run(et);
    et->bmi.current_time_step += et->bmi.time_step_size;
    return BMI_SUCCESS;
}

et_model *
new_bmi_et()
{
    et_model *data;
    data = (et_model*) malloc(sizeof(et_model));

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
        printf("File does not exist.\n Failed in function read_file_line_counts\n");
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
int read_init_config(const char* config_file, et_model* model)//,
{
    int config_line_count, max_config_line_length;
    // Note that this determines max line length including the ending return character, if present
    int count_result = read_file_line_counts(config_file, &config_line_count, &max_config_line_length);
    if (count_result == -1) {
        return BMI_FAILURE;
    }

    FILE* fp = fopen(config_file, "r");
    if (fp == NULL)
        return BMI_FAILURE;

    // TODO: document config file format (<param_key>=<param_val>, where array values are comma delim strings)

    char config_line[max_config_line_length + 1];

    for (int i = 0; i < config_line_count; i++) {
        char *param_key, *param_value;
        fgets(config_line, max_config_line_length + 1, fp);
        char* config_line_ptr = config_line;
        config_line_ptr = strsep(&config_line_ptr, "\n");
        param_key = strsep(&config_line_ptr, "=");
        param_value = strsep(&config_line_ptr, "=");

        if (strcmp(param_key, "yes_aorc") == 0) {
            model->et_options.yes_aorc = strtod(param_value, NULL);
            printf("set aorc boolean from config file \n");
            printf("%d\n", model->et_options.yes_aorc);
            continue;
        }
        if (strcmp(param_key, "wind_speed_measurement_height_m") == 0) {
            model->et_params.wind_speed_measurement_height_m = strtod(param_value, NULL);
            printf("set wind speed measurement height from config file \n");
            printf("%lf\n", model->et_params.wind_speed_measurement_height_m);
            continue;
        }
        if (strcmp(param_key, "humidity_measurement_height_m") == 0) {
            model->et_params.humidity_measurement_height_m = strtod(param_value, NULL);
            printf("set humidity measurement height from config file \n");
            printf("%lf\n", model->et_params.humidity_measurement_height_m);
            continue;
        }
        if (strcmp(param_key, "vegetation_height_m") == 0) {
            model->et_params.vegetation_height_m = strtod(param_value, NULL);
            printf("vegetation height from config file \n");
            printf("%lf\n", model->et_params.vegetation_height_m);
            continue;
        }
        if (strcmp(param_key, "zero_plane_displacement_height_m") == 0) {
            model->et_params.zero_plane_displacement_height_m = strtod(param_value, NULL);
            printf("zero_plane_displacement height from config file \n");
            printf("%lf\n", model->et_params.zero_plane_displacement_height_m);
            continue;
        }
        if (strcmp(param_key, "shortwave_radiation_provided") == 0) {
            model->et_options.shortwave_radiation_provided = strtod(param_value, NULL);
            printf("shortwave radiation provided boolean from config file \n");
            printf("%d\n", model->et_options.shortwave_radiation_provided);
            continue;
        }
        if (strcmp(param_key, "momentum_transfer_roughness_length_m") == 0) {
            model->et_params.momentum_transfer_roughness_length_m = strtod(param_value, NULL);
            printf("momentum_transfer_roughness_length_m from config file \n");
            printf("%lf\n", model->et_params.momentum_transfer_roughness_length_m);
            continue;
        }
        if (strcmp(param_key, "surface_longwave_emissivity") == 0) {
            model->surf_rad_params.surface_longwave_emissivity = strtod(param_value, NULL);
            printf("surface_longwave_emissivity from config file \n");
            printf("%lf\n", model->surf_rad_params.surface_longwave_emissivity);
            continue;
        }
        if (strcmp(param_key, "surface_shortwave_albedo") == 0) {
            model->surf_rad_params.surface_shortwave_albedo = strtod(param_value, NULL);
            printf("surface_shortwave_albedo from config file \n");
            printf("%lf\n", model->surf_rad_params.surface_shortwave_albedo);
            continue;
        }
        if (strcmp(param_key, "surface_shortwave_albedo") == 0) {
            model->surf_rad_params.surface_shortwave_albedo = strtod(param_value, NULL);
            printf("surface_shortwave_albedo from config file \n");
            printf("%lf\n", model->surf_rad_params.surface_shortwave_albedo);
            continue;
        }
        if (strcmp(param_key, "latitude_degrees") == 0) {
            model->solar_params.latitude_degrees = strtod(param_value, NULL);
            printf("latitude_degrees from config file \n");
            printf("%lf\n", model->solar_params.latitude_degrees);
            continue;
        }
        if (strcmp(param_key, "longitude_degrees") == 0) {
            model->solar_params.longitude_degrees = strtod(param_value, NULL);
            printf("longitude_degrees from config file \n");
            printf("%lf\n", model->solar_params.longitude_degrees);
            continue;
        }
        if (strcmp(param_key, "site_elevation_m") == 0) {
            model->solar_params.site_elevation_m = strtod(param_value, NULL);
            printf("site_elevation_m from config file \n");
            printf("%lf\n", model->solar_params.site_elevation_m);
            continue;
        }
        if (strcmp(param_key, "time_step_size") == 0) {
            model->bmi.time_step_size = strtod(param_value, NULL);
            printf("time_step_size from config file \n");
            printf("%d\n", model->bmi.time_step_size);
            continue;
        }
        if (strcmp(param_key, "num_timesteps") == 0) {
            model->bmi.num_timesteps = strtod(param_value, NULL);
            printf("num_timesteps from config file \n");
            printf("%d\n", model->bmi.num_timesteps);
            continue;
        }

    } // end loop through config

    return BMI_SUCCESS;
} // end: read_init_config

static int Get_var_type (Bmi *self, const char *name, char * type)
{
    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(type, output_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(type, input_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    type[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_itemsize (Bmi *self, const char *name, int * size)
{
    char type[BMI_MAX_TYPE_NAME];
    int type_result = Get_var_type(self, name, type);
    if (type_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }

    if (strcmp (type, "double") == 0) {
        *size = sizeof(double);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "float") == 0) {
        *size = sizeof(float);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "int") == 0) {
        *size = sizeof(int);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "short") == 0) {
        *size = sizeof(short);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "long") == 0) {
        *size = sizeof(long);
        return BMI_SUCCESS;
    }
    else {
        *size = 0;
        return BMI_FAILURE;
    }
}

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
        model->get_var_type = Get_var_type;
        model->get_var_itemsize = Get_var_itemsize;
    }

    return model;
}  // end: tegister_bmi_et