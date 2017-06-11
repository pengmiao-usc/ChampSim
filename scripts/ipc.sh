# Collect IPC value from results_50M with specified binary name
# Usage: ./ipc.sh 50 $BINARY_NAME

num_instr=${1}
binary=${2}

while read line
do
    ipc=`grep "Finished" results_${num_instr}M/${line}-${binary}.txt | awk '{print $10}'`
    echo $ipc
done < sim_list/run_list.txt
