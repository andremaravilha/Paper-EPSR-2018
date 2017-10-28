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
This script contains generates the instances used in the experiments described 
in the paper.
'''


import os
import argparse
import scheduling_generator as schd


if __name__ == "__main__":

    # Create command-line interface
    parser = argparse.ArgumentParser(description="Benchmark instances")
    parser.add_argument("path",
                        help="Path to directory where the instances will be saved.",
                        type=str)

    args = parser.parse_args()

    # Base name of instances
    name_prefix = ''
    name_suffix = '.txt'

    # Seed to initialize random number generator
    seed = 541;

    # Common settings
    integer_only = True             # Integer data only
    handle_time = 1                 # All switches take an unitary cost to be handled
    travel_time = [5, 20]           # Travel time ranges from 10 to 60 (expect for euclidean)
    prec = ['S', 'T', 'I', 'G']     # All types of precedence graph
    #symmetry = ['E', 'S', 'A']     # All types of travel time matrix is create for each instance size
    symmetry = ['E']
    

    # Number of switches
    nswitches = [10, 20, 60]

    # Settings for each value of switch
    data = {10:  {'teams':     [2, 4],
                  'stages':    [3],
                  'tx-remote': [10]},

            20:  {'teams':     [4, 6],
                  'stages':    [3, 5],
                  'tx-remote': [10]},

            60:  {'teams': [6, 10],
                  'stages': [5, 10],
                  'tx-remote': [10]}}

    # Create the output directory if it does not exist
    if not os.path.exists(args.path):
        os.makedirs(args.path)

    # Create instance files
    for n in nswitches:
        for sym_opt in symmetry:
            for prec_opt in prec:
                for m in data[n]['teams']:
                    for s in data[n]['stages']:
                        for tx_remote in data[n]['tx-remote']:

                            # Get the symmetry option
                            sym_value = ""
                            if sym_opt == "E":
                                sym_value = "euclidean"
                            elif sym_opt == "S":
                                sym_value = "symmetric"
                            elif sym_opt == "A":
                                sym_value = "asymmetric"

                            # Get the type of precedence graph
                            prec_value = ""
                            if prec_opt == "G":
                                prec_value = "general"
                            elif prec_opt == "I":
                                prec_value = "independent"
                            elif prec_opt == "T":
                                prec_value = "intree"
                            elif prec_opt == "S":
                                prec_value = "sequential"

                            # Create the instance
                            instance = schd.create_instance(n, m, s, prec_value, (tx_remote / 100), sym_value, handle_time,
                                                            [handle_time, handle_time], travel_time, integer_only, seed)

                            # Name of the instance file
                            #filename = "{}{}_{}_{}_{}_{}_{}{}".format(name_prefix, n, m, s, tx_remote, sym_opt, prec_opt, name_suffix)
                            filename = "{}{}_{}_{}_{}_{}{}".format(name_prefix, n, m, s, tx_remote, prec_opt, name_suffix)
                            path = os.path.join(args.path, filename)

                            # Write the instance to file
                            schd.write_instance(path, instance)

