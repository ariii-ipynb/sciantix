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

#include "Burnup.h"

void Burnup()
{	
	/**
	 * @brief This function defines the sciantix model *Burnup*.
	 * 
	 * The model Burnup is used to evaluate the local fuel burnup according to the local power density, evaluated from the local fission rate density.
	 * The calculation is executed in void Simulation::BurnupEvolution(), by using the Solver::Integrator.
	 * 
	 * ....................................................................................................................................................
	 * 
	 * !! Note on the evaluation of the bu when ρf != const !!
	 * 
	 * If one decides to include the update of the fuel density, affected by the ever evolving presence of fission products, gaseous and solid - 
	 * giving rise to gaseous and solid swelling, which affect, respectively, the porosity and the solid density - one must do so with caution.
	 * Indeed, due to the structure of this code and its simplifications, if the density, by any means, changes, the value of burn up will change as well.
	 * As this model well states, the burnup can be evaluated from the specific power as:
	 * bu = q[W/kg] * Δt [s] * c where c is a corrective factor which takes into accound the conversion from Ws/kg to MWd/kg (c = 1e-6 [MW/W] * 1/86400 [d/s])
	 * Furthermore, the specific power q is computed as:
	 * q = F * E_fiss / ρf
	 * Clearly, if one keeps the fission rate constant, the specific power will only change according to the evolution of the density, despite the fact that
	 * the latter is deeply connected to the change in the fission rate.
	 * Clearly:
	 * F = Σf * φ = σf * Nf * φ 
	 * Re-writing Nf [at/m3] in terms of the density:
	 * Nf = ρf * MM * N_av
	 * In this way the dependence of q on ρ would re-balance itself out
	 * Yet, introducing the evaluation of the neutronics of the system would be complicated and uncomfortable.
	 * Thus, we can just accept the fact that the burnup will - to some extent - be affected by the way one computes the evolution of the density and, hence, dependent on it.
	 * An alternative pathway, would be, perhaps, to couple Sciantix with a neutronic code which could provide the value of the fission rate at each time step, accounting for the change
	 * in the massic density.  
 	 */

	model.emplace_back();
	int model_index = int(model.size()) - 1;
	model[model_index].setName("Burnup");

	sciantix_variable[sv["Specific power"]].setFinalValue((history_variable[hv["Fission rate"]].getFinalValue() * (3.12e-17) / sciantix_variable[sv["Fuel density"]].getFinalValue()));
	
	std::string reference = "The local burnup is calculated from the fission rate density.";
	std::vector<double> parameter;
	parameter.push_back(sciantix_variable[sv["Specific power"]].getFinalValue() / 86400.0); // conversion to get burnup in MWd/kg

	model[model_index].setParameter(parameter);
	model[model_index].setRef(reference);



}
