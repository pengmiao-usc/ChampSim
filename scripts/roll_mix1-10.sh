# Run 4-core experiments (mix1 ~ mix10) registed in sim_list/4core_workloads.txt 
# Warmup 10M instructions and run 50M detailed instructions
# Usage: ./roll_mix1-10.sh $BINARY_NAME

binary=$1
option=$2
num=0

while [ $num -lt 10 ]
do
    ((num++))
    ./run_4core.sh ${binary} 10 50 ${num} ${option} &
done
