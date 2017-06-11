PREFETCHER=daampm
PREF1=daampm
POLICY1=pacman
PREF2=daampm
POLICY2=ship
MY_PREF=sppv2
MY_REPL=igc3

echo "                 8MB-LRU    LRU     ${POLICY1}    ${POLICY2}     ${MY_REPL}"
app1=`sed -n ''$1'p' mix_list.txt | awk '{print $1}'`
solo1=`grep "Final" -A 1 ./results_1B/${app1}-daampm-lru8mb-1core.txt | grep IPC | awk '{print $8}'`
result11=`grep -E "Final.*${app1}" -A 1 ./results_4core/mix${1}-${PREFETCHER}-lru-4core.txt | grep IPC | awk '{print $8}'`
result12=`grep -E "Final.*${app1}" -A 1 ./results_4core/mix${1}-${PREF1}-${POLICY1}-4core.txt | grep IPC | awk '{print $8}'`
result13=`grep -E "Final.*${app1}" -A 1 ./results_4core/mix${1}-${PREF2}-${POLICY2}-4core.txt | grep IPC | awk '{print $8}'`
result14=`grep -E "Final.*${app1}" -A 1 ./results_4core/mix${1}-${MY_PREF}-${MY_REPL}-4core.txt | grep IPC | awk '{print $8}'`
#echo "$app1 $solo1 $result11 $result12 $result13 $result14"
printf "%-16s" "$app1" "$solo1 $result11 $result12 $result13 $result14"
echo ""

app2=`sed -n ''$1'p' mix_list.txt | awk '{print $2}'`
solo2=`grep "Final" -A 1 ./results_1B/${app2}-daampm-lru8mb-1core.txt | grep IPC | awk '{print $8}'`
result21=`grep -E "Final.*${app2}" -A 1 ./results_4core/mix${1}-${PREFETCHER}-lru-4core.txt | grep IPC | awk '{print $8}'`
result22=`grep -E "Final.*${app2}" -A 1 ./results_4core/mix${1}-${PREF1}-${POLICY1}-4core.txt | grep IPC | awk '{print $8}'`
result23=`grep -E "Final.*${app2}" -A 1 ./results_4core/mix${1}-${PREF2}-${POLICY2}-4core.txt | grep IPC | awk '{print $8}'`
result24=`grep -E "Final.*${app2}" -A 1 ./results_4core/mix${1}-${MY_PREF}-${MY_REPL}-4core.txt | grep IPC | awk '{print $8}'`
#echo "$app2 $solo2 $result21 $result22 $result23 $result24"
printf "%-16s" "$app2" "$solo2 $result21 $result22 $result23 $result24"
echo ""

app3=`sed -n ''$1'p' mix_list.txt | awk '{print $3}'`
solo3=`grep "Final" -A 1 ./results_1B/${app3}-daampm-lru8mb-1core.txt | grep IPC | awk '{print $8}'`
result31=`grep -E "Final.*${app3}" -A 1 ./results_4core/mix${1}-${PREFETCHER}-lru-4core.txt | grep IPC | awk '{print $8}'`
result32=`grep -E "Final.*${app3}" -A 1 ./results_4core/mix${1}-${PREF1}-${POLICY1}-4core.txt | grep IPC | awk '{print $8}'`
result33=`grep -E "Final.*${app3}" -A 1 ./results_4core/mix${1}-${PREF2}-${POLICY2}-4core.txt | grep IPC | awk '{print $8}'`
result34=`grep -E "Final.*${app3}" -A 1 ./results_4core/mix${1}-${MY_PREF}-${MY_REPL}-4core.txt | grep IPC | awk '{print $8}'`
#echo "$app3 $solo3 $result31 $result32 $result33 $result34"
printf "%-16s" "$app3" "$solo3 $result31 $result32 $result33 $result34"
echo ""

app4=`sed -n ''$1'p' mix_list.txt | awk '{print $4}'`
solo4=`grep "Final" -A 1 ./results_1B/${app4}-daampm-lru8mb-1core.txt | grep IPC | awk '{print $8}'`
result41=`grep -E "Final.*${app4}" -A 1 ./results_4core/mix${1}-${PREFETCHER}-lru-4core.txt | grep IPC | awk '{print $8}'`
result42=`grep -E "Final.*${app4}" -A 1 ./results_4core/mix${1}-${PREF1}-${POLICY1}-4core.txt | grep IPC | awk '{print $8}'`
result43=`grep -E "Final.*${app4}" -A 1 ./results_4core/mix${1}-${PREF2}-${POLICY2}-4core.txt | grep IPC | awk '{print $8}'`
result44=`grep -E "Final.*${app4}" -A 1 ./results_4core/mix${1}-${MY_PREF}-${MY_REPL}-4core.txt | grep IPC | awk '{print $8}'`
#echo "$app4 $solo4 $result41 $result42 $result43 $result44"
printf "%-16s" "$app4" "$solo4 $result41 $result42 $result43 $result44"
echo ""

#weighted_sum_lru=$(echo "scale=6;$result1/$solo" | bc)

weighted_sum1=$(echo "scale=6;$result11/$solo1 + $result21/$solo2 + $result31/$solo3 + $result41/$solo4" | bc)
weighted_sum2=$(echo "scale=6;$result12/$solo1 + $result22/$solo2 + $result32/$solo3 + $result42/$solo4" | bc)
weighted_sum3=$(echo "scale=6;$result13/$solo1 + $result23/$solo2 + $result33/$solo3 + $result43/$solo4" | bc)
weighted_sum4=$(echo "scale=6;$result14/$solo1 + $result24/$solo2 + $result34/$solo3 + $result44/$solo4" | bc)

norm_policy1=$(echo "scale=6;$weighted_sum2/$weighted_sum1" | bc)
norm_policy2=$(echo "scale=6;$weighted_sum3/$weighted_sum1" | bc)
norm_mypolicy=$(echo "scale=6;$weighted_sum4/$weighted_sum1" | bc)
#echo $norm_policy1 $norm_policy2 $norm_mypolicy
printf "%-16s" "Weighted IPC" "                  $norm_policy1 $norm_policy2 $norm_mypolicy"
echo""
echo""
