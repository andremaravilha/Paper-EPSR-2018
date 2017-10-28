# MIT License
# 
# Copyright (c) 2017 André L. Maravilha
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

'''
This script contains functions to create instances for the maneuvers scheduling 
problem in the restoration of electric power distribution networks. Instances of 
this problem are equivalent to instances of the identical parallel machines 
scheduling problem with precedence constraints and setup times dependent on the 
sequence and machine (P | s_{i,j,k}, prec | C_{max}).
'''


import argparse
import random
import math


class Instance:
    """
    Instance of the Maneuvers Scheduling Problem.

    :param n:
    :param m:
    :param s:
    :param p:
    :param c:
    :param prec:
    :param technology:
    :param operation:
    :param stage:
    """

    def __init__(self, n, m, s, p, c, prec, technology, operation, stage):
        self.n = n
        self.m = m
        self.s = s
        self.p = p
        self.c = c
        self.prec = prec
        self.technology = technology
        self.operation = operation
        self.stage = stage



def create_instance(n, m, s, prec, tx_remote, symmetry, hdl_remote, hdl_limits, travel_limits, integer_only, seed):
    """
    Create an instance for the maneuverable scheduling.

    :param n: Number or switches.
    :param m: Number of teams.
    :param s: Number of stages.
    :param prec: type of precedence graph (independent, intree, sequential, general)
    :param tx_remote: Proportion of switches remotely handled.
    :param symmetry: Strategy used to define the travel time matrix (euclidean, symmetric, asymmetric).
    :param hdl_remote: Time to handle remotely handled switches.
    :param hdl_limits: A list as [lb, ub] that limits the time to manually handle switches.
    :param travel_limits: A list as [lb, ub] that limits the travel time between pair of locations.
    :param integer_only: Whether the handle time and travel time should be integer values only.
    :param seed: Seed used to initialize the randon number generator.
    :return: An instance of the switch operations scheduling problem
    """

    # Initialize the seed of the random number generator
    random.seed(seed)

    # Switches: technology
    technology = ["M"] * n;
    for i in random.sample(range(0, n), math.ceil(n * tx_remote)):
        technology[i] = "R"

    # Switches: time for handling
    p = [0] * n
    for i in range(0, n):
        if technology[i] == "R":
            p[i] = hdl_remote
        else:
            if integer_only:
                p[i] = random.randint(math.ceil(hdl_limits[0]), math.floor(hdl_limits[1]))
            else:
                p[i] = round(random.uniform(hdl_limits[0], hdl_limits[1]), 5)

    # Switches: operation
    operation = ["O"] * n
    operation[-1] = "C"
    for i in random.sample(range(0, n), s - 1):
        operation[i] = "C"

    # Switches: stage
    stage = [0] * n
    current_stage = 1
    for i in range(0, n):
        stage[i] = current_stage
        if operation[i] == "C":
            current_stage = current_stage + 1

    # Travel time
    c = [[[0 for i in range(0, n+1)] for i in range(0, n+1)] for l in range(0, m)]

    if symmetry == "euclidean":

        # Define coordinates
        xcoord = [round(random.uniform(travel_limits[0], travel_limits[1]), 3) for i in range(0, n+1)]
        ycoord = [round(random.uniform(travel_limits[0], travel_limits[1]), 3) for i in range(0, n+1)]

        # Compute travel time
        for l in range(0, m):
            for i in range(0, n+1):
                for j in range(i+1, n+1):
                    val = math.sqrt(math.pow(xcoord[i] - xcoord[j], 2) + math.pow(ycoord[i] - ycoord[j], 2))
                    val = round(val) if integer_only else round(val, 5)
                    c[l][i][j] = val
                    c[l][j][i] = val

    elif symmetry == "symmetric":

        # Compute travel time
        for l in range(0, m):
            for i in range(0, n+1):
                for j in range(i+1, n+1):
                    val = random.uniform(travel_limits[0], travel_limits[1])
                    val = round(val) if integer_only else round(val, 5)
                    c[l][i][j] = val
                    c[l][j][i] = val

    elif symmetry == "asymmetric":

        # Compute travel time
        for l in range(0, m):
            for i in range(0, n+1):
                for j in range(0, n+1):
                    if (i != j):
                        val = random.uniform(travel_limits[0], travel_limits[1])
                        val = round(val) if integer_only else round(val, 5)
                        c[l][i][j] = val

    # Precedence constraints
    P = [[] for i in range(0, n)]

    switches_to_open = [[] for i in range(0, s + 1)]
    switches_to_close = [[] for i in range(0, s + 1)]
    for i in range(0, n):
        if operation[i] == "C":
            switches_to_close[stage[i]].append(i)
        elif operation[i] == "O":
            switches_to_open[stage[i]].append(i)

    if prec == "independent":
        for j in range(0, n):
            if operation[j] == "C":
                for i in switches_to_open[stage[j]]:
                    P[j].append(i)

    elif prec == "intree":
        for j in range(0, n):
            if operation[j] == "C":
                for i in switches_to_open[stage[j]]:
                    P[j].append(i)
                for i in switches_to_close[stage[j] - 1]:
                    P[j].append(i)

    elif prec == "sequential":
        for j in range(0, n):
            if operation[j] == "O":
                for i in switches_to_close[stage[j] - 1]:
                    P[j].append(i)
            elif operation[j] == "C":
                if len(switches_to_open[stage[j]]) == 0:
                    for i in switches_to_close[stage[j] - 1]:
                        P[j].append(i)
                else:
                    for i in switches_to_open[stage[j]]:
                        P[j].append(i)

    elif prec == "general":

        # First stage
        j = switches_to_close[1][0]
        for i in switches_to_open[stage[j]]:
            P[j].append(i)

        # Next stages
        for current_stage in range(2, s + 1):
            prec_relation = random.choice(["independent", "intree", "sequential"])
            if prec_relation == "independent":
                j = switches_to_close[current_stage][0]
                for i in switches_to_open[stage[j]]:
                    P[j].append(i)
            elif prec_relation == "intree":
                j = switches_to_close[current_stage][0]
                for i in switches_to_open[stage[j]]:
                    P[j].append(i)
                for i in switches_to_close[stage[j] - 1]:
                    P[j].append(i)
            elif prec_relation == "sequential":
                for j in switches_to_open[current_stage]:
                    for i in switches_to_close[stage[j] - 1]:
                        P[j].append(i)
                j = switches_to_close[current_stage][0]
                if len(switches_to_open[stage[j]]) == 0:
                    for i in switches_to_close[stage[j] - 1]:
                        P[j].append(i)
                else:
                    for i in switches_to_open[stage[j]]:
                        P[j].append(i)

    # Create and return the intance of the problem
    return Instance(n, m, s, p, c, P, technology, operation, stage)



def write_instance(filename: str, instance: Instance):
    """
    Write the instance data into a plain text file.

    :param filename: Path and name of the file in which the instance will be written.
    :param instance: The instance that will be saved to file.
    """

    with open(filename, "w") as file:

        # Instance size
        file.write("{} {} {}\n".format(instance.n, instance.m, instance.s))

        # Switches data
        for i in range(0, instance.n):
            file.write("{} {} {} {} {}\n".format(i+1,
                                                 instance.p[i],
                                                 instance.technology[i],
                                                 instance.operation[i],
                                                 instance.stage[i]))

        # Precedence constraints
        for i in range(0, instance.n):
            file.write("{} {} ".format(i+1, len(instance.prec[i])))
            for j in instance.prec[i]:
                file.write("{} ".format(j+1))
            file.write("\n")

        # Travel time
        for l in range(0, instance.m):
            for i in range(0, instance.n + 1):
                for j in range (0, instance.n + 1):
                    file.write("{} ".format(instance.c[l][i][j]))
                file.write("\n")



def __create_instance(args):
    return create_instance(args.switches, args.teams, args.stages, args.prec, args.remote, args.symmetry,
                           args.handle_time_remote, [args.handle_time_min, args.handle_time_max],
                           [args.travel_time_min, args.travel_time_max], args.integer_only,
                           args.seed)



def __create_cli():
    """
    Create the Command-Line Interface of the application.

    :return: The command-line interface for parsing the user input.
    """

    parser = argparse.ArgumentParser(description="Instance generator for the Maneuvers Scheculing Problem")
    parser.add_argument("--filename",
                        help="Path and name of the file in which the instance will be saved.",
                        type=str,
                        required=True)
    parser.add_argument("--switches",
                        help="Number of switches.",
                        type=int,
                        required=True)
    parser.add_argument("--teams",
                        help="Number of teams available.",
                        type=int,
                        required=True)
    parser.add_argument("--stages",
                        help="Number of stages.",
                        type=int,
                        required=True)
    parser.add_argument("--prec",
                        help="Type of the precedence graph.",
                        type=str,
                        choices=["general", "independent", "intree", "sequential"],
                        default="general")
    parser.add_argument("--remote",
                        help="Proportion of switches remotely handled.",
                        type=float,
                        required=True)
    parser.add_argument("--seed",
                        help="Seed used to initialize the random number generator.",
                        type=int,
                        default=0)
    parser.add_argument("--integer-only",
                        help="Whether the values generated for this instances should "
                             "be truncated to integer values.",
                        action="store_true",
                        dest="integer_only")
    parser.add_argument("--symmetry",
                        help="Whether the travel distance matrix should be symmetric.",
                        type=str,
                        choices=["euclidean", "symmetric", "asymmetric"],
                        default="euclidean")
    parser.add_argument("--handle-time-remote",
                        help="Handle time for remotely handled switches.",
                        type=float,
                        default=1,
                        dest="handle_time_remote")
    parser.add_argument("--handle-time-min",
                        help="Minimum value for time to handle a manual switch.",
                        type=float,
                        default=1,
                        dest="handle_time_min")
    parser.add_argument("--handle-time-max",
                        help="Maximum value for time to handle a manual switch.",
                        type=float,
                        default=1,
                        dest="handle_time_max")
    parser.add_argument("--travel-time-min",
                        help="Defines the lower limit to travel time values. If the "
                             "option 'symmetry' is set to 'euclidean', this option "
                             "is interpreted as the lower limit of the coordinate "
                             "values used to determine the travel time.",
                        type=float,
                        default=10,
                        dest="travel_time_min")
    parser.add_argument("--travel-time-max",
                        help="Defines the upper limit to travel time values. If the "
                             "option 'symmetry' is set to 'euclidean', this option "
                             "is interpreted as the upper limit of the coordinate "
                             "values used to determine the travel time.",
                        type=float,
                        default=60,
                        dest="travel_time_max")


    return parser



if __name__ == "__main__":

    # Parse user arguments
    cli = __create_cli()
    args = cli.parse_args()

    # Create instance
    instance = __create_instance(args)

    # Write file with instance data
    write_instance(args.filename, instance)
