#!/usr/bin/env python3

from collections import defaultdict
import os
import re
import subprocess
import sys

if __name__ != "__main__":
	print("Cannot be used as a module.", file = sys.stderr)
	sys.exit(1)

benchmark_pattern = re.compile(r"\d\d\d\.(?P<benchmark>\S*)")
weight_pattern    = re.compile(r"(?P<weight>\d+\.?\d*) (?P<offset>\d+)")
compute_pattern   = re.compile(r"(?P<benchmark>.*)_(?P<offset>\d+)B, "
                               r"(?P<value>[+-]?\d+\.?\d*)")

weights = {}
with open("/scratch/cluster/akanksha/CRCRealTraces/spec_simpoint_info.txt",
          "r") as simpoint_info:
	current_benchmark = None
	for line in simpoint_info:
		match = benchmark_pattern.match(line)
		if match:
			current_benchmark = match.group("benchmark")
			weights[current_benchmark] = {}
		else:
			match = weight_pattern.match(line)
			if match:
				weights[current_benchmark][int(match.group("offset"))] \
                = float(match.group("weight"))

values = defaultdict(list)
with subprocess.Popen([os.path.abspath(sys.argv[1])] + sys.argv[2:],
                      stdout = subprocess.PIPE,
                      bufsize = 1,
                      universal_newlines = True) as compute:
	for line in compute.stdout:
		match = compute_pattern.match(line)
		if match:
			simpoint = (int(match.group("offset")),
			            float(match.group("value")))
			values[match.group("benchmark")].append(simpoint)

overall_mean = 0
overall_geomean = 1
n = 0
for benchmark, simpoints in sorted(values.items(),
                                   key = lambda pair: pair[0].lower()):
	if benchmark not in weights:
		continue
	weighted_geometric_mean = 1
	weights_sum = 0
	all_negative = True
	for offset, value in simpoints:
		value += 100
		if value > 0:
			all_negative = False
		weight = weights[benchmark][offset]
		weighted_geometric_mean *= abs(value) ** weight
		weights_sum += weight
	weighted_geometric_mean **= 1 / weights_sum
	if all_negative:
		weighted_geometric_mean *= -1
	weighted_geometric_mean -= 100
	overall_mean += weighted_geometric_mean
	overall_geomean *= weighted_geometric_mean + 100
	n += 1
	print(benchmark + ", " + str(weighted_geometric_mean))
overall_mean /= n
overall_geomean **= 1 / n
overall_geomean -= 100
print("Mean: " + str(overall_mean))
print("GeoMean: " + str(overall_geomean))
