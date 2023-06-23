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

#include "ChromiumSolubility.h"

void ChromiumSolubility()
{	
	/**
	 * @brief This function defines the sciantix model *CromiumSolubility*.
	 * 
	 * The model CromiumSolubility is used to evaluate the cromium solubility accordingly to the temperature and the oxigen content.
	 * Then the number of oxide chromium atoms is evaluated, accordingly to the oxigen content of the system. 
	 * 
	 */

	model.emplace_back();

	int model_index = int(model.size()) - 1;

	model[model_index].setName("Chromium solubility");
	std::string reference;
	std::vector<double> parameter;
	
	reference += "Ch. Riglet-Martial et al., JNM, 447 (2014) 63-72.";

	const double Oxigen_chemical_potential_0K = -479070; //(J/mol)
	double Oxigen_chemical_potential_Cr2O3 = Oxigen_chemical_potential_0K + PhysicsConstants::calorie * history_variable[hv["Temperature"]].getFinalValue() * (8.86*log10(history_variable[hv["Temperature"]].getFinalValue())-4.42 + 9.152*log10(2.4/100)); //(J/mol)
	double Oxigen_chemical_potential_Cr    = Oxigen_chemical_potential_0K + PhysicsConstants::calorie * history_variable[hv["Temperature"]].getFinalValue() * (8.86*log10(history_variable[hv["Temperature"]].getFinalValue())-4.42 + 9.152*log10(1.017/100)); //(J/mol)

    double log10_Oxigen_pressure_Cr203 = Oxigen_chemical_potential_Cr2O3/(PhysicsConstants::gas_constant*history_variable[hv["Temperature"]].getFinalValue()*log(10));  //(bar)
	double log10_Oxigen_pressure_Cr	   = Oxigen_chemical_potential_Cr   /(PhysicsConstants::gas_constant*history_variable[hv["Temperature"]].getFinalValue()*log(10));  //(bar)

	// Solubility of Chromium is evaluated as log10(y_Cr) = p*log10(P_O2) + V + U/T
	// Coefficients U and V came from experimental fitting, in this way the Gibbs Potential is approximated as a function of 1/T
	// Metallic chromium shows two different phases with threshold temperature of 1651 °C
	// CrO is expected to be negligible, accordingly to CrO phase diagram

	double V_coeff_Cr_ht;
	double V_coeff_Cr_lt;
	double U_coeff_Cr_ht; 
   	double U_coeff_Cr_lt; 
	double threshold_temp; 
	double end_temp;

	double C1;
	double C2;

	switch (int (input_variable[iv["iChromiumSolubility"]].getValue()))
		{
		case 0:
		{
			/**
	 		* @brief
			* Coefficient from solubility model by Ch. Riglet-Martial et al. (2014)
			*
			**/
			V_coeff_Cr_ht = -8.391; 
    		V_coeff_Cr_lt = -6.076; 
			U_coeff_Cr_ht =  2.816e+4; 
   			U_coeff_Cr_lt =  2.370e+4; 

			threshold_temp = 1651+273.15; //(K)
			end_temp = 1500 + 273.15; //(K)

			C1 = 0.005;
			C2 = 1800;

		} 

		case 1:
		{
			/**
	 		* @brief
			* Optimized coeffients
			*
			**/
    		V_coeff_Cr_lt = -5.948346054563108;
   			U_coeff_Cr_lt =  22989.0;

			threshold_temp = 4000; //(K)
			end_temp = 1721.2193652656406; //(K)

			C1 = 0.0017215611636416032;
			C2 = 1501.8547191537048;

		}

		case 2:
		{
			/**
	 		* @brief
			* User define coefficients
			*
			**/

			V_coeff_Cr_lt = 0;
   			U_coeff_Cr_lt = 0;

			threshold_temp = 4000; //(K)
			end_temp = 0; //(K)

			C1 = 0;
			C2 = 0;

		}

		}

    const double V_coeff_Cr2O3 =  0.1385; 
    const double U_coeff_Cr2O3 = -0.4827e+4;
    
    const double p_stoichiometry_Cr = 0.75;
	const double p_stoichiometry_Cr2O3 = 0;

	double Cr_solubility; 

	// routine to calculate the molar mass of Uranium
	double conv_fact = sciantix_variable[sv["Fuel density"]].getFinalValue() * PhysicsConstants::avogadro_number *10 * 0.8815 * 100; // to move from atoms/m3 to percentage in atoms (see Initializatio.cpp)
	double molar_mass_Uranium = sciantix_variable[sv["U234"]].getFinalValue()/conv_fact *pow(234.04095,2)+ sciantix_variable[sv["U235"]].getFinalValue()/conv_fact *pow(235.04393,2)+  
								sciantix_variable[sv["U236"]].getFinalValue()/conv_fact *pow(236.04557,2)+ sciantix_variable[sv["U237"]].getFinalValue()/conv_fact *pow(237.04873,2)+  
								sciantix_variable[sv["U238"]].getFinalValue()/conv_fact *pow(238.05079,2);

    if (history_variable[hv["Temperature"]].getFinalValue() < threshold_temp) 
	{
        Cr_solubility = pow(10, p_stoichiometry_Cr*log10_Oxigen_pressure_Cr + V_coeff_Cr_lt + U_coeff_Cr_lt/ history_variable[hv["Temperature"]].getFinalValue()); //(molar Cr solubility)
		Cr_solubility = 100*Cr_solubility * PhysicsConstants::molar_mass_Chromium / ((1-Cr_solubility)*molar_mass_Uranium + Cr_solubility * PhysicsConstants::molar_mass_Chromium + 2* PhysicsConstants::molar_mass_Oxigen ); //(% weight Cr /UO2)
	}

    else if (history_variable[hv["Temperature"]].getFinalValue() >= threshold_temp)
	{
        Cr_solubility = pow(10, p_stoichiometry_Cr*log10_Oxigen_pressure_Cr + V_coeff_Cr_ht + U_coeff_Cr_ht/ history_variable[hv["Temperature"]].getFinalValue()); //(molar Cr solubility)
		Cr_solubility = 100*Cr_solubility * PhysicsConstants::molar_mass_Chromium / ((1-Cr_solubility)*molar_mass_Uranium + Cr_solubility * PhysicsConstants::molar_mass_Chromium + 2* PhysicsConstants::molar_mass_Oxigen ); //(% weight Cr /UO2)
	}

	double Cr2O3_solubility = pow(10, p_stoichiometry_Cr2O3*log10_Oxigen_pressure_Cr203 + V_coeff_Cr2O3 + U_coeff_Cr2O3/ history_variable[hv["Temperature"]].getFinalValue()); //(molar Cr solubility)
	Cr2O3_solubility = 100*Cr2O3_solubility * PhysicsConstants::molar_mass_Chromium / ((1-Cr2O3_solubility)*molar_mass_Uranium + Cr2O3_solubility * PhysicsConstants::molar_mass_Chromium + 2* PhysicsConstants::molar_mass_Oxigen ); //(% weight Cr /UO2)

	// We set the solubility below the lower bound of the solubility model 
	if (history_variable[hv["Temperature"]].getFinalValue() < end_temp)
	{
		Oxigen_chemical_potential_Cr = Oxigen_chemical_potential_0K + PhysicsConstants::calorie * end_temp * (8.86*log10(end_temp)-4.42 + 9.152*log10(1.017/100)); //(J/mol)
		log10_Oxigen_pressure_Cr	    = Oxigen_chemical_potential_Cr   /(PhysicsConstants::gas_constant*end_temp*log(10));  //(bar)
		Cr_solubility = pow(10, p_stoichiometry_Cr*log10_Oxigen_pressure_Cr + V_coeff_Cr_lt + U_coeff_Cr_lt/ end_temp); //(molar Cr solubility)
		Cr_solubility = 100*Cr_solubility * PhysicsConstants::molar_mass_Chromium / ((1-Cr_solubility)*molar_mass_Uranium + Cr_solubility * PhysicsConstants::molar_mass_Chromium + 2* PhysicsConstants::molar_mass_Oxigen ); //(% weight Cr /UO2)
		Cr2O3_solubility = pow(10, V_coeff_Cr2O3 + U_coeff_Cr2O3/ end_temp); //(molar Cr solubility)
		Cr2O3_solubility = 100*Cr2O3_solubility * PhysicsConstants::molar_mass_Chromium / ((1-Cr2O3_solubility)*molar_mass_Uranium + Cr2O3_solubility * PhysicsConstants::molar_mass_Chromium + 2* PhysicsConstants::molar_mass_Oxigen ); //(% weight Cr /UO2)
	}

	// Calculations to get the amount of Cr in atoms

	double U_content = sciantix_variable[sv["U234"]].getFinalValue() + sciantix_variable[sv["U235"]].getFinalValue() +sciantix_variable[sv["U236"]].getFinalValue() +
					   sciantix_variable[sv["U237"]].getFinalValue() + sciantix_variable[sv["U238"]].getFinalValue(); // (at U/m3)

	double U_weight = U_content*molar_mass_Uranium/PhysicsConstants::avogadro_number; //(g U/m3)
	double O2_weight = U_content*2*PhysicsConstants::molar_mass_Oxigen/PhysicsConstants::avogadro_number; //(g O2/m3)
	double UO2_weight = U_weight + O2_weight; //(g UO2/m3)

	double Cr_weight = UO2_weight*sciantix_variable[sv["Chromium content"]].getFinalValue()*1e-6; //(g Cr/m3)
	double Cr_atoms = Cr_weight*PhysicsConstants::avogadro_number/PhysicsConstants::molar_mass_Chromium; //(atoms Cr/m3)

	// Anatilitical formulation of the ration between Cr203 and Cr-metal in the fuel 

	double Cr203_fraction = 1 - exp(C1*history_variable[hv["Temperature"]].getFinalValue() - C1*C2);

	if (sciantix_variable[sv["Burnup"]].getFinalValue() <= 30)
	{
		Cr203_fraction = 1 - exp(C1*history_variable[hv["Temperature"]].getFinalValue() - (C1 - 400*(sciantix_variable[sv["Burnup"]].getFinalValue()-30)/30)*C2);
	}

	double Cr_fraction    = 1 - Cr203_fraction;

	if (sciantix_variable[sv["Burnup"]].getFinalValue() == 0)
	{
		Cr203_fraction = 1;
		Cr_fraction = 0;
	}

	if (Cr203_fraction < 0)
	{
		Cr203_fraction = 0;
		Cr_fraction = 1;
	}

	// Evaluations of number of Chromium atoms in metal-oxide compounds

	double Cr_metal = Cr_atoms*Cr_fraction; //(atoms Cr/m3)
	double Cr_oxide = Cr_atoms*Cr203_fraction; //(atoms Cr/m3)

	double Cr_solution = U_weight*Cr_solubility/100; //(g Cr/m3)
	Cr_solution = Cr_solution*PhysicsConstants::avogadro_number/PhysicsConstants::molar_mass_Chromium; //(at Cr/m3)
	double Cr_precipitate = 0; // (at Cr/m3)

	if (Cr_solution > Cr_metal)
		Cr_solution = Cr_metal;
	else
		Cr_precipitate = Cr_metal - Cr_solution;

	double Cr203_solution = U_weight*Cr2O3_solubility/100; //(g Cr/m3)
	Cr203_solution = Cr203_solution*PhysicsConstants::avogadro_number/PhysicsConstants::molar_mass_Chromium; //(at Cr/m3)
	double Cr2O3_precipitate = 0; // (at Cr/m3)

	if (Cr203_solution > Cr_oxide)
		Cr203_solution = Cr_oxide;
	else
		Cr2O3_precipitate = Cr_oxide - Cr203_solution;
		
	parameter.push_back(Cr_precipitate);
	parameter.push_back(Cr_solution);
	parameter.push_back(Cr2O3_precipitate);
	parameter.push_back(Cr203_solution);

	model[model_index].setParameter(parameter);
	sciantix_variable[sv["Chromium solubility"]].setFinalValue(Cr_solubility);
	sciantix_variable[sv["Chromia solubility"]].setFinalValue(Cr2O3_solubility);
	sciantix_variable[sv["Chromium solution"]].setFinalValue(Cr_solution);
	sciantix_variable[sv["Chromium precipitate"]].setFinalValue(Cr_precipitate);
	sciantix_variable[sv["Chromia solution"]].setFinalValue(Cr203_solution);
	sciantix_variable[sv["Chromia precipitate"]].setFinalValue(Cr2O3_precipitate);
	matrix[sma["UO2Cr"]].setChromiumSolubility(Cr_solubility);
	matrix[sma["UO2Cr"]].setChromiaSolubility(Cr2O3_solubility);
	matrix[sma["UO2Cr"]].setChromiumSolution(Cr_solution);
	matrix[sma["UO2Cr"]].setChromiumPrecipitate(Cr_precipitate);
	matrix[sma["UO2Cr"]].setChromiaSolution(Cr203_solution);
	matrix[sma["UO2Cr"]].setChromiaPrecipitate(Cr2O3_precipitate);


	model[model_index].setRef(reference);
}