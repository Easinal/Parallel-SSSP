make
export LD_PRELOAD=/usr/local/lib/libjemalloc.so
folderOrigin="/data9/ezhu021/graph"
folderContract="/data9/zwan018/contract"
#2^11 - 2^25 "Germany"
steps=("8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216" "33554432")
# steps=("128" "256" "512" "1024" "2048" "4096" "8192" "16384" "32768")
radiuses=("0" "1" "2" "3" "4")
xs=("1" "2" "3" "4" "5")
# graphs=("com-orkut" "friendster")
graphs=("RoadUSA_sym_wgh" "Germany_sym_wgh" "hugebubbles-00020" "road-asia-osm" "hugetrace-00020" "europe_osm")
# "Germany_sym_wgh" "hugebubbles-00020" "road-asia-osm" "hugetrace-00020" "europe_osm"

for graph in ${graphs[@]}; do
    # for step in ${steps[@]}; do
    #     echo "$graph"
        # echo "$step"
        # echo "Rho Stepping"
        # ./sssp -i ${folderOrigin}/${graph}.adj -c ${folderContract}/${graph}.adj -p ${step} -w -v -a rho-stepping > ~/Parallel-SSSP/${graph}/rho_${step}.txt
        # echo "Delta Stepping"
        # ./sssp -i ${folderOrigin}/${graph}.adj -c ${folderContract}/${graph}.adj -p ${step} -w -v -a delta-stepping > ~/Parallel-SSSP/${graph}/delta_${step}.txt
    # done
    for radius in ${radiuses[@]}; do
        for x in ${xs[@]}; do
            ./sssp -i ${folderContract}/${graph}.adj -w -p 524288 -a delta-stepping -r ${radius} -x ${x} > ~/Parallel-SSSP-prev/${graph}/delta_${radius}_${x}.txt
        done
    done
    cd ${graph}
    python3 clean.py 
    cd ..
done

# ${folderOrigin}/${graph}.adj -c 
# folderOrigin="/data0/graphs/links"
# steps=("128" "256" "512" "1024" "2048" "4096" "8192" "16384" "32768")
# graphs=("com-orkut" "friendster")
# for graph in ${graphs[@]}; do
#     for step in ${steps[@]}; do
#         ./sssp -i ${folderOrigin}/${graph}_sym_wgh18.adj -c ${folderContract}/${graph}_sym_wgh18.adj -p ${step} -w -v -a rho-stepping > ~/Parallel-SSSP-prev/${graph}/rho_${step}.txt
#         ./sssp -i ${folderOrigin}/${graph}_sym_wgh18.adj -c ${folderContract}/${graph}_sym_wgh18.adj -p ${step} -w -v -a delta-stepping > ~/Parallel-SSSP-prev/${graph}/delta_${step}.txt
#     done
# done