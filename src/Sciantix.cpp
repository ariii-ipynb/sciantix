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

#include "Sciantix.h"

void Sciantix(int Sciantix_options[],
	double Sciantix_history[],
	double Sciantix_variables[],
	double Sciantix_scaling_factors[],
	double Sciantix_diffusion_modes[])
{
	SetVariables(Sciantix_options, Sciantix_history, Sciantix_variables, Sciantix_scaling_factors, Sciantix_diffusion_modes);

	SetMatrix();

	SetGas();

	SetSystem();

	Simulation sciantix_simulation;

	// std::cout << "lauree" << std::endl;

	// std::cout << sciantix_variable[sv["Fabrication porosity"]].getFinalValue() << std::endl;

	Burnup(); 									  									MapModel();
	sciantix_simulation.Burnup();

	EffectiveBurnup();                             	MapModel();
	sciantix_simulation.EffectiveBurnup();

	//std::cout << "HBD Alessio Magni" << std::endl;

	Densification();																MapModel();
	sciantix_simulation.Densification();

	Porosity();																		  MapModel();

	GrainGrowth();                                 	MapModel();
	sciantix_simulation.GrainGrowth();

	GrainBoundarySweeping();                       	MapModel();
	sciantix_simulation.GrainBoundarySweeping();

	GasProduction();                            		MapModel();
	sciantix_simulation.GasProduction();

	sciantix_simulation.GasDecay();

	IntraGranularBubbleEvolution();          				MapModel();
	sciantix_simulation.IntraGranularBubbleBehaviour();

	GasDiffusion();                                	MapModel();
	sciantix_simulation.GasDiffusion();

	//std::cout << "HBD Alessio Magni" << std::endl;
	sciantix_simulation.AthermalRelease();

	//std::cout << "HBD Alessio Magni" << std::endl;

	SolidSwelling();																MapModel();
	sciantix_simulation.SolidSwelling();

	GrainBoundaryMicroCracking();                  	MapModel();
	sciantix_simulation.GrainBoundaryMicroCracking();

	GrainBoundaryVenting();                        	MapModel();
	sciantix_simulation.GrainBoundaryVenting();

	InterGranularBubbleEvolution();                	MapModel();
	sciantix_simulation.InterGranularBubbleBehaviour();	

	/*std::cout << sciantix_variable[sv["Solid swelling"]].getFinalValue() << std::endl;*/
	/*std::cout << sciantix_variable[sv["Intragranular gas swelling"]].getFinalValue() << std::endl;*/
	/*std::cout << history_variable[hv["Fission rate"]].getFinalValue() << std::endl;*/

	DensityUpdate();                             		MapModel();

	FiguresOfMerit();

	UpdateVariables(Sciantix_variables, Sciantix_diffusion_modes);

	Output();

	history_variable.clear();
	sciantix_variable.clear();
	sciantix_system.clear();
	physics_variable.clear();
	model.clear();
	material.clear();
	gas.clear();
	matrix.clear();
}
