//////////////////////////////////////////////////////////////////////////////////////
//       _______.  ______  __       ___      .__   __. .___________. __  ___   ___  //
//      /       | /      ||  |     /   \     |  \ |  | |           ||  | \  \ /  /  //
//     |   (----`|  ,----'|  |    /  ^  \    |   \|  | `---|  |----`|  |  \  V  /   //
//      \   \    |  |     |  |   /  /_\  \   |  . `  |     |  |     |  |   >   <    //
//  .----)   |   |  `----.|  |  /  _____  \  |  |\   |     |  |     |  |  /  .  \   //
//  |_______/     \______||__| /__/     \__\ |__| \__|     |__|     |__| /__/ \__\  //
//                                                                                  //
//  Originally developed by D. Pizzocri & T. Barani                                 //
//                                                                                  //
//  Version: 2.0                                                                    //
//  Year: 2022                                                                      //
//  Authors: G. Zullo                                                               //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#include "EnvironmentComposition.h"

void EnvironmentComposition()
{
  /**
   * @brief This routine evaluates the oxygen partial pressure (in an environment extern to the fuel)
   * in the approximation of only pure steam present (e.g., no cladding or fully oxidized).
   * 
   * @ref Lewis et al. JNM 227 (1995) 83-109
   * 
   * This is used to estimate to UO2 equilibrium oxygen partial pressure and the equilibrium stoichiometry,
   * if macroscopic models for UO2+x are used.
   * 
   */

	if (!input_variable[iv["iStoichiometryDeviation"]].getValue()) return;

  model.emplace_back();
  int model_index = int(model.size()) - 1;
  model[model_index].setName("Environment composition");

  std::string reference;
  
  /// @param equilibrium_constant law of mass action for the water vapour decomposition
  H2O_decomposition_equilibrium_constant KH2O; // (atm^(1/2))
  KH2O.K_H2O_Ulich(history_variable[hv["Temperature"]].getFinalValue());
  
  double K_H2O_value = KH2O.get_KH2O();
  double steam_pressure = history_variable[hv["Steam pressure"]].getFinalValue(); // (atm)
  double gap_oxigen_partial_pressure = pow(pow(K_H2O_value,2)*pow(steam_pressure,2)/4, 1.0/3.0); // (atm)

  sciantix_variable[sv["Gap oxygen partial pressure"]].setFinalValue(gap_oxigen_partial_pressure); // (atm)
    
  reference = "Lewis et al. JNM 227 (1995) 83-109, D.R. Olander, Nucl. Technol. 74 (1986) 215.";
  model[model_index].setRef(reference);
}