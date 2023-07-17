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
//  Authors: G. Zullo                                                               //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#include "SciantixVariableDeclaration.h"
#include "HistoryVariableDeclaration.h"
#include "ModelDeclaration.h"
#include "MapSciantixVariable.h"
#include "MapHistoryVariable.h"

void EnvironmentComposition( );

class H2O_decomposition_equilibrium_constant 
{
  public:
    void K_H2O_Olander(double temperature)
    {
      K_H2O = exp(-26200 / temperature + 6.032);
    }

    void K_H2O_Olander_1986(double temperature)
    {
      K_H2O = exp(-30165.985 / temperature + 6.95);
    }

    void K_H2O_Kubaschewski(double temperature)
    {
      K_H2O = exp(0.9797 * log(temperature) - 1.1128 - 28827 / temperature);
    }

    void K_H2O_Ulich(double temperature)
    { 
      /// @ref Morel et al., CEA, Report NT/DTP/SECC no. DR94-55 (1994)
      K_H2O = 84.945 * exp(-25300 / temperature + 7.28e-4 * temperature);
    }

    double get_KH2O()
    {
      return K_H2O;
    }

  private:
    double K_H2O;
};
