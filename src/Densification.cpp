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

#include "Densification.h"

void Densification()
{	
	/**
	 * @brief This function defines the sciantix model *Densification*.
	 * 
	 *
	 * @author Arianna Pagani
	 */ 

	model.emplace_back();
	int model_index = int(model.size()) - 1;
	model[model_index].setName("Densification");

	std::vector<double> parameter;

	switch (int(input_variable[iv["iDensification"]].getValue()))
	{
		case 0:
		{
			parameter.push_back(0);
			parameter.push_back(0);	/*Source term*/
		}		
		break;
		case 1:
		{	
			parameter.push_back(2);	/*Decay rate*/
			parameter.push_back(0.006*exp(0.002*history_variable[hv["Temperature"]].getFinalValue()));	/*Source term*/
		}
		break;
	}


	model[model_index].setParameter(parameter);
	model[model_index].setRef("Megafit da PVU PhD");
}



