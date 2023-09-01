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

/// Initialization
/// This routine initializes Sciantix internal variables with
/// initial conditions/interface variables

#include "Initialization.h"

void Initialization()
{
	const double pi = CONSTANT_NUMBERS_H::MathConstants::pi;

	// Sciantix_history initialization
	Sciantix_history[0] = Temperature_input[0];
	Sciantix_history[1] = Temperature_input[0];
	Sciantix_history[2] = Fissionrate_input[0];
	Sciantix_history[3] = Fissionrate_input[0];
	Sciantix_history[4] = Hydrostaticstress_input[0];
	Sciantix_history[5] = Hydrostaticstress_input[0];
	Sciantix_history[6] = 0.0;

	// Sciantix_variables initialization
	Sciantix_variables[25] = 4.0e+13;  // Intergranular_bubble_concentration[0]
	Sciantix_variables[35] = 0.5;      // Intergranular_saturation_fractional_coverage[0]
	Sciantix_variables[37] = 1.0;      // Intergranular_fractional_intactness[0]

	Sciantix_variables[40] = 10970.0 * (1 - Sciantix_variables[66]);

	// https://pubchem.ncbi.nlm.nih.gov/compound/Uranium-235
	Sciantix_variables[41] *= Sciantix_variables[40] * 6.022e+24 * 0.8815 / 234.04095; // U-234
	Sciantix_variables[42] *= Sciantix_variables[40] * 6.022e+24 * 0.8815 / 235.04393; // U-235
	Sciantix_variables[43] *= Sciantix_variables[40] * 6.022e+24 * 0.8815 / 236.04557; // U-236
	Sciantix_variables[44] *= Sciantix_variables[40] * 6.022e+24 * 0.8815 / 237.04873; // U-237
	Sciantix_variables[45] *= Sciantix_variables[40] * 6.022e+24 * 0.8815 / 238.05079; // U-238
	
	/*std::cout << Sciantix_variables[73] << std::endl;*/

	// Initial HBS porosity
	Sciantix_variables[56] = 1.0 - Sciantix_variables[40] / 10970.0;

	// Intragranular similarity ratio
	Sciantix_variables[64] = 1.0;

	// Initial fuel porosity = fabrication porosity
	Sciantix_variables[67] = 	Sciantix_variables[66];

	// Initial solid density = theoretical density
	Sciantix_variables[69] = 10970.0;

	double fabrication_porosity = Sciantix_variables[66];
	double open_porosity;

	if (fabrication_porosity <= 1.0)
	{
		const bool check1 = (fabrication_porosity < 0.050) ? 1 : 0;
		const bool check2 = (fabrication_porosity > 0.058) ? 1 : 0;

		open_porosity = 
			(fabrication_porosity / 20) * (check1) + 
			(3.10 * fabrication_porosity - 0.1525) * (!check1 && !check2) + 
			(fabrication_porosity / 2.1 - 3.2e-4) * (check2);
	}
	else
	{
		open_porosity = 0.0;
		std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
	}
	Sciantix_variables[68] = open_porosity;


	// Projection on diffusion modes of the initial conditions
	double initial_condition(0.0);
	double projection_remainder(0.0);
	double reconstructed_solution(0.0);
	int iteration(0), iteration_max(20), n(0), np1(1), n_modes(40), k(0), K(20);
	double projection_coeff(0.0);
	projection_coeff = -sqrt(8.0 / pi);

	for (k = 0; k < K; ++k)
	{
		switch (k)
		{
		case 0: initial_condition = Sciantix_variables[2];  break;  // Xe in grain
		case 1: initial_condition = Sciantix_variables[3];  break;  // Xe in grain - solution
		case 2: initial_condition = Sciantix_variables[4];  break;  // Xe in grain - bubbles

		case 3: initial_condition = Sciantix_variables[8];  break;  // Kr in grain
		case 4: initial_condition = Sciantix_variables[9];  break;  // Kr in grain - solution
		case 5: initial_condition = Sciantix_variables[10]; break;  // Kr in grain - bubbles

		case 6: initial_condition = Sciantix_variables[14]; break;  // He in grain
		case 7: initial_condition = Sciantix_variables[15]; break;  // He in grain - solution
		case 8: initial_condition = Sciantix_variables[16]; break;  // He in grain - bubbles

		case 9: initial_condition = Sciantix_variables[49]; break;  // Xe133 in grain
		case 10: initial_condition = Sciantix_variables[50]; break;  // Xe133 in grain - solution
		case 11: initial_condition = Sciantix_variables[51]; break;  // Xe133 in grain - bubbles

		case 12: initial_condition = Sciantix_variables[58]; break;  // Kr85m in grain
		case 13: initial_condition = Sciantix_variables[59]; break;  // Kr85m in grain - solution
		case 14: initial_condition = Sciantix_variables[60]; break;  // Kr85m in grain - bubbles

		default: initial_condition = 0.0; break;
		}

		projection_remainder = initial_condition;
		for (iteration = 0; iteration < iteration_max; ++iteration)
		{
			reconstructed_solution = 0.0;
			for (n = 0; n < n_modes; ++n)
			{
				np1 = n + 1;
				const double n_coeff = pow(-1.0, np1) / np1;
				Sciantix_diffusion_modes[k * n_modes + n] += projection_coeff * n_coeff * projection_remainder;
				reconstructed_solution += projection_coeff * n_coeff * Sciantix_diffusion_modes[k * n_modes + n] * 3.0 / (4.0 * pi);
			}
			projection_remainder = initial_condition - reconstructed_solution;
		}
	}
}
