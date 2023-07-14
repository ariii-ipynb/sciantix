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
//  Authors: G. Zullo, G. Petrosillo                                                //
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

class H2O_decomposition_equilibrium_constant 
{
  public:
    void K_H2O_Olander(double temperature)
    {
      K_H2O = exp(-26200 / temperature + 6.032);
    }

    void K_H2O_Olander_1986(double temperature)
    {
      K_H2O = exp(-30165.985 / temperature + 6.95);
    }

    void K_H2O_Kubaschewski(double temperature)
    {
      K_H2O = exp(0.9797 * log(temperature) - 1.1128 - 28827 / temperature);
    }

    void K_H2O_Ulich(double temperature)
    { 
      /// @ref Morel et al., CEA, Report NT/DTP/SECC no. DR94-55 (1994)
      K_H2O = 84.945 * exp(-25300 / temperature + 7.28e-4 * temperature);
    }

    double get_KH2O()
    {
      return K_H2O;
    }

  private:
    double K_H2O;
};
