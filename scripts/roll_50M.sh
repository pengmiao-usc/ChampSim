# Run all benchmarks registed in sim_list/run_list.txt 
# Warmup 10M instructions and run 50M detailed instructions
# Usage: ./roll_50M.sh $BINARY_NAME

binary=$1
option=$2

while read trace
do
    ./run_champsim.sh ${binary} 10 50 ${trace} ${option} &
done < sim_list/run_list.txt
