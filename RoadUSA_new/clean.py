import re
st1 = ["delta","rho"]
st2=["8192", "16384", "32768", "65536", "131072", "262144", "524288", "1048576", "2097152", "4194304", "8388608", "16777216", "33554432"]

def main():
    with open("average.txt", "w") as f1:
        with open("median.txt", "w") as f2:
            for graph in st1:
                f1.write("{0}\nstep\torigin_time\tcontract_time\n".format(graph))
                f2.write("{0}\nstep\torigin_time\tcontract_time\n".format(graph))
                for tp in st2:
                    f1.write("{0}\t".format(tp))
                    f2.write("{0}\t".format(tp))
                    source_num = 0
                    tot_avg_time_origin = 0
                    tot_med_time_origin = 0
                    tot_avg_time_contract = 0
                    tot_med_time_contract = 0
                    with open("{0}_{1}.txt".format(graph, tp), "r") as sf:
                        fl = sf.readlines()
                        for line in fl:
                            sr = line.split()
                            if(sr[0]=="average"):
                                source_num += 1
                                tot_avg_time_origin += float(sr[3])
                                tot_avg_time_contract += float(sr[4])
                                #f1.write(sr[3]+"\n")
                            if(sr[0]=="median"):
                                tot_med_time_origin +=  float(sr[3])
                                tot_med_time_contract += float(sr[4])
                                #f2.write(sr[3]+"\n")
                        #
                    f1.write(str(tot_avg_time_origin/source_num)+"\t"+str(tot_avg_time_contract/source_num)+"\n")
                    f2.write(str(tot_med_time_origin/source_num)+"\t"+str(tot_med_time_contract/source_num)+"\n")
                    sf.close()
        f2.close()
    f1.close()

if __name__ == "__main__":
    main()
#