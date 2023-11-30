"""
MapAthermal.py is a python module made to handle the sciantix (standalone) postprocessing,
and retrieve the quantities in the output.txt file
the purposes are:
    - to map the fission gas release wrt to bu & T in order to highlight the athermal fission gas release contribution.
    - compare the predictive capabilities of SCIANTIX w/o a model for the athermal venting, w/ the Claisse Model and w/ the 
      model developed here in polimi by A. Pagani, D. Pizzocri, G. Zullo with the contribution of P. Van Uffelen

@author: Arianna Pagani


"""
import subprocess
import re

# %matplotlib inline
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

with open('settings_base.txt', 'r') as file:
  # Read the lines from the file into a list
   setting_base = file.readlines()

with open('settings_Claisse.txt', 'r') as file:
  # Read the lines from the file into a list
   setting_Claisse = file.readlines()

with open('settings_Pagani.txt', 'r') as file:
  # Read the lines from the file into a list
   setting_Pagani = file.readlines()


output = []
def sciantixxx(settings, t_end):
  local_output = []
  t_0 = 0

  T_0 = 373
  T_end = 1673

  F = 1e19

  T = T_0

  while T <= T_end:

    # print(T)
    input_history_1 = [t_0,'\t',T,'\t', F,'\t', 0, '\n']
    input_history_2 = [t_end,'\t', T,'\t', F,'\t', 0]
    result_string1 = ''.join(str(item) for item in input_history_1)
    # print(result_string1)
    result_string2 = ''.join(str(item) for item in input_history_2)
    # print(result_string2)

    if __name__ == "__main__":
      
      simulation_executable = './sciantix.x'
    
      history_path = 'input_history.txt'
      with open(history_path, 'w') as file:
        # Write content to the file
        file.write(result_string1)
        file.write(result_string2)
      
      setting_path = 'input_settings.txt'
      with open(setting_path, 'w') as file:
        # Write content to the file
        file.write(''.join(settings))
      subprocess.run(simulation_executable)
        
    # Specify the file path
    output_path = 'output.txt'

    # Open the file in read mode ('r')
    with open(output_path, 'r') as file:
        # Read the lines from the file into a list
        content_list = file.readlines()

    # Access the string from the list
    example_string = content_list[-1]

    # Extract numbers using regular expressions, including exponential notation
    numbers = re.findall(r'\b\d+\.\d+e[+-]?\d+|\b\d+e[+-]?\d+|\b\d+\.\d+|\b\d+\b', example_string)

    # Convert the list of strings to a list of floats
    numbers = [float(num) for num in numbers]

    index_T = 1
    index_FGR = 17
    index_bu = 35
    output_sim = [numbers[index_T], numbers[index_bu], numbers[index_FGR]]

    local_output.append(output_sim)
    T += 100
  return local_output


t = [5500, 14125, 22750, 31375, 40000]

output_base = []
output_Claisse = []
output_Pagani = []

for i in range(0, len(t)):
    
    single_base = sciantixxx(setting_base, t[i])
    output_base.append(single_base)

    single_Claisse = sciantixxx(setting_Claisse, t[i])
    output_Claisse.append(single_Claisse)

    single_Pagani = sciantixxx(setting_Pagani, t[i])
    output_Pagani.append(single_Pagani)

# output[which bu] [which T]

# print(output_base)
# print(output_Claisse)
# print(output_Pagani)

# print(len(output_base[0]))

fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

T_base = []
bu_base = []
FGR_base = []

for i in range(0, len(output_base)):    #bu
   for j in range(0, len(output_base[i])):  
    T_base.append(output_base[i][j][0])
    bu_base.append(output_base[i][j][1])
    FGR_base.append(output_base[i][j][2])

T_Claisse = []
bu_Claisse = []
FGR_Claisse = []

for i in range(0, len(output_Claisse)):    #bu
   for j in range(0, len(output_Claisse[i])):  
    T_Claisse.append(output_Claisse[i][j][0])
    bu_Claisse.append(output_Claisse[i][j][1])
    FGR_Claisse.append(output_Claisse[i][j][2])

T_Pagani = []
bu_Pagani = []
FGR_Pagani = []

for i in range(0, len(output_Pagani)):    #bu
   for j in range(0, len(output_Pagani[i])):  
    T_Pagani.append(output_Pagani[i][j][0])
    bu_Pagani.append(output_Pagani[i][j][1])
    FGR_Pagani.append(output_Pagani[i][j][2])


# Scatter plot
ax.scatter(T_base, bu_base, FGR_base, c='r', marker='o', label='w/o Athermal Release')
ax.scatter(T_Claisse, bu_Claisse, FGR_Claisse, c='b', marker='o', label='Claisse Model')
ax.scatter(T_Pagani, bu_Pagani, FGR_Pagani, c='g', marker='o', label='This Work')


# Set axis labels
ax.set_xlabel('T (K)')
ax.set_ylabel('β (MWd/kg)')
ax.set_zlabel('FGR (/)')

ax.set_zlim(0, 0.045)  # Adjust the limits according to your requirements

# Add a legend
ax.legend()

# Show the plot
plt.show()