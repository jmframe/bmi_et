#ifndef ET_C
#define ET_C

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//local includes
#include "../include/et.h"
#include "../include/et_tools.h"
#include "../include/EtEnergyBalanceMethod.h"
#include "../include/EtAerodynamicMethod.h"
#include "../include/EtCombinationMethod.h"
#include "../include/EtPriestleyTaylorMethod.h"
#include "../include/EtPenmanMonteithMethod.h"

extern void alloc_et_model(et_model *model) {
    // TODO: *******************
}

extern void free_et_model(et_model *model) {
    // TODO: ******************
}

extern int run(et_model* model)

{
  // populate the evapotranspiration forcing data structure:
  //---------------------------------------------------------------------------------------------------------------
  model->et_forcing.air_temperature_C             = (double)model->aorc.air_temperature_2m_K-TK;  // gotta convert it to C
  model->et_forcing.relative_humidity_percent     = (double)-99.9; // this negative number means use specific humidity
  model->et_forcing.specific_humidity_2m_kg_per_kg= (double)model->aorc.specific_humidity_2m_kg_per_kg;
  model->et_forcing.air_pressure_Pa               = (double)model->aorc.surface_pressure_Pa;
  model->et_forcing.wind_speed_m_per_s            = hypot((double)model->aorc.u_wind_speed_10m_m_per_s,
                                                   (double)model->aorc.v_wind_speed_10m_m_per_s);                 

  if(model->et_options.yes_aorc==TRUE)
  {
    // wind speed was measured at 10.0 m height, so we need to calculate the wind speed at 2.0m
    double numerator=log(2.0/model->et_params.zero_plane_displacement_height_m);
    double denominator=log(model->et_params.wind_speed_measurement_height_m/model->et_params.zero_plane_displacement_height_m);
    model->et_forcing.wind_speed_m_per_s = model->et_forcing.wind_speed_m_per_s*numerator/denominator;  // this is the 2 m value
    model->et_params.wind_speed_measurement_height_m=2.0;  // change because we converted from 10m to 2m height.
  }

  if(model->et_options.yes_aorc==TRUE) 
  {
    // transfer aorc forcing data into our data structure for surface radiation calculations
    model->surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m = (double)model->aorc.incoming_shortwave_W_per_m2;
    model->surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m  = (double)model->aorc.incoming_longwave_W_per_m2; 
    model->surf_rad_forcing.air_temperature_C                       = (double)model->aorc.air_temperature_2m_K-TK;
    // compute relative humidity from specific humidity..
    double saturation_vapor_pressure_Pa = calc_air_saturation_vapor_pressure_Pa(model->surf_rad_forcing.air_temperature_C);
    double actual_vapor_pressure_Pa = (double)model->aorc.specific_humidity_2m_kg_per_kg*(double)model->aorc.surface_pressure_Pa/0.622;
    model->surf_rad_forcing.relative_humidity_percent = 100.0*actual_vapor_pressure_Pa/saturation_vapor_pressure_Pa;
    // sanity check the resulting value.  Should be less than 100%.  Sometimes air can be supersaturated.
    if(100.0< model->surf_rad_forcing.relative_humidity_percent) model->surf_rad_forcing.relative_humidity_percent = 99.0;
  }

  if(model->et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    model->solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    calculate_solar_radiation(model);
  }
  
  // we must calculate the net radiation before calling the ET subroutine.
  if(model->et_options.use_aerodynamic_method==FALSE) 
  {
    // NOTE don't call this function use_aerodynamic_method option is TRUE
    model->et_forcing.net_radiation_W_per_sq_m=calculate_net_radiation_W_per_sq_m(model);
  }

  if(model->et_options.use_energy_balance_method ==TRUE)
    model->et_m_per_s=evapotranspiration_energy_balance_method(model);
  if(model->et_options.use_aerodynamic_method ==TRUE)
    model->et_m_per_s=evapotranspiration_aerodynamic_method(model);
  if(model->et_options.use_combination_method ==TRUE)
    model->et_m_per_s=evapotranspiration_combination_method(model);
  if(model->et_options.use_priestley_taylor_method ==TRUE)
    model->et_m_per_s=evapotranspiration_priestley_taylor_method(model);
  if(model->et_options.use_penman_monteith_method ==TRUE)
    model->et_m_per_s=evapotranspiration_penman_monteith_method(model);

  if(model->et_options.use_energy_balance_method ==TRUE)   printf("energy balance method:\n");
  if(model->et_options.use_aerodynamic_method ==TRUE)      printf("aerodynamic method:\n");
  if(model->et_options.use_combination_method ==TRUE)      printf("combination method:\n");
  if(model->et_options.use_priestley_taylor_method ==TRUE) printf("Priestley-Taylor method:\n");
  if(model->et_options.use_penman_monteith_method ==TRUE)  printf("Penman Monteith method:\n");
                                                 
  printf("calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",model->et_m_per_s);
  printf("calculated instantaneous potential evapotranspiration (PET) =%8.6lf mm/d\n",model->et_m_per_s*86400.0*1000.0);
  
  return 0;
}

void et_setup(et_model* model, int et_method_option)
{

  //##########################################################
  // TODO: ALL OPTIONS READ IN FROM CONFIGURATION FILE.
  // TODO: READ IN FORCINGS FROM ASCII FILE.
  //##########################################################

  //##########################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.
  //##########################################################
  model->et_method = et_method_option;
  model->et_options.use_energy_balance_method   = FALSE;
  model->et_options.use_aerodynamic_method      = FALSE;
  model->et_options.use_combination_method      = FALSE;
  model->et_options.use_priestley_taylor_method = FALSE;
  model->et_options.use_penman_monteith_method  = FALSE;
  if (et_method_option == 1)
    model->et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    model->et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    model->et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    model->et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    model->et_options.use_penman_monteith_method  = TRUE;
  // Set this flag to TRUE if meteorological inputs come from AORC
  model->et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  model->et_options.shortwave_radiation_provided = FALSE;

  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  model->aorc.incoming_longwave_W_per_m2     =  117.1;
  model->aorc.incoming_shortwave_W_per_m2    =  599.7;
  model->aorc.surface_pressure_Pa            =  101300.0;
  model->aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  model->aorc.air_temperature_2m_K           =  25.0+TK;
  model->aorc.u_wind_speed_10m_m_per_s       =  1.54;
  model->aorc.v_wind_speed_10m_m_per_s       =  3.2;
  model->aorc.latitude                       =  37.865211;
  model->aorc.longitude                      =  -98.12345;
  model->aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  model->et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  model->et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  model->et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(model->et_options.yes_aorc==TRUE)
  {
    model->et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }
  model->et_params.humidity_measurement_height_m=2.0; 
  model->et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  model->et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  model->et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  model->et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.

  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  model->surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  model->surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(model->et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    model->surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    model->surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    model->surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    model->surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    model->surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    model->surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    model->surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    model->surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    model->surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    model->surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    model->surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    model->surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    model->surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    model->surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  model->surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(model->et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    model->solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    model->solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    model->solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    model->solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    model->surf_rad_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    model->surf_rad_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    model->surf_rad_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    model->surf_rad_forcing.zulu_time                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    if(model->et_options.use_energy_balance_method ==TRUE)   printf("energy balance method:\n");
    if(model->et_options.use_aerodynamic_method ==TRUE)      printf("aerodynamic method:\n");
    if(model->et_options.use_combination_method ==TRUE)      printf("combination method:\n");
    if(model->et_options.use_priestley_taylor_method ==TRUE) printf("Priestley-Taylor method:\n");
    if(model->et_options.use_penman_monteith_method ==TRUE)  printf("Penman Monteith method:\n");
  
    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  return;
}

#endif  // ET_C