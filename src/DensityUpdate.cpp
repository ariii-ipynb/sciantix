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
//  Authors: D. Pizzocri, G. Zullo, A. Magni, E. Redaelli                           //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#include "DensityUpdate.h"

void DensityUpdate()
{
	/**
	 * @brief This function defines the sciantix model *DensityUpdate*.
	 * 
	 * The model *DensityUpdate* is used to evaluate the value of density considering a the evolution of porosity during irradiation.
	 * 
	 */

	model.emplace_back();
	int model_index = int(model.size()) - 1;

	model[model_index].setName("Density Update");

	/*sciantix_variable[sv["Fuel density"]].setFinalValue(
	sciantix_variable[sv["Solid density"]].getFinalValue() / (1.0 + sciantix_variable[sv["Porosity"]].getFinalValue())
	);*/

}
