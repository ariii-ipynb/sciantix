import os
import random
import shutil
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

subfolder_name = "GSA_output_files"
option_runSensitivity = False
samplings = 50
validation_database = "Baker"
sciantix_variable = "Intragranular gas swelling (/)"

def findSciantixVariablePosition(output, variable_name):
  """
  This function gets the output.txt file and the variable name,
  giving back its column index in the ndarray
  """
  i,j = np.where(output == variable_name)
  return int(j)

class globalSensitivityAnalysis():
    def __init__(self):
        self.current_folder = os.getcwd()
        self.file_name = 'input_scaling_factors.txt'
        self.file_path = os.path.join(self.current_folder, self.file_name)

    def readFile_inputScalingFactors(self):

        print(f"\nRunning: readFile_inputScalingFactors")
        print(f"Looking for input_scaling_factors.txt file in {os.getcwd()}")

        self.scaling_factors = {}
        with open(self.file_path, 'r') as file:
            lines = file.readlines()
            i = 0
            while i < len(lines):
                value = float(lines[i].strip())
                name = lines[i + 1].strip()[len("# scaling factor - "):]
                self.scaling_factors[name] = value
                i += 2

        print("\nScaling factor dictionary:")
        print(self.scaling_factors)

    def setValidationParameters(self, variable_name, validation_name, bias_name, deviation, sample_number):
        self.variable_name = variable_name
        self.bias_name = bias_name
        self.validation_name = validation_name
        self.deviation = deviation
        self.sample_number = sample_number
        self.folder_name = []

        self.folder_number = 0
        for file in os.listdir(self.current_folder):
            if validation_name in file and os.path.isdir(file):
                self.folder_name.append(file)
                self.folder_number = self.folder_number + 1

        print(f"\nLooking for {validation_name} database folders.")
        print(f"{self.folder_number} folders have been found!")

    def validiationCase_reference(self):

        print("\nRunning: validiationCase_reference")
        print(f"Collecting reference outputs for {self.variable_name}, in {self.validation_name} database.\n")

        files_and_directories = os.listdir(self.current_folder)
        sorted_files_and_directories = sorted(files_and_directories)
        
        self.reference_value_map = np.zeros(self.folder_number)

        i = 0
        for file in sorted_files_and_directories:
            if self.validation_name in file and os.path.isdir(file):
                os.chdir(file)
                print("Now in folder ", os.getcwd())
                shutil.copy("../sciantix.x", os.getcwd())
                                    
                for key in self.scaling_factors.keys():
                    self.scaling_factors[key] = 1.0

                with open("input_scaling_factors.txt", 'w') as file:
                    for key, value in self.scaling_factors.items():
                        file.write(f'{value}\n')
                        file.write(f'# scaling factor - {key}\n')

                os.system("./sciantix.x")

                data = np.genfromtxt('output.txt', dtype= 'str', delimiter='\t')
                variable_position = findSciantixVariablePosition(data, self.variable_name)

                # Maps
                self.reference_value_map[i] = data[-1,variable_position].astype(float)
                    
                i = i + 1
                os.chdir('..')

    def validiationCase_sensitivity(self):

        print("\nRunning: validiationCase_sensitivity")
        print(f"Collecting outputs biasing {self.bias_name}, in {self.validation_name} database.\n")

        files_and_dirs = os.listdir(self.current_folder)
        sorted_files_and_dirs = sorted(files_and_dirs)

        self.scaling_factor_map = np.zeros((self.folder_number, self.sample_number))
        self.sensitivity_coefficient_map = np.zeros((self.folder_number, self.sample_number))
        self.variable_value_map = np.zeros((self.folder_number, self.sample_number))

        i = 0
        for file in sorted_files_and_dirs:
            if self.validation_name in file and os.path.isdir(file):
                os.chdir(file)
                print("\nNow in folder ", os.getcwd())
                shutil.copy("../sciantix.x", os.getcwd())
                
                for j in range(self.sample_number):
                    
                    print(i,j)
                    
                    bias = random.uniform(1 - self.deviation, 1 + self.deviation)
                    self.scaling_factors[self.bias_name] = bias

                    # write input_scaling_factors.txt file with biased scaling factor
                    with open("input_scaling_factors.txt", 'w') as sf_file:
                        for key, value in self.scaling_factors.items():
                            sf_file.write(f'{value}\n')
                            sf_file.write(f'# scaling factor - {key}\n')

                    os.system("./sciantix.x")

                    # Maps
                    self.scaling_factor_map[i][j] = bias

                    data = np.genfromtxt('output.txt', dtype= 'str', delimiter='\t')
                    variable_position = findSciantixVariablePosition(data, self.variable_name)
                    self.variable_value_map[i][j] = data[-1,variable_position].astype(float)

                    self.sensitivity_coefficient_map[i][j] = ((1/self.reference_value_map[i])*((self.reference_value_map[i] - self.variable_value_map[i][j])/(1 - self.scaling_factor_map[i][j])))

                    # clean condition
                    with open("input_scaling_factors.txt", 'w') as sf_file:
                        for key, value in self.scaling_factors.items():
                            sf_file.write(f'1.0\n')
                            sf_file.write(f'# scaling factor - {key}\n')

                    os.remove("execution.txt")
                    os.remove("input_check.txt")

                i = i + 1
                os.chdir('..')

    def validationCase_plot(self):

        for i in range(self.folder_number):
            fig, ax = plt.subplots(1,2)

            plt.subplots_adjust(left=0.1,
                                bottom=0.1,
                                right=0.9,
                                top=0.9,
                                wspace=0.34,
                                hspace=0.4)

            ax[0].scatter(self.scaling_factor_map[i][:], self.variable_value_map[i][:], c = '#98E18D', edgecolors= '#999AA2', marker = 'o', s=20, label = self.folder_name[i])
            ax[0].set_xlabel(f"scaling factor - {self.bias_name}")
            ax[0].set_ylabel(self.variable_name)
            ax[0].legend()

            ax[1].scatter(self.scaling_factor_map[i][:], self.sensitivity_coefficient_map[i][:], c = '#98E18D', edgecolors= '#999AA2', marker = 'o', s=20, label = self.folder_name[i])
            ax[1].set_xlabel(f"scaling factor - {self.bias_name}")
            ax[1].set_ylabel("Sensitivity coefficient")
            ax[1].legend()

            plt.show()

    def validationCase_save(self):

        print("\nSaving validation output")

        if not os.path.exists(subfolder_name):
            os.makedirs(subfolder_name)

        np.save(os.path.join(subfolder_name, f"{self.validation_name}_sensitivity_coefficient_map_{self.bias_name}.npy"), self.sensitivity_coefficient_map)
        np.save(os.path.join(subfolder_name, f"{self.validation_name}_scaling_factor_map_{self.bias_name}.npy"), self.scaling_factor_map)
        np.save(os.path.join(subfolder_name, f"{self.validation_name}_variable_value_map_{self.bias_name}.npy"), self.variable_value_map)
        np.save(os.path.join(subfolder_name, f"{self.validation_name}_reference_value_map_{self.bias_name}.npy"), self.reference_value_map)

    def validationCase_load(self):

        print("\nLoading validation quantities")

        # rows: cases
        # columns: samplings

        self.sensitivity_coefficient_map = np.load(os.path.join(subfolder_name, f"{self.validation_name}_sensitivity_coefficient_map_{self.bias_name}.npy"))
        self.scaling_factor_map = np.load(os.path.join(subfolder_name, f"{self.validation_name}_scaling_factor_map_{self.bias_name}.npy"))
        self.variable_value_map = np.load(os.path.join(subfolder_name, f"{self.validation_name}_variable_value_map_{self.bias_name}.npy"))
        self.reference_value_map = np.load(os.path.join(subfolder_name, f"{self.validation_name}_reference_value_map_{self.bias_name}.npy"))

    def globalSensitivityCoefficients(self):
        self.global_sensitivity_coefficients = np.mean(self.sensitivity_coefficient_map, axis= 1) # column-averages

        fig, ax = plt.subplots()

        ax.scatter(self.folder_name, self.global_sensitivity_coefficients, c = '#98E18D', edgecolors= '#999AA2', marker = 'o', s=20, label= "averaged sensitivity coefficient")
        # ax.set_xlabel(self.validation_name)
        ax.set_ylabel(self.bias_name)
        ax.legend()
        
        plt.show()

#################
# resolution rate
#################

resolution_rate = globalSensitivityAnalysis()
resolution_rate.readFile_inputScalingFactors()
resolution_rate.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "resolution rate",
                                        deviation = 0.15, sample_number = samplings)

if(option_runSensitivity == True):
    resolution_rate.validiationCase_reference()
    resolution_rate.validiationCase_sensitivity()
    resolution_rate.validationCase_save()

else:
    # load & plot
    resolution_rate.validationCase_load()

resolution_rate.validationCase_plot()
resolution_rate.globalSensitivityCoefficients()

###############
# trapping rate
###############

trapping_rate = globalSensitivityAnalysis()
trapping_rate.readFile_inputScalingFactors()
trapping_rate.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "trapping rate",
                                      deviation = 0.15, sample_number = samplings)
if(option_runSensitivity == True):
    trapping_rate.validiationCase_reference()
    trapping_rate.validiationCase_sensitivity()
    trapping_rate.validationCase_save()

else:
    # load & plot
    trapping_rate.validationCase_load()

trapping_rate.validationCase_plot()
trapping_rate.globalSensitivityCoefficients()

#################
# diffusivity
#################

diffusivity = globalSensitivityAnalysis()
diffusivity.readFile_inputScalingFactors()
diffusivity.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "diffusivity",
                                    deviation = 0.5, sample_number = samplings)

if(option_runSensitivity == True):
    diffusivity.validiationCase_reference()
    diffusivity.validiationCase_sensitivity()
    diffusivity.validationCase_save()

else:
    # load & plot
    diffusivity.validationCase_load()

diffusivity.validationCase_plot()
diffusivity.globalSensitivityCoefficients()

#################
# nucleation rate
#################

nucleation_rate = globalSensitivityAnalysis()
nucleation_rate.readFile_inputScalingFactors()
nucleation_rate.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "nucleation rate",
                                        deviation = 0.1, sample_number = samplings)

if(option_runSensitivity == True):
    nucleation_rate.validiationCase_reference()
    nucleation_rate.validiationCase_sensitivity()
    nucleation_rate.validationCase_save()

else:
    # load & plot
    nucleation_rate.validationCase_load()

nucleation_rate.validationCase_plot()
nucleation_rate.globalSensitivityCoefficients()

#################
# temperature
#################

temperature = globalSensitivityAnalysis()
temperature.readFile_inputScalingFactors()
temperature.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "temperature",
                                    deviation = 0.25, sample_number = samplings)

if(option_runSensitivity == True):
    temperature.validiationCase_reference()
    temperature.validiationCase_sensitivity()
    temperature.validationCase_save()

else:
    # load & plot
    temperature.validationCase_load()

temperature.validationCase_plot()
temperature.globalSensitivityCoefficients()

#################
# fission rate
#################

fission_rate = globalSensitivityAnalysis()
fission_rate.readFile_inputScalingFactors()
fission_rate.setValidationParameters(variable_name= sciantix_variable, validation_name= validation_database, bias_name= "fission rate",
                                     deviation = 0.2, sample_number = samplings)

if(option_runSensitivity == True):
    fission_rate.validiationCase_reference()
    fission_rate.validiationCase_sensitivity()
    fission_rate.validationCase_save()

else:
    # load & plot
    fission_rate.validationCase_load()

fission_rate.validationCase_plot()
fission_rate.globalSensitivityCoefficients()
