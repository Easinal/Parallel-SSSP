import re
st1 = ["Germany","USA"]
st2 = ["bf","delta","rho"]

def main():
    with open("average.txt", "w") as f1:
        with open("median.txt", "w") as f2:
            for graph in st1:
                for tp in st2:
                    f1.write("{0}_{1}\n".format(graph, tp))
                    f2.write("{0}_{1}\n".format(graph, tp))
                    f1.write("origin\tcontracted\n")
                    f2.write("origin\tcontracted\n")
                    with open("{0}_{1}.txt".format(graph, tp), "r") as sf:
                        fl = sf.readlines()
                        for line in fl:
                            sr = line.split()
                            if(sr[0]=="average"):
                                f1.write(sr[3]+"\t"+sr[4]+"\n")
                            if(sr[0]=="median"):
                                f2.write(sr[3]+"\t"+sr[4]+"\n")
                        #
                    sf.close()
        f2.close()
    f1.close()

if __name__ == "__main__":
    main()
#