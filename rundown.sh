./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 200000 -w -v -c -a rho-stepping > result/Germany_rho.txt
./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 200000 -w -v -c -a delta-stepping > result/Germany_delta.txt
./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 200000 -w -v -c -a bellman-ford > result/Germany_bf.txt
./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 200000 -w -v -c -a rho-stepping > result/USA_rho.txt
./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 200000 -w -v -c -a delta-stepping > result/USA_delta.txt
./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 200000 -w -v -c -a bellman-ford > result/USA_bf.txt
