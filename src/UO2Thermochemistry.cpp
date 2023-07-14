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
   * @param[in] stoichiometry_deviation
   * @param[in] temperature
   * 
   * @param[out] PO2_x oxygen partial pressure in UO2+x (in atm) 
   * from Blackburn’s relation, @ref *Blackburn (1973) J. Nucl. Mater., 46, 244–252*
   * 
   */

	if (!input_variable[iv["iStoichiometryDeviation"]].getValue()) return;

  model.emplace_back();
  int model_index = int(model.size()) - 1;

  model[model_index].setName("UO2 thermochemistry");
  
  std::string reference;
  std::vector<double> parameter;

  reference = "Blackburn (1973) J. Nucl. Mater., 46, 244-252.";

  OxygenPotentialRepresentation equilibrium_stoichiometry;


  sciantix_variable[sv["Equilibrium stoichiometry deviation"]].setFinalValue(
    
  );

  parameter.push_back(sciantix_variable[sv["Stoichiometry deviation"]].getInitialValue());

  model[model_index].setParameter(parameter);
  model[model_index].setRef(reference);
}

class OxygenPotentialRepresentation
{
  public:
    double temperature;
    double oxygen_gap_pressure;
    double equilibrium_stoichiometry;

    double equilibrium_stoichiometry_Blackburn(double temperature, double pressure, double x)
    {
      double entropic_coeff = -9.92;
      double x_coeff = 108;
      double enthalpic_coeff = -32700;

      double fun(0.0);
      double deriv(0.0);
      double x1(0.0);
      unsigned short int iter(0);
      const double tol(1.0e-3);
      const unsigned short int max_iter(50);
    
      if(pressure==0)
	  		std::cout << "Warning: Check equilibrium_stoichiometry_Blackburn!" << std::endl;
    
      if(x == 0.0)
        x = 1.0e-7;

      while (iter < max_iter)
      {
        fun =  -entropic_coeff + x_coeff * pow(x, 2) + 2 * log(x * (2 + x) / (1 - x)) + enthalpic_coeff / temperature - log(pressure);

        deriv = x_coeff + 2.0 / x + 2 / (1.0 - x);

        x1 = x - fun/deriv;
        x = x1;

        if(abs(fun)<tol)
          return x1;

        iter++;
      }
    }
};