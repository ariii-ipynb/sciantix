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
//  Authors: D. Pizzocri, G. Zullo.                                                 //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#include "Porosity.h"

void Porosity()
{	
	/**
	 * @brief This function defines the sciantix model *Porosity*.
	 * 
	 * The model Porosity is used to evaluate 
	 * 
	 * - open porosity
	 * - athermal venting factor (fraction of grain surface connected to open poristy)
	 * 
	 * according to fabrication porosity
	 * 
	 */

	model.emplace_back();
	int model_index = int(model.size()) - 1;
	model[model_index].setName("Porosity");

	std::vector<double> parameter;

	sciantix_variable[sv["Open porosity"]].setFinalValue(openPorosity(sciantix_variable[sv["Fabrication porosity"]].getFinalValue()));
	sciantix_variable[sv["Athermal venting factor"]].setFinalValue(athermalVentingFactor(sciantix_variable[sv["Open porosity"]].getFinalValue()));

	parameter.push_back(athermalVentingFactor(sciantix_variable[sv["Open porosity"]].getFinalValue()));

	model[model_index].setParameter(parameter);
	model[model_index].setRef("Claisse et al. Journal of Nuclear Materials 466 (2015) 351-356.");
}

double openPorosity(double fabrication_porosity)
{
	/**
	 * @brief This function evaluates the sciantix_variable "Open porosity" according to the value of fabrication porosity 
	 * @ref **Claisse et al. Journal of Nuclear Materials 466 (2015) 351-356**
	 * 
	 * @author A. Pagani
	 * 
	 */

	if (fabrication_porosity <= 1.0)
	{
		const bool check1 = (fabrication_porosity < 0.050) ? 1 : 0;
		const bool check2 = (fabrication_porosity > 0.058) ? 1 : 0;

		return fabrication_porosity * (
			(fabrication_porosity / 20) * (check1) + 
			(3.10 * fabrication_porosity - 0.1525) * (!check1 && !check2) + 
			(fabrication_porosity / 2.1 - 3.2e-4) * (check2)
		);
	}
	else if (fabrication_porosity > 1)
	{
		std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
		return 0;
	}
}

double athermalVentingFactor(double open_p)
{
	/**
	 * @brief This function has the sole purpose of evaluating the ratio between the open surface of the grain edge and the total one.
	 * @ref **Claisse et al. Journal of Nuclear Materials 466 (2015) 351-356**
	 * 
	 * @author A. Pagani
	 * 
	 */
	if (open_p !=0){
		double athermal_venting = 1.54 * sqrt(open_p);
	  return athermal_venting;
	}
	else return 0; 	
}
