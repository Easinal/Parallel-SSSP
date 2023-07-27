import re
st1 = ["delta","rho"]
st2=["8192",  "16384", "32768", "65536", "131072", "262144"]
#"16384", 
def main():
    with open("average.txt", "w") as f1:
        for graph in st1:
            f1.write("{0}\nstep\torigin_time\tcontract_time\n".format(graph))
            for tp in st2:
                f1.write("{0}\t".format(tp))
                source_num = 0
                tot_avg_time_origin = 0
                tot_avg_time_contract = 0
                with open("{0}_{1}.txt".format(graph, tp), "r") as sf:
                    fl = sf.readlines()
                    for line in fl:
                        sr = line.split()
                        if(len(sr)==0):continue
                        if(sr[0]=="Average"):
                            source_num += 1
                            tot_avg_time_origin += float(sr[2])
                            tot_avg_time_contract += float(sr[3])
                            #print(sr[2]+"\t"+sr[3]+"\n")
                        #
                print("{0}_{1}.txt".format(graph,tp))
                print(source_num)
                f1.write(str(tot_avg_time_origin/source_num)+"\t"+str(tot_avg_time_contract/source_num)+"\n")
                sf.close()
    f1.close()

if __name__ == "__main__":
    main()
#
