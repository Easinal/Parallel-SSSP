#include "sssp.h"

#include <fstream>
#include <functional>
#include <numeric>
#include <set>

#include "dijkstra.h"

using namespace std;
using namespace parlay;

template <class Algo>
void run(Algo &algo, const Graph &G, bool verify) {
  for (int v = 0; v < NUM_SRC; v++) {
    NodeId s = hash32(v) % G.n;
    printf("source %d: %-10d\n", v, s);
    double total_time = 0;
    for (int i = 0; i <= NUM_ROUND; i++) {
      internal::timer t;
      algo.sssp(s);
      t.stop();
      if (i == 0) {
        printf("Warmup Round: %f\n", t.total_time());
      } else {
        printf("Round %d: %f\n", i, t.total_time());
        total_time += t.total_time();
      }
    }
    double average_time = total_time / NUM_ROUND;
    printf("Average time: %f\n", average_time);

    ofstream ofs("sssp.tsv", ios_base::app);
    ofs << average_time << '\t';
    ofs.close();

    if (verify) {
      printf("Running verifier...\n");
      internal::timer t;
      auto dist = algo.sssp(s);
      t.stop();
      printf("Our running time: %f\n", t.total_time());
      verifier(s, G, dist);
    }
    printf("\n");
  }
}

template <class Algo>
void run2(Algo &algo, const Graph &G, Algo &algo2, bool verify) {
  for (int v = 0; v < NUM_SRC; v++) {
    NodeId s = hash32(v) % G.n;
    printf("source %d: %-10d\n", v, s);
    double total_time = 0;
    for (int i = 0; i <= NUM_ROUND; i++) {
      internal::timer t;
      algo.sssp(s);
      t.stop();
      if (i == 0) {
        printf("Warmup Round: %f\n", t.total_time());
      } else {
        printf("Round %d: %f\n", i, t.total_time());
        total_time += t.total_time();
      }
    }
    double average_time = total_time / NUM_ROUND;
    double total_time2 = 0;
    for (int i = 0; i <= NUM_ROUND; i++) {
      internal::timer t;
      algo2.sssp(s);
      t.stop();
      if (i == 0) {
        printf("Warmup Round: %f\n", t.total_time());
      } else {
        printf("Round %d: %f\n", i, t.total_time());
        total_time2 += t.total_time();
      }
    }
    double average_time2 = total_time2 / NUM_ROUND;
    printf("Average time: %f\t%f\n", average_time, average_time2);

    ofstream ofs("sssp.tsv", ios_base::app);
    ofs << average_time << '\t' << average_time2 << '\t';
    ofs.close();


    if (verify) {
      printf("Running verifier...\n");
      internal::timer t;
      auto dist = algo.sssp(s);
      t.stop();
      internal::timer t2;
      auto dist2 = algo2.sssp(s);
      t2.stop();
      printf("Our running time: %f\t%f\n", t.total_time(), t2.total_time());
      verifier2(s, G, dist, dist2);
    }
    printf("\n");
  }
}
// ./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -c -a rho-stepping
// ./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -c -a delta-stepping
// ./sssp -i /data0/zwan018/origin/Germany_sym_wgh.adj -f /data0/zwan018/contract/Germany_sym_wgh.adj -p 2097152 -w -v -c -a bellman-ford
// ./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a rho-stepping
// ./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a delta-stepping
// ./sssp -i /data0/zwan018/origin/RoadUSA_sym_wgh.adj -f /data0/zwan018/contract/RoadUSA_sym_wgh.adj -p 8388608 -w -v -c -a bellman-ford
// ./sssp -i /data0/zwan018/origin/twitter.adj -f /data0/zwan018/contract/twitter.adj -p 200000 -w -v -c -a rho-stepping


int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(
        stderr,
        "Usage: %s [-i input_file] [-c contracted_file] [-p parameter] [-w] [-s] [-v] [-c] [-a] "
        "algorithm]\n"
        "Options:\n"
        "\t-i,\tinput file path\n"
        "\t-c,\tcontracted graph\n"
        "\t-p,\tparameter(e.g. delta, rho)\n"
        "\t-w,\tweighted input graph\n"
        "\t-s,\tsymmetrized input graph\n"
        "\t-v,\tverify result\n"
        "\t-a,\talgorithm: [rho-stepping] [delta-stepping] [bellman-ford]\n"
        "\t-r,\tradius of i*100 steps\n"
        "\t-x,\tratio of radius stepping\n",
        argv[0]);
    exit(EXIT_FAILURE);
  }
  char c;
  bool weighted = false;
  bool symmetrized = false;
  bool verify = false;
  bool contract = false;
  size_t param = ULLONG_MAX;
  int algo = rho_stepping;
  int radius_range = 0;
  int x = 1;
  char const *FILEPATH = nullptr;
  char const *FILEPATH2 = nullptr;
  while ((c = getopt(argc, argv, "i:c:p:a:r:x:wsv")) != -1) {
    switch (c) {
      case 'i':
        FILEPATH = optarg;
        break;
      case 'c':
        FILEPATH2 = optarg;
        contract = true;
        break;
      case 'p':
        param = atol(optarg);
        break;
      case 'a':
        if (!strcmp(optarg, "rho-stepping")) {
          algo = rho_stepping;
        } else if (!strcmp(optarg, "delta-stepping")) {
          algo = delta_stepping;
        } else if (!strcmp(optarg, "bellman-ford")) {
          algo = bellman_ford;
        } else {
          fprintf(stderr, "Error: Unknown algorithm %s\n", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      case 'w':
        weighted = true;
        break;
      case 's':
        symmetrized = true;
        break;
      case 'v':
        verify = true;
        break;
      case 'r':
        radius_range = atol(optarg);
        break;
      case 'x':
        x = atol(optarg);
        if (x<=0){
          fprintf(stderr, "Error: x must be positive\n");
          exit(EXIT_FAILURE);
        }
        break;
      default:
        fprintf(stderr, "Error: Unknown option %c\n", optopt);
        exit(EXIT_FAILURE);
    }
  }
  Graph G(weighted, symmetrized);
  Graph G2(weighted, symmetrized, contract);

  printf("Reading graph...\n");
  G.read_graph(FILEPATH);
  if (!weighted) {
    printf("Generating edge weights...\n");
    G.generate_weight();
  }

  if(contract){
    printf("Reading contracted graph...\n");
    G2.read_graph(FILEPATH2);
    if (!weighted) {
      printf("Impossible!\n");
      return 0;
    }
  }


  fprintf(stdout,
          "Running on %s: |V|=%zu, |E|=%zu, param=%zu, num_src=%d, "
          "num_round=%d\n",
          FILEPATH, G.n, G.m, param, NUM_SRC, NUM_ROUND);

  int sd_scale = 1;
  int sd_scale2 = 1;
  if(contract){
    if (algo == rho_stepping) {
      Rho_Stepping solver(G, param, radius_range,x);
      solver.set_sd_scale(sd_scale);
      Rho_Stepping solver2(G2, param, radius_range,x);
      solver2.set_sd_scale(sd_scale2);
      run2(solver, G, solver2, verify);
    } else if (algo == delta_stepping) {
      Delta_Stepping solver(G, param, radius_range,x);
      solver.set_sd_scale(sd_scale);
      Delta_Stepping solver2(G2, param, radius_range,x);
      solver2.set_sd_scale(sd_scale2);
      run2(solver, G, solver2, verify);
    } else if (algo == bellman_ford) {
      Bellman_Ford solver(G);
      solver.set_sd_scale(sd_scale);
      Bellman_Ford solver2(G2);
      solver2.set_sd_scale(sd_scale2);
      run2(solver, G, solver2, verify);
    }
    return 0;
  }
  if (algo == rho_stepping) {
    Rho_Stepping solver(G);
    solver.set_sd_scale(sd_scale);
    run(solver, G, verify);
  } else if (algo == delta_stepping) {
    Delta_Stepping solver(G);
    solver.set_sd_scale(sd_scale);
    run(solver, G, verify);
  } else if (algo == bellman_ford) {
    Bellman_Ford solver(G);
    solver.set_sd_scale(sd_scale);
    run(solver, G, verify);
  }
  return 0;
}
