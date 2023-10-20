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

	/*sciantix_variable[sv["Open porosity"]].setFinalValue(openPorosity(sciantix_variable[sv["Fabrication porosity"]].getFinalValue()));*/ 

	sciantix_variable[sv["Athermal venting factor"]].setFinalValue(athermalVentingFactor(sciantix_variable[sv["Open porosity"]].getFinalValue()));

	if (history_variable[hv["Time"]].getFinalValue() == 0)
		{sciantix_variable[sv["Open porosity"]].setFinalValue(openPorosity(sciantix_variable[sv["Fabrication porosity"]].getFinalValue()));
		}
	parameter.push_back(athermalVentingFactor(sciantix_variable[sv["Open porosity"]].getFinalValue()));

	model[model_index].setParameter(parameter);
	model[model_index].setRef("Claisse et al. Journal of Nuclear Materials 466 (2015) 351-356.");
}

double openPorosity(double fabrication_porosity)
{
	/*
	 *
	 * @brief This function evaluates the sciantix_variable "Open porosity" according to the value of fabrication porosity 
	 * @ref **Claisse et al. Journal of Nuclear Materials 466 (2015) 351-356**
	 * 
	 * @author A. Pagani
	 * 
	 */
	switch (int(input_variable[iv["iPorosity"]].getValue()))
	{
		case 0:
		return(0);
		break;
	
		case 1:
		{
			if (fabrication_porosity <= 1.0)
			{
				const bool check1 = (fabrication_porosity < 0.050) ? 1 : 0;
				const bool check2 = (fabrication_porosity > 0.058) ? 1 : 0;

				return(
					(fabrication_porosity / 20) * (check1) + 
					(3.10 * fabrication_porosity - 0.1525) * (!check1 && !check2) + 
					(fabrication_porosity / 2.1 - 3.2e-4) * (check2)
				);
			}
			else
			{
				std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
				return(0);
			}
		}	
		break;

		case 2:
		{
			if (fabrication_porosity <= 1.0)
			{
        double x1_step1[3] = {0.0309254289000001, 50.5545836563236, -1};
        double b1 = -0.76352583531298012787;
        double IW1_1 = 2.4890484320254797623;
        double b2 = 0.030657996141059989936;
        double LW2_1 = 1.003372825212876851;
        double y1_step1[3] = {-1, 63.2911366167282, 0.00091428483};
    

        fabrication_porosity = (fabrication_porosity - x1_step1[0]) * x1_step1[1] + x1_step1[2];
				double n = b1 + IW1_1 * fabrication_porosity;
				double a1 = 2 / (1 + exp(-2 * n)) - 1;
				double a2 = b2 + LW2_1 * a1;
				double p_open = (a2 - y1_step1[0]) / y1_step1[1] + y1_step1[2];

				return(p_open);
			}
			else
			{
				std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
				return(0);
			}
		}	
		break;

		default:
			ErrorMessages::Switch("Porosity.cpp", "iPorosity", int(input_variable[iv["iPorosity"]].getValue()));
			break;

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
