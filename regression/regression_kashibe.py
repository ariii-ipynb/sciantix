"""

This is a python script to execute the regression (running the validation database) of sciantix.

@author G. Zullo

"""

""" ------------------- Import requiered depedencies ------------------- """

import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt
import shutil
from regression_functions import *
import scipy.stats as stats
from sklearn.linear_model import LinearRegression

""" ------------------- Global Variables ------------------- """

number_of_tests_failed = 0


time = []
temperature = []
FGR = []
bbconc = []
burnup = []

##sample_number = len(igSwelling1)


""" ------------------- Functions ------------------- """

# Verify the test results
def check_result(number_of_tests_failed):
  if are_files_equal('output.txt', 'output_gold.txt') == True:
    print(f"Test passed!\n")
  else:
    print(f"Test failed!\n")
    number_of_tests_failed += 1

  return number_of_tests_failed

# Verify the existence of the files: output.txt and output_gold.txt
def check_output(file):
  try :
    data = import_data("output.txt")
  except :
    print(f"output.txt not found in {file}")
    data = np.zeros(shape=(1, 1))

  try :
    data_gold = import_data("output_gold.txt")
  except :
    print(f"output_gold.txt not found in {file}")
    data_gold = np.ones(shape=(1, 1))

  return data, data_gold

# Execute sciantix in the current test folder
def do_sciantix():
  # copying input files from the regression folder into the current folder
  shutil.copy("../input_settings.txt", os.getcwd())
  shutil.copy("../input_scaling_factors.txt", os.getcwd())

  # copying and executing sciantix.exe into cwd
  shutil.copy("../sciantix.x", os.getcwd())
  os.system("./sciantix.x")

  # removing useless file
  os.remove("sciantix.x")
  os.remove("execution.txt")
  os.remove("input_check.txt")
  # os.remove("overview.txt")

# Replace the existing output_gold.txt with the new output.txt
def do_gold():
  try :
    os.path.exists('output.txt')

    os.remove('output_gold.txt')
    os.rename('output.txt', 'output_gold.txt')

  except :
    print(f"output.txt not found in {file}")

# Plot the regression test results
def do_plot(x,y):
  # SCIANTIX 1.0 vs. SCIANTIX 2.0
  fig, ax = plt.subplots()

  ax.scatter(x, y, c = '#98E18D', edgecolors= '#999AA2', marker = 'o', s=20, label='SCIANTIX 2.0')

  # ax.plot([1e-3, 1e2],[1e-3, 1e2], '-', color = '#757575')
  # ax.plot([1e-3, 1e2],[2e-3, 2e2],'--', color = '#757575')
  # ax.plot([1e-3, 1e2],[5e-4, 5e1],'--', color = '#757575')
  # ax.set_xlim(1e-2, 1e1)
  # ax.set_ylim(1e-2, 1e1)

  # ax.set_xscale('log')
  # ax.set_yscale('log')

  # ax.set_title('Intragranular gaseous swelling')
  ax.set_xlabel('Burnup (MWd/kg)')
  ax.set_ylabel('Intragranular bubble concentratrion (bub/m3)')
  ax.legend()

  plt.show()



# Main function of the baker regression
def regression_kashibe(wpath, mode_Kashibe, mode_gold, mode_plot, folderList, number_of_tests, number_of_tests_failed):

  # Exit of the function without doing anything
  if mode_Kashibe == 0 :
    return folderList, number_of_tests, number_of_tests_failed

  # Get list of all files and directories in wpath
  files_and_dirs = os.listdir(wpath)

  # Sort them by filename
  sorted_files_and_dirs = sorted(files_and_dirs)

  # Iterate over sorted list
  for file in sorted_files_and_dirs:
    # Verify on a given folder, if Baker is in it's name
    if "Kashibe" in file and os.path.isdir(file):
      folderList.append(file)
      os.chdir(file)

      print(f"Now in folder {file}...")
      number_of_tests += 1

      # mode_gold = 0 : Use SCIANTIX / Don't use GOLD and check result
      if mode_gold == 0:

        do_sciantix()
        data, data_gold = check_output(file)
        number_of_tests_failed = check_result(number_of_tests_failed)

      # mode_gold = 1 : Use SCIANTIX / Use GOLD
      if mode_gold == 1:

        do_sciantix()
        data, data_gold = check_output(file)
        print("...golding results.")
        do_gold()

      # mode_gold = 2 : Don't use SCIANTIX / Don't use GOLD and check result
      if mode_gold == 2:

        data, data_gold = check_output(file)
        number_of_tests_failed = check_result(number_of_tests_failed)

      # mode_gold = 3 : Don't use SCIANTIX / Use GOLD
      if mode_gold == 3:

        data, data_gold = check_output(file)
        print("...golding existing results.")
        do_gold()

      # Retrieve the generated data of Time
      timePos = findSciantixVariablePosition(data, "Time (h)")
      time.append(data[-1:,timePos].astype(float))
      
      #Retrieve the generated data of Temperature
      temperaturePos = findSciantixVariablePosition(data, "Temperature (K)")
      temperature.append(data[-1, temperaturePos].astype(float))

      #Retrieve the generated data of Burnup
      burnupPos = findSciantixVariablePosition(data, "Burnup (MWd/kgUO2)")
      burnup.append(data[-1, burnupPos].astype(float))

      # Retrieve the generated data of Fission gas release
      FGRPos = findSciantixVariablePosition(data, "Fission gas release (/)")
      FGR.append(data[-1,FGRPos].astype(float))

      # Retrieve the generated data of Intragranular bubble concentration (bub/m3)
      bbconcPos = findSciantixVariablePosition(data, "Intragranular bubble concentration (bub/m3)")
      bbconc.append(data[-1,bbconcPos].astype(float))

      # arrays
      # time = data[1:,timePos].astype(float)
      # temperature = data[1:,temperaturePos].astype(float)
      # burnup = data[1:,burnupPos].astype(float) / 0.8814
      # FGR = data[1:,FGRPos].astype(float)
      # bbconc = data[1:,bbconcPos].astype(float)

      os.chdir('..')

  # Check if the user has chosen to display the various plots
  if mode_plot == 1:
    do_plot(burnup, bbconc)

  return folderList, number_of_tests, number_of_tests_failed
