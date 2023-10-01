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
//  Authors: D. Pizzocri, G. Zullo, A. Pagani                                       //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////


#include "SolidSwelling.h"

void SolidSwelling()
{
	/**
	 * @brief This function defines the sciantix model *SolidSwelling*.
	 * 
	 * The model SolidSwelling is used to evaluate 
	 * 
	 * - SolidSwelling SolidSwelling
   * 
	 */

	model.emplace_back();
	int model_index = int(model.size()) - 1;
	model[model_index].setName("Solid swelling");


	std::vector<double> parameter;

  const double N_av = CONSTANT_NUMBERS_H::PhysicsConstants::avogadro_number;

  const double v_U = 40.93e-24;						/*Fuel specific volume (UO2 and PuO2) [m3/molecule]*/

  /* Here I'd need something that takes the value of the lattice parameter to properly calculate v_U*/

  // covolume
  const double v_SOL = 40.93e-24;					/*Soluble fission products specific volume*/
  const double v_MI = 14.73e-24;					/*Metallic inclusions specific volume*/
  const double v_AEO = 71.2e-24;					/*Alkaline earth oxides specific volume*/
  const double v_Oth = 31.1e-24;					/*Other fission products specific volume*/
  const double v_Xe = 84.8e-24;						/*Xenon covolume*/

  /*Here I need to ponder on the differentiation of the yielding depending on the matrix, for now we only care about UO2*/

  /*D. Olander- Fundamental aspects of nuclear reactor fuel elements, chapter 12.1*/
  /*Here I'd need a differentiation of the yielding with respect to the type of fuel matrix... to be improved*/

  const double y_ZrNb  = 0.149;						/*Zr + Nb*/
  const double y_Yre = 0.534;						  /*Y + rare earths*/
  const double y_BaSr = 0.149;						/*Ba + Sr*/				
  const double y_Mo = 0.240;							/*Mo*/
  const double y_RuTcRhPd = 0.263;				/*Ru + Tc + Rh + Pd*/
  const double y_CsRb = 0.226;						/*Cs + Rb*/
  const double y_ITe = 0.012;							/*I + Te*/

  const double y_SOL = y_ZrNb + y_Yre;		// soluble FPs
  const double y_MI = y_Mo + y_RuTcRhPd;  // metallic inclusions
  const double y_AEO = y_BaSr;            // alkaline earth oxides
  const double y_Oth = y_CsRb + y_ITe;    // others

  const double swell_SOL = y_SOL * (v_SOL / v_U);
  const double swell_MI =  y_MI * (v_MI / v_U);
  const double swell_AEO = y_AEO * (v_AEO / v_U);
  const double swell_Oth = y_Oth * (v_Oth / v_U);

  
  double swell_TOT  = (swell_SOL + swell_MI + swell_AEO + swell_Oth) - 1.0;  
  double corr_fact = 1.072;


  switch (int(input_variable[iv["iSolidSwelling"]].getValue()))
  { 
    case 0: 
    {
      swell_TOT = 0;
      corr_fact = 0;
      break;
    }

      case 1:
      {
      swell_TOT = swell_TOT;
      corr_fact = corr_fact;
      break;
      }
  }

	sciantix_variable[sv["Solid swelling"]].setFinalValue(
				sciantix_variable[sv["FIMA"]].getFinalValue() * swell_TOT/ 100);

	sciantix_variable[sv["Xe solid swelling"]].setFinalValue(
				sciantix_variable[sv["Xe in intragranular solution"]].getFinalValue() / (sciantix_variable[sv["U"]].getFinalValue()) * corr_fact // maybe uo2
		);
  
  sciantix_variable[sv["Kr solid swelling"]].setFinalValue(
				sciantix_variable[sv["Kr in intragranular solution"]].getFinalValue() / (sciantix_variable[sv["U"]].getFinalValue()) * corr_fact // maybe uo2
		); 


	parameter.push_back(swell_TOT);
  model[model_index].setParameter(parameter);
  
  model[model_index].setRef("Olander, D. R. Fundamental aspects of nuclear reactor fuel elements. 1976.");
}
