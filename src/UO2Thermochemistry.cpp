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

#include "UO2Thermochemistry.h"

void UO2Thermochemistry()
{
  /**
   * @brief This routine defines the model to evaluate the oxygen partial pressure (in atm) in hyperstoichiometric UO2+x fuel
   * as a function of:
   * 
   * @param[in] T tuel temperature (K)
   * @param[in] PO2_x oxygen partial pressure in UO2+x (in atm) 
   * from Blackburn’s relation, @ref *Blackburn (1973) J. Nucl. Mater., 46, 244–252*
   * 
   * @param[out] equilibrium_stoichiometry_deviation
   */

	if (!input_variable[iv["iStoichiometryDeviation"]].getValue()) return;

  model.emplace_back();
  int model_index = int(model.size()) - 1;

  model[model_index].setName("UO2 thermochemistry");
  
  std::string reference;
  std::vector<double> parameter;

  reference = "Blackburn (1973) J. Nucl. Mater., 46, 244-252.";

  // oxygen-to-metal ratio increase due to fission products
  sciantix_variable[sv["Stoichiometry deviation"]].setFinalValue(
    solver.Integrator(
      sciantix_variable[sv["Stoichiometry deviation"]].getInitialValue(),
      (1.4e-4 / 0.88),
      sciantix_variable[sv["Burnup"]].getIncrement()
    )
  );

  // Equilibrium stoichiometry - Blackburn
  // sciantix_variable[sv["Equilibrium stoichiometry deviation"]].setFinalValue(
  //   solver.equilibriumStoiochiometryBlackburn(
  //     history_variable[hv["Temperature"]].getFinalValue(),
  //     sciantix_variable[sv["Gap oxygen partial pressure"]].getFinalValue(),
  //     sciantix_variable[sv["Stoichiometry deviation"]].getFinalValue()
  //   )
  // );

  // Parameters for Blackburn model
  // double entropic_coeff = -9.92;
  // double x_coeff = 108;
  // double enthalpic_coeff = -32700;

  // Equilibrium stoichiometry - Lindemer and Besmann
  sciantix_variable[sv["Equilibrium stoichiometry deviation"]].setFinalValue(
    solver.equilibriumStoiochiometryLindemerBesmann(
      history_variable[hv["Temperature"]].getFinalValue(),
      sciantix_variable[sv["Gap oxygen partial pressure"]].getFinalValue(),
      sciantix_variable[sv["Stoichiometry deviation"]].getFinalValue()
    )
  );

  // Parameters for Lindemer and Besmann model
  double entropic_coeff = -126/8.314;
  double x_coeff = 0.0;
  double enthalpic_coeff = -312807/8.314;

  parameter.push_back(entropic_coeff);
  parameter.push_back(x_coeff);
  parameter.push_back(enthalpic_coeff);

  model[model_index].setParameter(parameter);
  model[model_index].setRef(reference);
}

double oxygenPotentialBlackburn(double x, double T)
{
  double entropic_coeff = -9.92;
  double x_coeff = 108;
  double enthalpic_coeff = -32700;

  return exp(-entropic_coeff + x_coeff * pow(x, 2) + 2 * log(x * (2 + x) / (1 - x)) + enthalpic_coeff / T);
}

double oxygenPotentialLindemerBesmann(double x, double T)
{
  double entropic_coeff = -126/8.314;
  // double x_coeff = 0.0;c
  double enthalpic_coeff = -312807/8.314;

  return exp(enthalpic_coeff / T - entropic_coeff + 2 * log((x * pow((1 - 2*x),2)) * pow((1 - 3*x),-3)));
}