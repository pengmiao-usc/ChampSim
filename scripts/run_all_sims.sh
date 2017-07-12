#!/bin/bash
if [ $# -lt 1 ] 
then
    echo "Usage : ./run_all_sims.sh <binary> <output>"
    exit
fi

binary=$1
echo $binary
output=$2
echo $output

for f in /scratch/cluster/akanksha/CRCRealTraces/*.gz
do
    benchmark=$(basename "$f")
    benchmark="${benchmark%.*}"
    benchmark="${benchmark%.*}"
    trace_file=$benchmark
#    echo $f $benchmark 

    output_dir="/scratch/cluster/akanksha/CRCRealOutput/ledger/$output/"
    if [ ! -e "$output_dir" ] ; then
        mkdir $output_dir
        mkdir "$output_dir/scripts"
    fi
    condor_dir="$output_dir"
    script_name="$benchmark"

    #cd $output_dir
    command="/u/akanksha/ChampSim_public/ChampSim_public/scripts/run_champsim.sh $binary 0 1000 $trace_file $output_dir"

    /u/akanksha/cache_study/condor_shell --silent --log --condor_dir="$condor_dir" --condor_suffix="$benchmark" --output_dir="$output_dir/scripts" --simulate --script_name="$script_name" --cmdline="$command"

        #Submit the condor file
     /lusr/opt/condor/bin/condor_submit $output_dir/scripts/$script_name.condor

done

