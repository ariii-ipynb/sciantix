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

""" ------------------- Global Variables ------------------- """

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
  #shutil.copy("../input_settings.txt", os.getcwd())
  #shutil.copy("../input_scaling_factors.txt", os.getcwd())

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
def do_plot(exp_data, data_x, data_y, data_gx, data_gy):

  fig, ax = plt.subplots()

  ax.scatter(exp_data[:,1], exp_data[:,0], c = '#C7C924', edgecolors= '#999AA2', marker = 'o', s=20, label='Riglet-Martial data')
  ax.scatter(data_x, data_y, c = '#98E18D', edgecolors= '#999AA2', marker = 'o', s=20, label='SCIANTIX 2.0')
  ax.scatter(data_gx, data_gy, c = '#65E13D', edgecolors= '#999AA2', marker = 'o', s=20, label='SCIANTIX 2.0 - Gold')

  # ax.plot([1e-3, 1e2],[1e-3, 1e2], '-', color = '#757575')
  # ax.plot([1e-3, 1e2],[2e-3, 2e2],'--', color = '#757575')
  # ax.plot([1e-3, 1e2],[5e-4, 5e1],'--', color = '#757575')

  # ax.set_xlim(0, 2.5e3)
  # ax.set_ylim(0, 1e25)

  # ax.set_yscale("log")

  # ax.set_title('Intergranular gaseous swelling')
  ax.set_xlabel('Temperature (K)')
  ax.set_ylabel('Chromium content (%weight)')
  ax.legend()

  plt.show()


# Main function of the Chromium regression
def regression_chromium(wpath, mode_chromium, mode_gold, mode_plot, folderList, number_of_tests, number_of_tests_failed):

  # Exit of the function without doing anything
  if mode_chromium == 0:
    return folderList, number_of_tests, number_of_tests_failed

  # Get list of all files and directories in wpath
  files_and_dirs = os.listdir(wpath)

  # Sort them by filename
  sorted_files_and_dirs = sorted(files_and_dirs)
  #print(sorted_files_and_dirs)

  # Iterate over sorted list
  for file in sorted_files_and_dirs:
    # Verify on a given folder, if Chromium is in it's name
    if "chromium" in file and os.path.isdir(file):
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

      try :
        data_RigletMartial1 = import_data("Riglet-Martial_data_exp1.txt")
      except :
        print(f"Riglet-Martial_data_exp1.txt not found in {file}")
        data_RigletMartial1 = np.zeros(shape=(1, 1))

      chromiumRigletMartialdata1 = np.genfromtxt("Riglet-Martial_data_exp1.txt")

      # output.txt
      # find indexes
      temperaturePos = findSciantixVariablePosition(data, "Temperature (K)")
      ChromiumPos = findSciantixVariablePosition(data, "Chromium solution (at/m3)")
      ChromiaPos = findSciantixVariablePosition(data, "Chromia solution (at/m3)")

      # arrays
      temperature = data[1:,temperaturePos].astype(float)
      Chromium = data[1:,ChromiumPos].astype(float) + data[1:,ChromiaPos].astype(float)

      # output_gold.txt
      # find indexes
      temperaturePosG = findSciantixVariablePosition(data_gold, "Temperature (K)")
      ChromiumPosG = findSciantixVariablePosition(data_gold, "Chromium solution (at/m3)")
      ChromiaPosG = findSciantixVariablePosition(data_gold, "Chromia solution (at/m3)")

      # arrays
      temperatureG = data_gold[1:,temperaturePosG].astype(float)
      ChromiumG = data_gold[1:,ChromiumPosG].astype(float) + data_gold[1:,ChromiaPosG].astype(float)

      # Check if the user has chosen to display the various plots
      if mode_plot == 1:
        do_plot(chromiumRigletMartialdata1, temperature, Chromium, temperatureG, ChromiumG)

      os.chdir('..')

  return folderList, number_of_tests, number_of_tests_failed








