#!/bin/bash
if [ $# -lt 2 ] 
then
    echo "Usage : ./run_all_sims.sh <baseline> <dut>"
    exit
fi

baseline=$1
echo $baseline
dut=$2
echo $dut

speedup_average=1.0
count=`ls -lh $baseline/*.txt | wc -l`
echo $count

for f in /scratch/cluster/akanksha/CRCRealTraces/*.gz
do
    benchmark=$(basename "$f")
    benchmark="${benchmark%.*}"
    benchmark="${benchmark%.*}"
    baseline_file="$baseline/$benchmark"".txt"
    dut_file="$dut/$benchmark"".txt"
    
#    echo "$baseline_file $dut_file"
 
    speedup=`perl speedup.pl $baseline_file $dut_file`
    echo "$benchmark, $speedup"
    speedup_average=`perl geomean.pl $speedup $speedup_average $count`
done

echo "Average: $speedup_average"
