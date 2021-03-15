#include <stdio.h>
#include <math.h>
#include <string.h>

#include "EtWrapperFunction.h"
#include "EtSetParams.h"

double run_et_main(int et_method_option) {
  
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options;
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;

  et_setup(aorc, solar_options, solar_params, solar_forcing, set_et_options,
           et_params, et_forcing, surf_rad_params, surf_rad_forcing);

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method

  //the following two variables are set in function et_setup()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;

  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  return et_m_per_s;
}