make clean
make
folderOrigin="/data0/zwan018/origin"
folderContract="/data0/zwan018/contract"
folderStar="/data0/zwan018/star"
#2^11 - 2^25  
steps=("4096" "8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216" "33554432")
graphs=("RoadUSA" "Germany" )
./sssp -i ${folderOrigin}/RoadUSA_sym_wgh.adj -f ${folderContract}/RoadUSA_sym_wgh.adj -d ${folderStar}/RoadUSA_sym_wgh.adj -p 65536 -w -v -c -a rho-stepping > RoadUSA_baseline/rho_65536.txt
./sssp -i ${folderOrigin}/RoadUSA_sym_wgh.adj -f ${folderContract}/RoadUSA_sym_wgh.adj -d ${folderStar}/RoadUSA_sym_wgh.adj -p 262144 -w -v -c -a delta-stepping > RoadUSA_baseline/delta_65536.txt
./sssp -i ${folderOrigin}/Germany_sym_wgh.adj -f ${folderContract}/Germany_sym_wgh.adj -d ${folderStar}/Germany_sym_wgh.adj -p 65536 -w -v -c -a rho-stepping > Germany_baseline/rho_65536.txt
./sssp -i ${folderOrigin}/Germany_sym_wgh.adj -f ${folderContract}/Germany_sym_wgh.adj -d ${folderStar}/Germany_sym_wgh.adj -p 2097152 -w -v -c -a delta-stepping > Germany_baseline/delta_2097152.txt


#./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -r /data0/zwan018/residual/Germany_sym_wgh.adj -p 2097152 -w -v -c -a rho-stepping > result/Germany_rho.txt
#./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -c -a delta-stepping > result/Germany_delta.txt
#./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -c -a bellman-ford > result/Germany_bf.txt
#./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a rho-stepping > result/USA_rho.txt
#./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a delta-stepping > result/USA_delta.txt
#./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a bellman-ford > result/USA_bf.txt
