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

#ifndef SIMULATION_H
#define SIMULATION_H

#include <cmath>
#include <vector>
#include "Solver.h"
#include "Model.h"
#include "MapModel.h"
#include "MapSciantixVariable.h"
#include "MapPhysicsVariable.h"
#include "HistoryVariableDeclaration.h"
#include "SciantixVariableDeclaration.h"
#include "ModelDeclaration.h"
#include "SolverDeclaration.h"
#include "PhysicsVariableDeclaration.h"
#include "GasDeclaration.h"
#include "MapGas.h"
#include "HighBurnupStructureFormation.h"
#include "HighBurnupStructurePorosity.h"
#include "SciantixDiffusionModeDeclaration.h"
#include "SystemDeclaration.h"
#include "MapSystem.h"
#include "MatrixDeclaration.h"
#include "MapMatrix.h"
#include "ConstantNumbers.h"
#include "DensityUpdate.h"

/// @brief
/// Derived class representing the operations of SCIANTIX. The conjunction of the models with the implemented solvers results in the simulation.

class Simulation : public Solver, public Model
{
public:
	void Burnup()
	{

		const double N_av = CONSTANT_NUMBERS_H::PhysicsConstants::avogadro_number;
		const double E_fiss = CONSTANT_NUMBERS_H::ConstantParameters::E_fiss;
		const double M_U = CONSTANT_NUMBERS_H::ConstantParameters::MM_U235;
	

		/// @brief Burnup uses the solver Integrator to computes the fuel burnup from the local power density.
		/// This method is called in Sciantix.cpp, after the definition of the Burnup model.
		sciantix_variable[sv["Burnup"]].setFinalValue(
			solver.Integrator(
				sciantix_variable[sv["Burnup"]].getInitialValue(),
				model[sm["Burnup"]].getParameter().at(0),
				physics_variable[pv["Time step"]].getFinalValue()
			)
		);

		sciantix_variable[sv["FIMA"]].setFinalValue(
			sciantix_variable[sv["Burnup"]].getFinalValue() / 9.5
		);

	
		if(history_variable[hv["Fission rate"]].getFinalValue() > 0.0)
			sciantix_variable[sv["Irradiation time"]].setFinalValue(
				solver.Integrator(
						sciantix_variable[sv["Burnup"]].getInitialValue(),
						model[sm["Burnup"]].getParameter().at(0),
						physics_variable[pv["Time step"]].getFinalValue()));
			

		if (history_variable[hv["Fission rate"]].getFinalValue() > 0.0)
			sciantix_variable[sv["Irradiation time"]].setFinalValue(
					solver.Integrator(
							sciantix_variable[sv["Irradiation time"]].getInitialValue(),
							1.0 / sciantix_variable[sv["Specific power"]].getFinalValue(),
							24.0 * sciantix_variable[sv["Burnup"]].getIncrement()));
		else
			sciantix_variable[sv["Irradiation time"]].setConstant();
	}

	void EffectiveBurnup()
	{
		/// @brief EffectiveBurnup uses the solver Integrator to computes the effective burnup of the fuel, if the
		/// criteria on the temperature are required.
		/// This method is called in Sciantix.cpp after the definition of the effective burnup model.
		sciantix_variable[sv["Effective burnup"]].setFinalValue(
				solver.Integrator(
						sciantix_variable[sv["Effective burnup"]].getInitialValue(),
						model[sm["Effective burnup"]].getParameter().at(0),
						physics_variable[pv["Time step"]].getFinalValue()));
	}

	void HighBurnupStructureFormation()
	{
		/// @brief
		/// HighBurnupStructureEvolution
		/// HighBurnupStructureEvolution is used to compute the local restructured volume fraction of the fuel,
		/// in the HBS region.
		/// This method is called in Sciantix.cpp after the definition of the model HighBurnupStructureFormation.

		if (!int(input_variable[iv["iHighBurnupStructureFormation"]].getValue()))
			return;

		double coefficient =
				model[sm["High burnup structure formation"]].getParameter().at(0) *
				model[sm["High burnup structure formation"]].getParameter().at(1) *
				pow(sciantix_variable[sv["Effective burnup"]].getFinalValue(), 2.54);

		sciantix_variable[sv["Restructured volume fraction"]].setFinalValue(
				solver.Decay(
						sciantix_variable[sv["Restructured volume fraction"]].getInitialValue(),
						coefficient,
						coefficient,
						sciantix_variable[sv["Effective burnup"]].getIncrement()));
	}

	void GasProduction()
	{
		/**
		 * @brief GasProduction computes the gas produced from the production rate.
		 *
		 */

		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].setFinalValue(
					solver.Integrator(
							sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getInitialValue(),
							model[sm["Gas production - " + sciantix_system[i].getGasName() + " in " + matrix[0].getName()]].getParameter().at(0),
							model[sm["Gas production - " + sciantix_system[i].getGasName() + " in " + matrix[0].getName()]].getParameter().at(1)));
		}
	}

	void GasDecay()
	{
		/// @brief
		/// Gas Decay
		// This for loop slides over all the considered gases (both stable and radioactive ones)
		// but computes the decayed concentration only of radioactive isotopes
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() > 0.0)
			{
				sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].setFinalValue(
						solver.Decay(
								sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].getInitialValue(),
								gas[ga[sciantix_system[i].getGasName()]].getDecayRate(),
								gas[ga[sciantix_system[i].getGasName()]].getDecayRate() * sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getFinalValue(),
								physics_variable[pv["Time step"]].getFinalValue()));
			}
		}
	}

	void XeDiffusion()
	{
		if (int(input_variable[iv["iDiffusionSolver"]].getValue()) == 1)
		{
			sciantix_variable[sv["Xe in grain"]].setFinalValue(
					solver.SpectralDiffusion(
							xe_diffusion_modes,
							model[sm["Gas diffusion - Xe in " + matrix[0].getName()]].getParameter(),
							physics_variable[pv["Time step"]].getFinalValue()));

			double equilibrium_fraction(1.0);
			if ((sciantix_system[sy["Xe in UO2"]].getResolutionRate() + sciantix_system[sy["Xe in UO2"]].getTrappingRate()) > 0.0)
				equilibrium_fraction = sciantix_system[sy["Xe in UO2"]].getResolutionRate() / (sciantix_system[sy["Xe in UO2"]].getResolutionRate() + sciantix_system[sy["Xe in UO2"]].getTrappingRate());

			sciantix_variable[sv["Xe in intragranular solution"]].setFinalValue(
					equilibrium_fraction * sciantix_variable[sv["Xe in grain"]].getFinalValue());

			sciantix_variable[sv["Xe in intragranular bubbles"]].setFinalValue(
					(1.0 - equilibrium_fraction) * sciantix_variable[sv["Xe in grain"]].getFinalValue());
		}

		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			double initial_value_solution = sciantix_variable[sv["Xe in intragranular solution"]].getFinalValue();
			double initial_value_bubbles = sciantix_variable[sv["Xe in intragranular bubbles"]].getFinalValue();

			solver.SpectralDiffusionNonEquilibrium(
					initial_value_solution,
					initial_value_bubbles,
					xe_diffusion_modes_solution,
					xe_diffusion_modes_bubbles,
					model[sm["Gas diffusion - Xe in " + matrix[0].getName()]].getParameter(),
					physics_variable[pv["Time step"]].getFinalValue());

			sciantix_variable[sv["Xe in intragranular solution"]].setFinalValue(initial_value_solution);
			sciantix_variable[sv["Xe in intragranular bubbles"]].setFinalValue(initial_value_bubbles);
			sciantix_variable[sv["Xe in grain"]].setFinalValue(initial_value_solution + initial_value_bubbles);
		}
	}

	void Xe133Diffusion()
	{
		if (input_variable[iv["iDiffusionSolver"]].getValue() == 1)
		{
			sciantix_variable[sv["Xe133 in grain"]].setFinalValue(
					solver.SpectralDiffusion(
							xe133_diffusion_modes,
							model[sm["Gas diffusion - Xe133 in UO2"]].getParameter(),
							physics_variable[pv["Time step"]].getFinalValue()));

			double equilibrium_fraction(1.0);
			if ((sciantix_system[sy["Xe133 in UO2"]].getResolutionRate() + sciantix_system[sy["Xe133 in UO2"]].getTrappingRate()) > 0.0)
				equilibrium_fraction = sciantix_system[sy["Xe133 in UO2"]].getResolutionRate() / (sciantix_system[sy["Xe133 in UO2"]].getResolutionRate() + sciantix_system[sy["Xe133 in UO2"]].getTrappingRate());

			sciantix_variable[sv["Xe133 in intragranular solution"]].setFinalValue(
					equilibrium_fraction * sciantix_variable[sv["Xe133 in grain"]].getFinalValue());

			sciantix_variable[sv["Xe133 in intragranular bubbles"]].setFinalValue(
					(1 - equilibrium_fraction) * sciantix_variable[sv["Xe133 in grain"]].getFinalValue());
		}

		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			double initial_value_solution = sciantix_variable[sv["Xe133 in intragranular solution"]].getFinalValue();
			double initial_value_bubbles = sciantix_variable[sv["Xe133 in intragranular bubbles"]].getFinalValue();

			solver.SpectralDiffusionNonEquilibrium(
					initial_value_solution,
					initial_value_bubbles,
					xe133_diffusion_modes_solution,
					xe133_diffusion_modes_bubbles,
					model[sm["Gas diffusion - Xe133 in UO2"]].getParameter(),
					physics_variable[pv["Time step"]].getFinalValue());

			sciantix_variable[sv["Xe133 in intragranular solution"]].setFinalValue(initial_value_solution);
			sciantix_variable[sv["Xe133 in intragranular bubbles"]].setFinalValue(initial_value_bubbles);
			sciantix_variable[sv["Xe133 in grain"]].setFinalValue(initial_value_solution + initial_value_bubbles);
		}
	}

	void Kr85mDiffusion()
	{
		if (input_variable[iv["iDiffusionSolver"]].getValue() == 1)
		{
			sciantix_variable[sv["Kr85m in grain"]].setFinalValue(
					solver.SpectralDiffusion(
							kr85m_diffusion_modes,
							model[sm["Gas diffusion - Kr85m in UO2"]].getParameter(),
							physics_variable[pv["Time step"]].getFinalValue()));

			double equilibrium_fraction(1.0);
			if ((sciantix_system[sy["Kr85m in UO2"]].getResolutionRate() + sciantix_system[sy["Kr85m in UO2"]].getTrappingRate()) > 0.0)
				equilibrium_fraction = sciantix_system[sy["Kr85m in UO2"]].getResolutionRate() / (sciantix_system[sy["Kr85m in UO2"]].getResolutionRate() + sciantix_system[sy["Kr85m in UO2"]].getTrappingRate());

			sciantix_variable[sv["Kr85m in intragranular solution"]].setFinalValue(
					equilibrium_fraction * sciantix_variable[sv["Kr85m in grain"]].getFinalValue());

			sciantix_variable[sv["Kr85m in intragranular bubbles"]].setFinalValue(
					(1 - equilibrium_fraction) * sciantix_variable[sv["Kr85m in grain"]].getFinalValue());
		}

		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			double initial_value_solution = sciantix_variable[sv["Kr85m in intragranular solution"]].getFinalValue();
			double initial_value_bubbles = sciantix_variable[sv["Kr85m in intragranular bubbles"]].getFinalValue();

			solver.SpectralDiffusionNonEquilibrium(
					initial_value_solution,
					initial_value_bubbles,
					kr85m_diffusion_modes_solution,
					kr85m_diffusion_modes_bubbles,
					model[sm["Gas diffusion - Kr85m in UO2"]].getParameter(),
					physics_variable[pv["Time step"]].getFinalValue());

			sciantix_variable[sv["Kr85m in intragranular solution"]].setFinalValue(initial_value_solution);
			sciantix_variable[sv["Kr85m in intragranular bubbles"]].setFinalValue(initial_value_bubbles);
			sciantix_variable[sv["Kr85m in grain"]].setFinalValue(initial_value_solution + initial_value_bubbles);
		}
	}

	void KrDiffusion()
	{
		if (input_variable[iv["iDiffusionSolver"]].getValue() == 1)
		{
			sciantix_variable[sv["Kr in grain"]].setFinalValue(
					solver.SpectralDiffusion(
							kr_diffusion_modes,
							model[sm["Gas diffusion - Kr in " + matrix[0].getName()]].getParameter(),
							physics_variable[pv["Time step"]].getFinalValue()));

			double equilibrium_fraction(1.0);
			if ((sciantix_system[sy["Kr in UO2"]].getResolutionRate() + sciantix_system[sy["Kr in UO2"]].getTrappingRate()) > 0.0)
				equilibrium_fraction = sciantix_system[sy["Kr in UO2"]].getResolutionRate() / (sciantix_system[sy["Kr in UO2"]].getResolutionRate() + sciantix_system[sy["Kr in UO2"]].getTrappingRate());

			sciantix_variable[sv["Kr in intragranular solution"]].setFinalValue(
					equilibrium_fraction * sciantix_variable[sv["Kr in grain"]].getFinalValue());

			sciantix_variable[sv["Kr in intragranular bubbles"]].setFinalValue(
					(1 - equilibrium_fraction) * sciantix_variable[sv["Kr in grain"]].getFinalValue());
		}

		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			double initial_value_solution = sciantix_variable[sv["Kr in intragranular solution"]].getFinalValue();
			double initial_value_bubbles = sciantix_variable[sv["Kr in intragranular bubbles"]].getFinalValue();

			solver.SpectralDiffusionNonEquilibrium(
					initial_value_solution,
					initial_value_bubbles,
					kr_diffusion_modes_solution,
					kr_diffusion_modes_bubbles,
					model[sm["Gas diffusion - Kr in " + matrix[0].getName()]].getParameter(),
					physics_variable[pv["Time step"]].getFinalValue());

			sciantix_variable[sv["Kr in intragranular solution"]].setFinalValue(initial_value_solution);
			sciantix_variable[sv["Kr in intragranular bubbles"]].setFinalValue(initial_value_bubbles);
			sciantix_variable[sv["Kr in grain"]].setFinalValue(initial_value_solution + initial_value_bubbles);
		}
	}

	/**
	 * @brief ### Intragranular helium diffusion
	 *
	 */
	void HeDiffusion()
	{
		/**
		 * @brief iDiffusionSolver = 1
		 *
		 */
		if (input_variable[iv["iDiffusionSolver"]].getValue() == 1)
		{
			sciantix_variable[sv["He in grain"]].setFinalValue(
					solver.SpectralDiffusion(
							he_diffusion_modes,
							model[sm["Gas diffusion - He in UO2"]].getParameter(),
							physics_variable[pv["Time step"]].getFinalValue()));

			double equilibrium_fraction(1.0);
			if ((sciantix_system[sy["He in UO2"]].getResolutionRate() + sciantix_system[sy["He in UO2"]].getTrappingRate()) > 0.0)
				equilibrium_fraction = sciantix_system[sy["He in UO2"]].getResolutionRate() / (sciantix_system[sy["He in UO2"]].getResolutionRate() + sciantix_system[sy["He in UO2"]].getTrappingRate());

			sciantix_variable[sv["He in intragranular solution"]].setFinalValue(equilibrium_fraction * sciantix_variable[sv["He in grain"]].getFinalValue());
			sciantix_variable[sv["He in intragranular bubbles"]].setFinalValue((1.0 - equilibrium_fraction) * sciantix_variable[sv["He in grain"]].getFinalValue());
		}

		/**
		 * @brief iDiffusionSolver = 2
		 *
		 */
		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			double initial_value_solution = sciantix_variable[sv["He in intragranular solution"]].getFinalValue();
			double initial_value_bubbles = sciantix_variable[sv["He in intragranular bubbles"]].getFinalValue();

			solver.SpectralDiffusionNonEquilibrium(
					initial_value_solution,
					initial_value_bubbles,
					he_diffusion_modes_solution,
					he_diffusion_modes_bubbles,
					model[sm["Gas diffusion - He in UO2"]].getParameter(),
					physics_variable[pv["Time step"]].getFinalValue());

			sciantix_variable[sv["He in intragranular solution"]].setFinalValue(initial_value_solution);
			sciantix_variable[sv["He in intragranular bubbles"]].setFinalValue(initial_value_bubbles);
			sciantix_variable[sv["He in grain"]].setFinalValue(initial_value_solution + initial_value_bubbles);
		}

		if (sciantix_variable[sv["He in intragranular bubbles"]].getInitialValue() > 0.0)
			sciantix_variable[sv["Intragranular similarity ratio"]].setFinalValue(sqrt(sciantix_variable[sv["He in intragranular bubbles"]].getFinalValue() / sciantix_variable[sv["He in intragranular bubbles"]].getInitialValue()));
		else
			sciantix_variable[sv["Intragranular similarity ratio"]].setFinalValue(0.0);
	}

	void GasDiffusion()
	{
		/// @brief
		/// GasDiffusion
		/// This simulation method solves the PDE for the intra-granular gas diffusion within the (ideal) spherical fuel grain.

		XeDiffusion();
		Xe133Diffusion();
		KrDiffusion();
		HeDiffusion();
		Kr85mDiffusion();

		// Calculation of the gas concentration arrived at the grain boundary, by mass balance.
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].setFinalValue(
					sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " in grain"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " released"]].getInitialValue());

			if (sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getFinalValue() < 0.0)
				sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].setFinalValue(0.0);
		}

		/**
		 * @brief If **iGrainBoundaryBehaviour = 0** (e.g., grain-boundary calculations are neglected),
		 * all the gas arriving at the grain boundary is released.
		 *
		 */
		if (input_variable[iv["iGrainBoundaryBehaviour"]].getValue() == 0)
		{
			for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			{
				sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].setFinalValue(0.0);

				sciantix_variable[sv[sciantix_system[i].getGasName() + " released"]].setFinalValue(
						sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getFinalValue() -
						sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].getFinalValue() -
						sciantix_variable[sv[sciantix_system[i].getGasName() + " in grain"]].getFinalValue());
			}
		}
	}

	void Densification()
	{

		double dens_factor = solver.Decay(
				sciantix_variable[sv["Densification factor"]].getInitialValue(),
				model[sm["Densification"]].getParameter().at(0),
				model[sm["Densification"]].getParameter().at(1),
				sciantix_variable[sv["Burnup"]].getIncrement());

		if (dens_factor < 1)
			sciantix_variable[sv["Densification factor"]].setFinalValue(dens_factor);
		else
			sciantix_variable[sv["Densification factor"]].setFinalValue(1);

		// std::cout << model[sm["Densification"]].getParameter().at(0) << std::endl;
		// std::cout << model[sm["Densification"]].getParameter().at(1) << std::endl;
		// std::cout << sciantix_variable[sv["Densification factor"]].getFinalValue() << std::endl;

		sciantix_variable[sv["Fabrication porosity"]].setFinalValue(sciantix_variable[sv["Fabrication porosity"]].getFinalValue() *
																																(1 - sciantix_variable[sv["Densification factor"]].getFinalValue()));

		sciantix_variable[sv["Open porosity"]].setFinalValue(sciantix_variable[sv["Open porosity"]].getFinalValue() *
																												 (1 - sciantix_variable[sv["Densification factor"]].getFinalValue()));
	}

	void AthermalRelease()
	{
		// Gas is vented by subtracting a fraction of the gas concentration at grain boundaries arrived from diffusion
		// Bf = Bf - f_ath * dB
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].setFinalValue(
					solver.Integrator(
							sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getFinalValue(),
							-sciantix_variable[sv["Athermal venting factor"]].getFinalValue(),
							sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getIncrement()));
		}


		// std::cout << sciantix_variable[sv["Athermal venting factor"]].getFinalValue() << std::endl;

	}

	void GrainGrowth()
	{
		/**
		 * @brief ### GrainGrowth
		 *
		 */
		sciantix_variable[sv["Grain radius"]].setFinalValue(
				solver.QuarticEquation(model[sm["Grain growth"]].getParameter()));
	}

	void IntraGranularBubbleBehaviour()
	{
		/**
		 * @brief IntraGranularBubbleBehaviour is a method of the object Simulation.
		 * This method computes concentration and average size of intragranular gas bubbles.
		 *
		 */

		// dN / dt = - getParameter().at(0) * N + getParameter().at(1)
		sciantix_variable[sv["Intragranular bubble concentration"]].setFinalValue(
				solver.Decay(
						sciantix_variable[sv["Intragranular bubble concentration"]].getInitialValue(),
						model[sm["Intragranular bubble evolution"]].getParameter().at(0),
						model[sm["Intragranular bubble evolution"]].getParameter().at(1),
						physics_variable[pv["Time step"]].getFinalValue()));

		// Atom per bubbles and bubble radius
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
			{
				if (sciantix_variable[sv["Intragranular bubble concentration"]].getFinalValue() > 0.0)
					sciantix_variable[sv["Intragranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].setFinalValue(
							sciantix_variable[sv[sciantix_system[i].getGasName() + " in intragranular bubbles"]].getFinalValue() /
							sciantix_variable[sv["Intragranular bubble concentration"]].getFinalValue());

				else
					sciantix_variable[sv["Intragranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].setFinalValue(0.0);

				sciantix_variable[sv["Intragranular bubble volume"]].addValue(
						sciantix_system[i].getVolumeInLattice() * sciantix_variable[sv["Intragranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].getFinalValue());
			}
		}

		// Intragranular bubble radius
		sciantix_variable[sv["Intragranular bubble radius"]].setFinalValue(0.620350491 * pow(sciantix_variable[sv["Intragranular bubble volume"]].getFinalValue(), (1.0 / 3.0)));

		// Swelling
		// 4/3 pi N R^3
		sciantix_variable[sv["Intragranular gas swelling"]].setFinalValue(4.188790205 *
																																			pow(sciantix_variable[sv["Intragranular bubble radius"]].getFinalValue(), 3) *
																																			sciantix_variable[sv["Intragranular bubble concentration"]].getFinalValue());

		sciantix_variable[sv["Porosity"]].addValue(sciantix_variable[sv["Intragranular gas swelling"]].getIncrement());
	}

	void InterGranularBubbleBehaviour()
	{
		const double pi = CONSTANT_NUMBERS_H::MathConstants::pi;

		// Vacancy concentration
		sciantix_variable[sv["Intergranular vacancies per bubble"]].setFinalValue(
				solver.LimitedGrowth(sciantix_variable[sv["Intergranular vacancies per bubble"]].getInitialValue(),
														 model[sm["Intergranular bubble evolution"]].getParameter(),
														 physics_variable[pv["Time step"]].getFinalValue()));

		// Bubble volume
		double vol(0);
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		// This for loop slides over only the stable gases, which determine the grain-boundary bubble dynamics
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
			{
				vol += sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].getFinalValue() *
							 gas[ga[sciantix_system[i].getGasName()]].getVanDerWaalsVolume();
			}
		}
		vol += sciantix_variable[sv["Intergranular vacancies per bubble"]].getFinalValue() * matrix[sma["UO2"]].getSchottkyVolume();
		sciantix_variable[sv["Intergranular bubble volume"]].setFinalValue(vol);

		// Bubble radius
		sciantix_variable[sv["Intergranular bubble radius"]].setFinalValue(
				0.620350491 * pow(sciantix_variable[sv["Intergranular bubble volume"]].getFinalValue() / (matrix[sma["UO2"]].getLenticularShapeFactor()), 1. / 3.));

		// Bubble area
		sciantix_variable[sv["Intergranular bubble area"]].setFinalValue(
				pi * pow(sciantix_variable[sv["Intergranular bubble radius"]].getFinalValue() * sin(matrix[sma["UO2"]].getSemidihedralAngle()), 2));

		// Coalescence
		double dbubble_area = sciantix_variable[sv["Intergranular bubble area"]].getIncrement(); // dA
		sciantix_variable[sv["Intergranular bubble concentration"]].setFinalValue(
				solver.BinaryInteraction(sciantix_variable[sv["Intergranular bubble concentration"]].getInitialValue(), 2.0, dbubble_area));

		// Conservation
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
			{
				sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].rescaleValue(
						sciantix_variable[sv["Intergranular bubble concentration"]].getInitialValue() /
						sciantix_variable[sv["Intergranular bubble concentration"]].getFinalValue());
			}
		}

		double n_at(0);
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
				n_at += sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].getFinalValue();
		}
		sciantix_variable[sv["Intergranular atoms per bubble"]].setFinalValue(n_at);

		sciantix_variable[sv["Intergranular vacancies per bubble"]].rescaleValue(
				sciantix_variable[sv["Intergranular bubble concentration"]].getInitialValue() /
				sciantix_variable[sv["Intergranular bubble concentration"]].getFinalValue());

		vol = 0.0;
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
			{
				vol += sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].getFinalValue() *
							 gas[ga[sciantix_system[i].getGasName()]].getVanDerWaalsVolume();
			}
		}
		vol += sciantix_variable[sv["Intergranular vacancies per bubble"]].getFinalValue() * matrix[sma["UO2"]].getSchottkyVolume();
		sciantix_variable[sv["Intergranular bubble volume"]].setFinalValue(vol);

		sciantix_variable[sv["Intergranular bubble radius"]].setFinalValue(
				0.620350491 * pow(sciantix_variable[sv["Intergranular bubble volume"]].getFinalValue() / (matrix[sma["UO2"]].getLenticularShapeFactor()), 1. / 3.));

		sciantix_variable[sv["Intergranular bubble area"]].setFinalValue(
				pi * pow(sciantix_variable[sv["Intergranular bubble radius"]].getFinalValue() * sin(matrix[sma["UO2"]].getSemidihedralAngle()), 2));

		// Fractional coverage
		sciantix_variable[sv["Intergranular fractional coverage"]].setFinalValue(
				sciantix_variable[sv["Intergranular bubble area"]].getFinalValue() *
				sciantix_variable[sv["Intergranular bubble concentration"]].getFinalValue());

		// Intergranular gas release
		//                          F0
		//   ___________A0____________
		//   |_________A1__________  |
		//   |                    |  |
		//   |          F1        N1 N0
		//   |                    |  |
		//   |____________________|__|
		const double similarity_ratio = sqrt(
				sciantix_variable[sv["Intergranular saturation fractional coverage"]].getFinalValue() /
				sciantix_variable[sv["Intergranular fractional coverage"]].getFinalValue());

		if (similarity_ratio < 1.0)
		{
			sciantix_variable[sv["Intergranular bubble area"]].rescaleValue(similarity_ratio);
			sciantix_variable[sv["Intergranular bubble concentration"]].rescaleValue(similarity_ratio);
			sciantix_variable[sv["Intergranular fractional coverage"]].rescaleValue(pow(similarity_ratio, 2));
			sciantix_variable[sv["Intergranular bubble volume"]].rescaleValue(pow(similarity_ratio, 1.5));
			sciantix_variable[sv["Intergranular bubble radius"]].rescaleValue(pow(similarity_ratio, 0.5));
			sciantix_variable[sv["Intergranular vacancies per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));
			sciantix_variable[sv["Intergranular atoms per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));

			// New intergranular gas concentration
			for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			{
				if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
					sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));
			}

			for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			{
				sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].rescaleValue(pow(similarity_ratio, 2.5));
			}
		}

		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			sciantix_variable[sv[sciantix_system[i].getGasName() + " released"]].setFinalValue(
					sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " in grain"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getFinalValue());
		}

		// Swelling
		sciantix_variable[sv["Intergranular gas swelling"]].setFinalValue(
				3 / sciantix_variable[sv["Grain radius"]].getFinalValue() *
				sciantix_variable[sv["Intergranular bubble concentration"]].getFinalValue() *
				sciantix_variable[sv["Intergranular bubble volume"]].getFinalValue());

		sciantix_variable[sv["Porosity"]].addValue(sciantix_variable[sv["Intergranular gas swelling"]].getIncrement());
	}

	void GrainBoundarySweeping()
	{
		// Sweeping of the gas concentrations
		// dC / df = - C

		if (!input_variable[iv["Grain-boundary sweeping"]].getValue())
			return;

		// intra-granular gas diffusion modes
		if (input_variable[iv["iDiffusionSolver"]].getValue() == 1)
		{
			for (int i = 0; i < n_modes; ++i)
			{
				he_diffusion_modes[i] =
						solver.Decay(
								he_diffusion_modes[i],
								1.0,
								0.0,
								model[sm["Grain-boundary sweeping"]].getParameter().at(0));
			}
		}

		else if (input_variable[iv["iDiffusionSolver"]].getValue() == 2)
		{
			for (int i = 0; i < n_modes; ++i)
			{
				he_diffusion_modes_solution[i] =
						solver.Decay(
								he_diffusion_modes_solution[i],
								1.0,
								0.0,
								model[sm["Grain-boundary sweeping"]].getParameter().at(0));

				he_diffusion_modes_bubbles[i] =
						solver.Decay(
								he_diffusion_modes_bubbles[i],
								1.0,
								0.0,
								model[sm["Grain-boundary sweeping"]].getParameter().at(0));
			}
		}
	}

	void GrainBoundaryMicroCracking()
	{
		/// GrainBoundaryMicroCracking is method of simulation which executes the SCIANTIX simulation for the grain-boundary micro-cracking induced by a temperature difference.
		/// This method calls the related model "Grain-boundary micro-cracking", takes the model parameters and solve the model ODEs.

		// If the model has not been activated, exit from this simulation.
		// The same line is in GrainBoundaryMicroCracking.C
		if (!input_variable[iv["iGrainBoundaryMicroCracking"]].getValue())
			return;

		// ODE for the intergranular fractional intactness: this equation accounts for the reduction of the intergranular fractional intactness following a temperature transient
		// df / dT = - dm/dT f
		sciantix_variable[sv["Intergranular fractional intactness"]].setFinalValue(
				solver.Decay(sciantix_variable[sv["Intergranular fractional intactness"]].getInitialValue(),
										 model[sm["Grain-boundary micro-cracking"]].getParameter().at(0), // 1st parameter = microcracking parameter
										 0.0,
										 history_variable[hv["Temperature"]].getIncrement()));

		// ODE for the intergranular fractional coverage: this equation accounts for the reduction of the intergranular fractional coverage following a temperature transient
		// dFc / dT = - ( dm/dT f) Fc
		sciantix_variable[sv["Intergranular fractional coverage"]].setFinalValue(
				solver.Decay(sciantix_variable[sv["Intergranular fractional coverage"]].getInitialValue(),
										 model[sm["Grain-boundary micro-cracking"]].getParameter().at(0) * sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue(),
										 0.0,
										 history_variable[hv["Temperature"]].getIncrement()));

		// ODE for the saturation fractional coverage: this equation accounts for the reduction of the intergranular saturation fractional coverage following a temperature transient
		// dFcsat / dT = - (dm/dT f) Fcsat
		sciantix_variable[sv["Intergranular saturation fractional coverage"]].setFinalValue(
				solver.Decay(
						sciantix_variable[sv["Intergranular saturation fractional coverage"]].getInitialValue(),
						model[sm["Grain-boundary micro-cracking"]].getParameter().at(0) * sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue(),
						0.0,
						history_variable[hv["Temperature"]].getIncrement()));

		// ODE for the intergranular fractional intactness: this equation accounts for the healing of the intergranular fractional intactness with burnup
		// df / dBu = - h f + h
		sciantix_variable[sv["Intergranular fractional intactness"]].setFinalValue(
				solver.Decay(
						sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue(),
						model[sm["Grain-boundary micro-cracking"]].getParameter().at(1), // 2nd parameter = healing parameter
						model[sm["Grain-boundary micro-cracking"]].getParameter().at(1),
						sciantix_variable[sv["Burnup"]].getIncrement()));

		// ODE for the saturation fractional coverage: this equation accounts for the healing of the intergranular saturation fractional coverage with burnup
		// dFcsat / dBu = h (1-f) Fcsat
		sciantix_variable[sv["Intergranular saturation fractional coverage"]].setFinalValue(
				solver.Decay(
						sciantix_variable[sv["Intergranular saturation fractional coverage"]].getFinalValue(),
						model[sm["Grain-boundary micro-cracking"]].getParameter().at(1) * (1.0 - sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue()),
						0.0,
						sciantix_variable[sv["Burnup"]].getIncrement()));

		// Re-scaling: to maintain the current fractional coverage unchanged
		const double similarity_ratio = sqrt(
				sciantix_variable[sv["Intergranular fractional coverage"]].getFinalValue() / sciantix_variable[sv["Intergranular fractional coverage"]].getInitialValue());

		if (similarity_ratio < 1.0)
		{
			sciantix_variable[sv["Intergranular bubble area"]].rescaleValue(similarity_ratio);
			sciantix_variable[sv["Intergranular bubble concentration"]].rescaleValue(similarity_ratio);
			sciantix_variable[sv["Intergranular fractional coverage"]].rescaleValue(pow(similarity_ratio, 2));
			sciantix_variable[sv["Intergranular bubble volume"]].rescaleValue(pow(similarity_ratio, 1.5));
			sciantix_variable[sv["Intergranular bubble radius"]].rescaleValue(pow(similarity_ratio, 0.5));
			sciantix_variable[sv["Intergranular vacancies per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));
			sciantix_variable[sv["Intergranular atoms per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));

			for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			{
				if (gas[ga[sciantix_system[i].getGasName()]].getDecayRate() == 0.0)
					sciantix_variable[sv["Intergranular " + sciantix_system[i].getGasName() + " atoms per bubble"]].rescaleValue(pow(similarity_ratio, 1.5));
			}

			for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			{
				sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].rescaleValue(pow(similarity_ratio, 2.5));
			}
		}

		// Fission gas release due to grain-boundary micro-cracking
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
		{
			sciantix_variable[sv[sciantix_system[i].getGasName() + " released"]].setFinalValue(
					sciantix_variable[sv[sciantix_system[i].getGasName() + " produced"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " decayed"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " in grain"]].getFinalValue() -
					sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getFinalValue());
		}
	}

	void SolidSwelling()
	{
		sciantix_variable[sv["Intragranular swelling"]].setFinalValue(
			sciantix_variable[sv["Xe solid swelling"]].getFinalValue() +
			sciantix_variable[sv["Kr solid swelling"]].getFinalValue() + 
			sciantix_variable[sv["Intragranular gas swelling"]].getFinalValue()
		);

		sciantix_variable[sv["Solid density"]].setFinalValue(
			matrix[sma["UO2"]].getTheoreticalDensity() /
			(1.0 + sciantix_variable[sv["Solid swelling"]].getFinalValue() + 
			sciantix_variable[sv["Xe solid swelling"]].getFinalValue() + 
			sciantix_variable[sv["Kr solid swelling"]].getFinalValue())
		);
	}

	void GrainBoundaryVenting()
	{
		if (!int(input_variable[iv["iGrainBoundaryVenting"]].getValue()))
			return;

		double sigmoid_variable;
		sigmoid_variable = sciantix_variable[sv["Intergranular fractional coverage"]].getFinalValue() *
											 exp(1.0 - sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue());

		// Vented fraction
		sciantix_variable[sv["Intergranular vented fraction"]].setFinalValue(
				1.0 /
				pow((1.0 + model[sm["Grain-boundary venting"]].getParameter().at(0) *
											 exp(-model[sm["Grain-boundary venting"]].getParameter().at(1) *
													 (sigmoid_variable - model[sm["Grain-boundary venting"]].getParameter().at(2)))),
						(1.0 / model[sm["Grain-boundary venting"]].getParameter().at(0))));

		// Venting probability
		sciantix_variable[sv["Intergranular venting probability"]].setFinalValue(
				(1.0 - sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue()) + sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue() * sciantix_variable[sv["Intergranular vented fraction"]].getFinalValue());

		// Gas is vented by subtracting a fraction of the gas concentration at grain boundaries arrived from diffusion
		// Bf = Bf - p_v * dB
		for (std::vector<System>::size_type i = 0; i != sciantix_system.size(); ++i)
			sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].setFinalValue(
					solver.Integrator(
							sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getFinalValue(),
							-sciantix_variable[sv["Intergranular venting probability"]].getFinalValue(),
							sciantix_variable[sv[sciantix_system[i].getGasName() + " at grain boundary"]].getIncrement()));
	}

	void HighBurnupStructurePorosity()
	{
		/// @brief
		/// HighBurnupStructurePorosity is method of simulation which executes the SCIANTIX simulation for the evolution of the porosity of a HBS matrix.
		/// This method takes the model parameters, solves the model ODEs and updates the matrix density coherently with the actual porosity.

		if (!int(input_variable[iv["iHighBurnupStructurePorosity"]].getValue()))
			return;

		// porosity evolution
		sciantix_variable[sv["HBS porosity"]].setFinalValue(
				solver.Integrator(
						sciantix_variable[sv["HBS porosity"]].getInitialValue(),
						model[sm["High burnup structure porosity"]].getParameter().at(0),
						sciantix_variable[sv["Burnup"]].getIncrement())
				/*Include it in the porosity evolution*/
		);

		sciantix_variable[sv["Porosity"]].addValue(sciantix_variable[sv["HBS porosity"]].getIncrement());
	}

	Simulation() {}
	~Simulation() {}
};

#endif
