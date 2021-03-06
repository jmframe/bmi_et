#ifndef ET_PRIESTLEY_TAYLOR_METHOD_H
#define ET_PRIESTLEY_TAYLOR_METHOD_H

// FUNCTION AND SUBROUTINE PROTOTYPES

double evapotranspiration_priestley_taylor_method(et_model *model);

//############################################################*
// subroutine to calculate evapotranspiration using           *
// Chow, Maidment, and Mays textbook                          *
// F.L. Ogden, NOAA National Weather Service, 2020            *
//############################################################*
double evapotranspiration_priestley_taylor_method(et_model *model)
{
  // local varibles
  double psychrometric_constant_Pa_per_C;
  double slope_sat_vap_press_curve_Pa_s;
  double moist_air_density_kg_per_m3;
  double water_latent_heat_of_vaporization_J_per_kg;
  double moist_air_gas_constant_J_per_kg_K;
  double moist_air_specific_humidity_kg_per_m3;
  double vapor_pressure_deficit_Pa;
  double liquid_water_density_kg_per_m3;
  double lambda_et;
  double radiation_balance_evapotranspiration_rate_m_per_s;
  double instantaneous_et_rate_m_per_s;
  double mass_flux;
  double delta;
  double gamma;

  calculate_intermediate_variables(model);

  liquid_water_density_kg_per_m3 = model->inter_vars.liquid_water_density_kg_per_m3;
  water_latent_heat_of_vaporization_J_per_kg=model->inter_vars.water_latent_heat_of_vaporization_J_per_kg;
  vapor_pressure_deficit_Pa=model->inter_vars.vapor_pressure_deficit_Pa;
  moist_air_gas_constant_J_per_kg_K=model->inter_vars.moist_air_gas_constant_J_per_kg_K;
  moist_air_density_kg_per_m3=model->inter_vars.moist_air_density_kg_per_m3;
  slope_sat_vap_press_curve_Pa_s=model->inter_vars.slope_sat_vap_press_curve_Pa_s;
  water_latent_heat_of_vaporization_J_per_kg=model->inter_vars.water_latent_heat_of_vaporization_J_per_kg;
  psychrometric_constant_Pa_per_C=model->inter_vars.psychrometric_constant_Pa_per_C;

  delta=slope_sat_vap_press_curve_Pa_s;
  gamma=psychrometric_constant_Pa_per_C;

  lambda_et=0.0;
  if( (model->et_options.use_aerodynamic_method == FALSE ) && (model->et_options.use_penman_monteith_method==FALSE) )
  {
    // This is equation 3.5.9 from Chow, Maidment, and Mays textbook.
    lambda_et=model->et_forcing.net_radiation_W_per_sq_m;
    radiation_balance_evapotranspiration_rate_m_per_s=lambda_et/
                                        (liquid_water_density_kg_per_m3*water_latent_heat_of_vaporization_J_per_kg);
  }
  instantaneous_et_rate_m_per_s=1.3*delta/(delta+gamma)*radiation_balance_evapotranspiration_rate_m_per_s;
  return(instantaneous_et_rate_m_per_s);
}

#endif // ET_PRIESTLEY_TAYLOR_METHOD_H
