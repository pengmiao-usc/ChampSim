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

miss_reduction_average=0
count=`ls -lh $baseline/*.txt | wc -l`
echo $count

for f in /scratch/cluster/akanksha/CRCRealTraces/*.gz
do
    benchmark=$(basename "$f")
    benchmark="${benchmark%.*}"
    benchmark="${benchmark%.*}"
    baseline_file="$baseline/$benchmark"".txt"
    dut_file="$dut/$benchmark"".txt"

    #miss_reduction=`perl hits.pl $name $config $policy`
    miss_reduction=`perl miss-reduction.pl $baseline_file $dut_file`
    echo "$benchmark, $miss_reduction"
    miss_reduction_average=`perl arithmean.pl $miss_reduction $miss_reduction_average $count`
done

echo "Average: $miss_reduction_average"
