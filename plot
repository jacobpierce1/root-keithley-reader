#!/bin/bash
# this script will execute plot_keithley.cpp in root, allowing for command line args.

# construct single string from all args
args='("'
while [ $1 != "" ]; do
    args=$args$1'!'  # this delimiter cannot be used anywhere in input (title, legend, etc)
    shift
done
args=$args$1'")'

# run the file using args
file="plot_keithley.cpp"$args
echo $file
root $file
