folderOrigin="/data0/zwan018/origin"
folderContract="/data0/zwan018/contract"
rho=("1000" "3000" "10000" "30000" "100000" "300000" "1000000" "3000000" "10000000" "30000000" )
delta=("1000" "3000" "10000" "30000" "100000" "300000" "1000000" "3000000" "10000000" "30000000" )
graphs=("Germany" "USA")

for graph in ${graphs[@]}; do
    for step in ${rho[@]}; do
        ./sssp -i ${folderOrigin}/${graph}_sym_wgh.adj -f ${folderContract}/${graph}_sym_wgh.adj -p ${step} -w -v -a rho-stepping > ${graph}/rho_${step}.txt
    done
    for step in ${delta[@]}; do
        ./sssp -i ${folderOrigin}/${graph}_sym_wgh.adj -f ${folderContract}/${graph}_sym_wgh.adj -p ${step} -w -v -a delta-stepping > ${graph}/delta_${step}.txt
    done
done

#./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -a rho-stepping > result2/Germany_rho.txt
#./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -a delta-stepping > result2/Germany_delta.txt
#./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -a rho-stepping > result2/USA_rho.txt
#./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -a delta-stepping > result2/USA_delta.txt
