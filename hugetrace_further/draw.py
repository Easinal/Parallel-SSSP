import re
import matplotlib.pyplot as plt
st1 = ["delta","rho"]
st2=["8192",  "16384", "32768", "65536", "131072", "262144", "524288", "1048576", "2097152", "4194304", "8388608", "16777216", "33554432"]
#"16384", 
def main():
    with open("hugetrace_delta_2097152.txt", "r") as origin:
        tot_round = 0
        fl=origin.readlines()
        rounds = []
        size = []
        label = 0
        for line in fl:
            sr=line.split()
            if (len(sr) > 0):
                rounds = []
                size = []
                for wd in sr:
                    if (wd[-1] == ":" and wd[0] != 's'):
                        rounds.append(int(wd[0:-1]))
                    elif (wd[-1] == ',' and wd[0] != 's'):
                        size.append(int(wd[0:-1]))
                if (label % 2 == 0):
                    plt.plot(rounds,size,label = "origin")
                else:
                    plt.plot(rounds,size,label = "contract")
                label = label + 1
                print(label)
        plt.xlabel('round')
        plt.ylabel('size of frontier')
        plt.title('hugetrace rho delta stepping 2097152')
        plt.legend()
        plt.savefig('hugetrace_delta.pdf')
    origin.close()

if __name__ == "__main__":
    main()
#
