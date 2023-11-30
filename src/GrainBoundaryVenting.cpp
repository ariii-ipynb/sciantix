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

#include "GrainBoundaryVenting.h"

void GrainBoundaryVenting()
{
	model.emplace_back();
	int model_index = int(model.size()) - 1;
	model[model_index].setName("Grain-boundary venting");

	std::vector<double> parameter;
	std::string reference;

	/// @brief
	/// These model defines the venting of fission gas from the grain boundaries of the UO2 fuel matrix.

	switch (int(input_variable[iv["iGrainBoundaryVenting"]].getValue()))
	{
	case 0:
	{
		/// @brief
		/// This case corresponds to no grain boundary venting.

		reference = "not considered.";

		parameter.push_back(0.0);

		break;
	}

	case 1:
	{

		/// @brief
		/// This case defines a set of parameters of the sigmoid function describing the venting probability 

		// screw parameter
		const double screw_parameter = 0.1;

		// span parameter
		const double span_parameter = 10.0;

		// cent parameter
		const double cent_parameter = 0.43;

	
		double sigmoid_variable;
		sigmoid_variable = sciantix_variable[sv["Intergranular fractional coverage"]].getFinalValue() *
											 exp(1.0 - sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue());

		// Vented fraction
		sciantix_variable[sv["Intergranular vented fraction"]].setFinalValue(
				1.0 /
				pow((1.0 + (sf_screw_parameter * screw_parameter) *
											 exp(-(sf_span_parameter * span_parameter) *
													 (sigmoid_variable - (sf_cent_parameter * cent_parameter)))),
						(1.0 / (sf_screw_parameter * screw_parameter))));

		// Venting probability
		sciantix_variable[sv["Intergranular venting probability"]].setFinalValue(
				(1.0 - sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue()) + sciantix_variable[sv["Intergranular fractional intactness"]].getFinalValue() * sciantix_variable[sv["Intergranular vented fraction"]].getFinalValue());
		
		parameter.push_back(sciantix_variable[sv["Intergranular venting probability"]].getFinalValue());

		reference = "from Pizzocri et al., D6.4 (2020), H2020 Project INSPYRE";

		break;
	}
	case 2:
	{
		/**
		* @brief Athermal venting according to Claisse & Van Uffelen (2015).
		* 
		* @author A. Pagani
		* 
		*/
		double fabrication_porosity = sciantix_variable[sv["Fabrication porosity"]].getFinalValue();
		if (fabrication_porosity <= 1.0)
			{
				// Claisse's fit
					const bool check1 = (fabrication_porosity < 0.050) ? 1 : 0;
					const bool check2 = (fabrication_porosity > 0.058) ? 1 : 0;

					double p_open = (fabrication_porosity / 20) * (check1) + 
									 (3.10 * fabrication_porosity - 0.1525) * (!check1 && !check2) + 
									(fabrication_porosity / 2.1 - 3.2e-4) * (check2);
					if (history_variable[hv["Time"]].getFinalValue() == 0)
					{
						sciantix_variable[sv["Open porosity"]].setFinalValue(p_open);
					}
					double athermal_venting = 1.54 * sqrt(p_open);
				if (history_variable[hv["Time"]].getFinalValue() == 0)
				{
					sciantix_variable[sv["Open porosity"]].setFinalValue(p_open);
				}
					sciantix_variable[sv["Intergranular venting probability"]].setFinalValue(athermal_venting);
					parameter.push_back(sciantix_variable[sv["Intergranular venting probability"]].getFinalValue());
			}
			else
			{
				std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
				parameter.push_back(0.0);
			}
	}
	break;

	case 3:
	{	/**
		* @brief Athermal venting according to Pagani et al.
		* 
		* @author A. Pagani
		* 
		*/
		// Pagani's fit: FNN built on the data from Song et al., (2000).

			double fabrication_porosity = sciantix_variable[sv["Fabrication porosity"]].getFinalValue();
			
			if (fabrication_porosity <= 1.0)
			{
        double Px1_step1[3] = {0.0309254289000001, 50.5545836563236, -1};
        double Pb1 = -0.76352583531298012787;
        double PIW1_1 = 2.4890484320254797623;
        double Pb2 = 0.030657996141059989936;
        double PLW2_1 = 1.003372825212876851;
        double Py1_step1[3] = {-1, 63.2911366167282, 0.00091428483};
    

        fabrication_porosity = (fabrication_porosity - Px1_step1[0]) * Px1_step1[1] + Px1_step1[2];
				double Pn = Pb1 + PIW1_1 * fabrication_porosity;
				double Pa1 = 2 / (1 + exp(-2 * Pn)) - 1;
				double Pa2 = Pb2 + PLW2_1 * Pa1;
				double p_open = (Pa2 - Py1_step1[0]) / Py1_step1[1] + Py1_step1[2];

				if (history_variable[hv["Time"]].getFinalValue() == 0)
				{
					sciantix_variable[sv["Open porosity"]].setFinalValue(p_open);
				}
				double i_offset[] = {1.67557472604624,0.0200027413864773,3.78023892133081e-06,0.0242517088661909,336.502196086807,2.58669442083626e+17};
        
				double i_gain[] = { 2.72859602857881,28.5730418805038,277276.473200955,0.0400202848104835,0.00163773486033113, 2.27129465494893e-20 };
        double i_min = -1;
 
        double b1[] = { -1.8205020144980150754,0.57265361380678825309,0.5613062885986996875,1.3713799688952865719,0.86942612540124863951,1.9606062780183723948,1.2080134741599797277,-1.1589982750787828358,-0.30938471823994556642,-1.7144217831949442044 };
        double IW1_1[] = { 1.1013736772312812118,-0.047998320831712987244,-0.3533524550865264624,0.41959241931158824945,-0.20662716954855134266,0.049117911935156410852,
        0.52366174598413861574,0.054903733389925796216,0.28598903173163930935,-0.120861820745802373,0.70829308422250114585,-0.13723754649917441206,
        0.085121240679260939954,4.3459248852024643384,0.032838609978346575136,-0.085744800942920501585,-0.028726084052892453258,-0.0072787529183555065362,
        -0.60009156503264493576,-0.47168440020959290226,-0.35605801685877563889,1.1610116818903375435,0.11295557068838818493,0.0057157089018326206337,
        0.32942986526968959238,0.061246941005226748778,0.1008404924993178825,0.86468329668619259287,-0.17109303151776206198,0.070652826477962921148,
        -0.77784036297201075705,-0.15158438963758086304,-0.0308443212641777674,0.1246325807091639426,-1.0820939733524370663,0.20293582682387498184,
        0.51470709860905761168,0.25732937383186615943,0.19002633405790186893,0.60643798763941791918,-0.30817959793866978035,0.083384016950252673461,
        0.39427533893273691845,0.44893056880590986868,0.31189655243795094686,-1.5423206410799987776,-0.3163339235432545693,0.024605011699621114063,
        -0.51105993484509038005,-0.54711503793047766564,-0.51276465525219905306,1.3421853210193386285,1.6895546034622426212,-0.25305990174798953518,
        -0.52027282672432084709,-0.10968551007777141426,-0.1329461396122790795,0.41206802410067228104,1.8923536372587794752,-0.36413608703956573676};
 
        double b2 = 0.81975757649298830465;
        std::vector<double> LW2_1 = { 0.53688299953557960809, 0.57977466930480758833, 0.29489931833779486903, 2.0446915490465520371, 1.8692465103231785672, -1.3533250934031610946, -1.8123046790041832388, 1.7307702363786152677, -0.23981582585319524692, -0.559848499668203603012 };
 
        double o_offset = 0.0657316210152818;
        double o_gain = 2.14099254432474;
        double o_min = -1;

				// Inputs
				const double theta = 2.0 / 3.0 * 3.14;
				double p = sciantix_variable[sv["Fabrication porosity"]].getFinalValue();
				double l = 1.0/3.0 * 1.1 * 2.0 * sciantix_variable[sv["Grain radius"]].getFinalValue();
				double bu = sciantix_variable[sv["Burnup"]].getFinalValue();
				double T = history_variable[hv["Temperature"]].getFinalValue();
				double F = history_variable[hv["Fission rate"]].getFinalValue();

        double input[] = { theta,  p,  l,  bu,  T,  F };
 
        // Dimension
        const int n_neurons(10);
 
        // Input
        const int input_number = 6;
 
        for (int i = 0; i < input_number; ++i)
				{
          input[i] = (input[i] - i_offset[i]) * i_gain[i] + i_min;
				}
 
         // Layer 1
         double pp[n_neurons] = {};
         double arg[n_neurons] = {};
 
         // IW1_1 * input = arg
         solver.dotProduct2D(IW1_1, input, n_neurons, input_number, arg);
 
        for (int i = 0; i < n_neurons; ++i)
        {
            pp[i] = b1[i] + arg[i];
            pp[i] = 2 / (1 + exp(-2 * pp[i])) - 1;
        }
        double oo;
        double asp;
        asp = solver.dotProduct1D(LW2_1, pp, n_neurons);
        oo = b2 + asp;

        oo = (oo - o_min) / o_gain + o_offset;
				// std::cout << oo << std::endl;
				double athermal_venting = 1.54 * sqrt(p_open);

				sciantix_variable[sv["Intergranular venting probability"]].setFinalValue(oo * athermal_venting);
				parameter.push_back(sciantix_variable[sv["Intergranular venting probability"]].getFinalValue());

			}
			else
			{
				std::cout << "ERROR: invalid fabrication porosity value!" << std::endl;
				parameter.push_back(0.0);

			}
	}
	break;

	default:
		ErrorMessages::Switch("GrainBoundaryVenting.cpp", "iGrainBoundaryVenting", int(input_variable[iv["iGrainBoundaryVenting"]].getValue()));
		break;
	}

	model[model_index].setParameter(parameter);
	model[model_index].setRef(reference);
}