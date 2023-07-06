make clean
make
export LD_PRELOAD=/usr/local/lib/libjemalloc.so
folderOrigin="/data9/ezhu021/graph"
folderContract="/data9/ezhu021/contract"
#2^11 - 2^25 "Germany"
steps=("131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216" "33554432")
graphs=("europe_osm")
for graph in ${graphs[@]}; do
    for step in ${steps[@]}; do
        echo "$graph"
        echo "$step"
        echo "Rho Stepping"
        ./sssp -i ${folderOrigin}/${graph}.adj -c ${folderContract}/${graph}.adj -p ${step} -w -v -a rho-stepping > ~/Parallel-SSSP/${graph}/rho_${step}.txt
        echo "Delta Stepping"
        ./sssp -i ${folderOrigin}/${graph}.adj -c ${folderContract}/${graph}.adj -p ${step} -w -v -a delta-stepping > ~/Parallel-SSSP/${graph}/delta_${step}.txt
    done
done
# folderOrigin="/data0/graphs/links"
# steps=("128" "256" "512" "1024" "2048" "4096" "8192" "16384" "32768")
# graphs=("com-orkut" "friendster")
# for graph in ${graphs[@]}; do
#     for step in ${steps[@]}; do
#         ./sssp -i ${folderOrigin}/${graph}_sym_wgh18.adj -c ${folderContract}/${graph}_sym_wgh18.adj -p ${step} -w -v -a rho-stepping > ~/Parallel-SSSP-prev/${graph}/rho_${step}.txt
#         ./sssp -i ${folderOrigin}/${graph}_sym_wgh18.adj -c ${folderContract}/${graph}_sym_wgh18.adj -p ${step} -w -v -a delta-stepping > ~/Parallel-SSSP-prev/${graph}/delta_${step}.txt
#     done
# done