make clean
make
folderOrigin="/data/zwan018/origin"
folderContract="/data/zwan018/contract"
folderStar="/data/zwan018/star"
folderResidual="/data/zwan018/residual"
#2^11 - 2^25 
steps=("8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216" "33554432")
#"RoadUSA"
graphs=("Germany" )
for graph in ${graphs[@]}; do
    for step in ${steps[@]}; do
        ./sssp -i ${folderOrigin}/${graph}_sym_wgh.adj -f ${folderContract}/${graph}_sym_wgh.adj -d ${folderStar}/${graph}_sym_wgh.adj -r ${folderResidual}/${graph}_sym_wgh.adj -p ${step} -w -v -c -a rho-stepping > ${graph}_new/rho_${step}.txt
        ./sssp -i ${folderOrigin}/${graph}_sym_wgh.adj -f ${folderContract}/${graph}_sym_wgh.adj -d ${folderStar}/${graph}_sym_wgh.adj -r ${folderResidual}/${graph}_sym_wgh.adj -p ${step} -w -v -c -a delta-stepping > ${graph}_new/delta_${step}.txt
    done
done

#./sssp -i /data/zwan018/origin/Germany_sym_wgh.adj -f /data/zwan018/contract/Germany_sym_wgh.adj -d /data/zwan018/star/Germany_sym_wgh.adj -r /data/zwan018/residual/Germany_sym_wgh.adj -p 2097152 -w -v -c -a rho-stepping
#./sssp -i /data/zwan018/origin/RoadUSA_sym_wgh.adj -f /data/zwan018/contract/RoadUSA_sym_wgh.adj -d /data/zwan018/star/RoadUSA_sym_wgh.adj -r /data/zwan018/residual/RoadUSA_sym_wgh.adj -p 16384 -w -v -c -a rho-stepping > result2/Germany_rho.txt
#./sssp -i /data/zwan018/origin/Germany_sym_wgh.adj -f /data/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -a delta-stepping > result2/Germany_delta.txt
#./sssp -i /data/zwan018/origin/RoadUSA_sym_wgh.adj -f /data/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -a rho-stepping > result2/USA_rho.txt
#./sssp -i /data/zwan018/origin/RoadUSA_sym_wgh.adj -f /data/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -a delta-stepping > result2/USA_delta.txt
