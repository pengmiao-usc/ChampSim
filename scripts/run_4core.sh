TRACE_DIR=/scratch/cluster/akanksha/CRCRealTraces
binary=${1}
n_warm=${2}
n_sim=${3}
num=${4}
outputdir=${5}
option=${6}

trace1=`sed -n ''$num'p' /u/akanksha/ChampSim_public/ChampSim_public/sim_list/4core_workloads.txt | awk '{print $1}'`
trace2=`sed -n ''$num'p' /u/akanksha/ChampSim_public/ChampSim_public/sim_list/4core_workloads.txt | awk '{print $2}'`
trace3=`sed -n ''$num'p' /u/akanksha/ChampSim_public/ChampSim_public/sim_list/4core_workloads.txt | awk '{print $3}'`
trace4=`sed -n ''$num'p' /u/akanksha/ChampSim_public/ChampSim_public/sim_list/4core_workloads.txt | awk '{print $4}'`

echo "(/u/akanksha/ChampSim_public/ChampSim_public/bin/${binary} -warmup_instructions ${n_warm}000000 -simulation_instructions ${n_sim}000000 ${option} -hide_heartbeat -traces ${TRACE_DIR}/${trace1}.trace.gz ${TRACE_DIR}/${trace2}.trace.gz ${TRACE_DIR}/${trace3}.trace.gz ${TRACE_DIR}/${trace4}.trace.gz) &> ${outputdir}/mix${num}.txt"
(/u/akanksha/ChampSim_public/ChampSim_public/bin/${binary} -warmup_instructions ${n_warm}000000 -simulation_instructions ${n_sim}000000 ${option} -hide_heartbeat -traces ${TRACE_DIR}/${trace1}.trace.gz ${TRACE_DIR}/${trace2}.trace.gz ${TRACE_DIR}/${trace3}.trace.gz ${TRACE_DIR}/${trace4}.trace.gz) &> ${outputdir}/mix${num}.txt
