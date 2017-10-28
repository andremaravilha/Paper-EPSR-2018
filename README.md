# Scheduling Maneuvers for the Restoration of Electric Power Distribution Networks

> **Contributors:** André L. Maravilha<sup>1,3</sup>, Fillipe Goulart<sup>1,3</sup>, Eduardo G. Carrano<sup>2,3</sup>, Felipe Campelo<sup>2,3</sup>  
> <sup>1</sup> *Graduate Program in Electrical Engineering, Universidade Federal de Minas Gerais (PPGEE, UFMG)*  
> <sup>2</sup> *Dept. Electrical Engineering, Universidade Federal de Minas Gerais (UFMG)*  
> <sup>3</sup> *Operations Research and Complex Systems Lab., Universidade Federal de Minas Gerais (ORCS Lab, UFMG)*


## 1. About

This repository contains the source code of the manuscript entitled "Scheduling maneuvers for the restoration of electric power distribution networks: formulation and heuristics", written by André L. Maravilha, Fillipe Goulart, Eduardo G. Carrano and Felipe Campelo, currently submitted to the *Electric Power Systems Research* ([EPSR](https://www.journals.elsevier.com/electric-power-systems-research)).

All methods were coded in C++ (version C++17). The project was developed using [CLion](https://www.jetbrains.com/clion/) (version 2017.2.3) with [CMake](https://cmake.org/) (version 3.8) and  GNU Compiller Collection ([GCC](https://gcc.gnu.org/), version 6.3.0) on a Linux machine. The content of this repository is available under the MIT license. The mixed integer programming (MIP) formulations were solved with [Gurobi](http://www.gurobi.com/) (version 7.5.1).


## 2. How to build the project

#### 2.1. Important comments before building the project

To compile this project you need to have Gurobi (version 7.5.1 or later), GCC (version 6.3.0 or later) and CMake (version 3.8 or later) installed on you computer.

#### 2.2. Building the project

Inside the root directory of the project, run the following commands:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../source
make
```

The project is configured to search for Gurobi 7.5.1 at `/opt/gurobi/` directory. If Gurobi is installed in another directory or your are using a version different from 7.5.1, you have to set the correct path for Gurobi installation and the name of its libraries. For example, if Gurobi is installed under the directory `/opt/gurobi751/linux64/`, you can run the CMake as follows:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DGUROBI_PATH=/opt/gurobi751/linux64 -DGUROBI_LIBRARY=gurobi75 ../source
make
```

## 3. Running the project

Inside the `experiments` directory, you can find a Python script `run.py` that performs the same experiment described in the manuscript that evaluates the performance of the algorithms. To run it, after compiling the project (as described in the previous section) and inside the `experiments` directory, run the following command:
```
python3 run.py
```  
After its completion, you will find a CSV file `results.csv` with the results of the experiments.

Note that to run this script you need the benchmark instances. These instances located in the directory `instances/benchmark/`. The scripts used to create them are located in the directory `instances/generator`, which can be used to create new instances.

However, if you want to run the optimizaton methods with other settings, you can run the executable created after the building the project. The subsection below shows some examples of how to use it and the section 4, *Parameters description*, shows and describes all parameters.


### 3.1. Usage examples

After build the project, you can run the executable created. Below we present some examples on how to run the executable. For the examples, consider `schd` as the name of the executable file after building and `instance.txt` as the name of the file containing the instance data and located at the same directory of the executable. A complete list of parameters is given in the next sections.

###### Show the help message:
```
./schd --help
```

###### Using the greedy heuristic:
```
./schd -v -s -d 3 --algorithm greedy --file instance.txt
```

In the example abore, the greedy heuristic is performed to find a solution.

###### Using the ILS-based heuristic:
```
./schd -v -s -d 3 --algorithm ils --file instance.txt
```

In the example abore, the ILS-based heuristic is performed to find a solution. It starts from the solution found by the greedy heuristic and try to find improved solutions using local search.

###### Using the MIP formulation based on precedence variables:
```
./schd -v -s -d 3 --algorithm mip-precedence --file instance.txt
```

In the example abore, the Gurobi is used to solve the MIP formulation based on precedence variables built from the instance data. As no time limit nor iteration (MIP nodes) limit are set, it stops when the optimal solution is found.

###### Using the MIP formulation based on linear ordering varaibles:
```
./schd -v -s -d 3 --algorithm mip-linear-ordering --file instance.txt
```

In the example abore, the Gurobi is used to solve the MIP formulation based on linear ordering variables built from the instance data. As no time limit nor iteration (MIP nodes) limit are set, it stops when the optimal solution is found.


## 4. Parameters description

#### 4.1. General parameters:

`-h`, `--help`  
Show a help message and exit.

`-f <VALUE>`, `--file <VALUE>`  
Name of the file containing the instance data.

`--algorithm <VALUE>`  
The algorithm used to solve the instance. Valid values are:
* `greedy`: Greedy heuristic.
* `ils`: ILS-based heuristic.
* `mip-precedence`: Solves the MIP formulation based on precedence variables using Gurobi solver.
* `mip-linear-ordering`: Solves the MIP formulation based on linear ordering variables using Gurobi solver.

`--seed <VALUE>`  
(Default: `0`)  
Set the seed used to initialize the random number generator used the methods (it is used to set the seed of Gurobi solver as well).

`--threads <VALUE>`  
(Default: `1`)  
Number of threads to be used (if the algorithms is able to use multithreading). If set to 0 (zero), all threads available are used.

`--time-limit <VALUE>`  
(Default: `1e100`)  
Limit the total time expended (in seconds).

`--iterations-limit <VALUE>`  
(Default: a very large number)  
Limit the total number of iterations expended. For MIP models, this parameters means the maximum number of MIP nodes explored.

#### 4.2. Printing parameters:

`-v`, `--verbose`  
Display the progress of the optimization process throughout its running.

`-s`, `--solution`  
Display the best solution found.

`-d`, `--details`  
(Default: `1`)  
Set the level of details to show at the end of the the optimization process. Valid values are:
* `0`: Show nothing;
* `1`: Show the status of the optimization process and the value of the objective function, if any;
* `2`: Show the status of the objective function, followed by the value of the objective function, the runtime in seconds, the number of iterations - or MIP nodes explored for MIP formulations -, the value of the linear relaxation, and the MIP optimality gap;
* `3`: Show a more detailed report about the optimization process.

For options `1` and `2`, all values are separated by a single blank space. If some information is not available, a question mark is printed in its place. The possible status are:
* `ERROR`: If an error occurred during the optimization process;
* `UNKNOWN`: The algorithm was not able to find a solution;
* `SUBOPTIMAL`: The algorithm found a feasible solution, but it was not able to prove its optimality.
* `OPTIMAL`: The algorithm found the optimal solution.
* `INFEASIBLE`: The algorithm returned an infeasible solution or the problem is infeasible.
* `UNBOUNDED`: The problem is unbounded.
* `INF_OR_UNBD`: The problem is infeasible or unbounded.

#### 4.3. ILS-based heuristic parameters:

`--perturbation-passes-limit <VALUE>`  
(Default: `5`)  
The highest value of perturbation strength. If no improvement is found after a perturbation with this strength, the ILS stops.

#### 4.4. MIP formulation parameters:

`--warm-start`  
If set, Gurobi will use the solution found by the greedy heuristic as starting solution.


## 5. Instance files

The instance files are plain text files. The data are formatted as follows:  
```
n m s

1 p[1] technology[1] action[1] stage[1]
2 p[3] technology[2] action[2] stage[2]
...
n p[n] technology[n] action[n] stage[n]

1 size(P[1]) P[1][1] P[1][2] ... P[1][size(P[1])]
2 size(P[2]) P[2][1] P[2][2] ... P[2][size(P[1])]
...
n size(P[n]) P[n][1] P[n][2] ... P[n][size(P[1])]

c[0][0][1] c[0][1][1] ... c[0][n][1]
c[1][0][1] c[1][1][1] ... c[1][n][1]
c[2][0][1] c[2][1][1] ... c[2][n][1]
                      ...
c[n][0][1] c[n][1][1] ... c[n][n][1]

c[0][0][2] c[0][1][2] ... c[0][n][2]
c[1][0][2] c[1][1][2] ... c[1][n][2]
c[2][0][2] c[2][1][2] ... c[2][n][2]
                      ...
c[n][0][2] c[n][1][2] ... c[n][n][2]

c[0][0][m] c[0][1][m] ... c[0][n][m]
c[1][0][m] c[1][1][m] ... c[1][n][m]
c[2][0][m] c[2][1][m] ... c[2][n][m]
                      ...
c[n][0][m] c[n][1][m] ... c[n][n][m]
```  
in which `n` is the number of switch operations, `m` is the number of maintenance teams available and `s` is the number of stages in which the switch operations are divided. After this data about the dimension of the instance, it follows the data about each switch operations, in which `p[i]` is the time to operate the switch `i`, `technology[i]` is the technology of the switch (`M` for manual and `R` for remote), `action[i]` is the type of operation to perform (`O` for openning and `C` for closing), and `stage[i]` is the stage the switch `i` belongs.

Next, it follows the precedence constraints. For each switch `i`, its predecessors are listed. For this, `size(P[i])` is the number of predecessors of `i` and `P[i][j]` is the j-th predecessor of the list. Finally, the displacement matrices described. For this, `c[i][j][l]` is the displacement time the team `l` takes to go from `i` to `j`.

