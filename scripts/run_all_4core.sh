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

for i in `seq 1 70`;
do
#    echo $f $benchmark 
    num=$i

    output_dir="/scratch/cluster/akanksha/CRCRealOutput/4core/ledger/$output/"
    if [ ! -e "$output_dir" ] ; then
        mkdir $output_dir
        mkdir "$output_dir/scripts"
    fi
    condor_dir="$output_dir"
    script_name="$num"

    #cd $output_dir
    command="/u/akanksha/ChampSim_public/ChampSim_public/scripts/run_4core.sh $binary 0 250 $num $output_dir"
#    echo $command

    /u/akanksha/cache_study/condor_shell --silent --log --condor_dir="$condor_dir" --condor_suffix="$num" --output_dir="$output_dir/scripts" --simulate --script_name="$script_name" --cmdline="$command"

        #Submit the condor file
     /lusr/opt/condor/bin/condor_submit $output_dir/scripts/$script_name.condor

done

