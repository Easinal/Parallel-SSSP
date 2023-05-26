#include "sssp.h"

#include <fstream>
#include <functional>
#include <numeric>
#include <set>

#include "dijkstra.h"

using namespace std;
<<<<<<< HEAD
using namespace pbbs;
sequence<NodeId> sortedLayer;
sequence<size_t> layerOffset;
size_t layer;

void SSSP::degree_sampling(size_t sz) {
  static uint32_t seed = 353442899;
  for (size_t i = 0; i < SSSP_SAMPLES; i++) {
    NodeId u = que[cur][hash32(seed) % sz];
    sample_deg[i] = G.offset[u + 1] - G.offset[u];
    seed++;
  }
}

void SSSP::sparse_sampling(size_t sz) {
  static uint32_t seed = 998244353;
  for (size_t i = 0; i < SSSP_SAMPLES; i++) {
    NodeId u = que[cur][hash32(seed) % sz];
    sample_dist[i] = info[u].dist;
    seed++;
  }
  sort(sample_dist, sample_dist + SSSP_SAMPLES);
}

size_t SSSP::dense_sampling() {
  static uint32_t seed = 10086;
  int num_sample = 0;
  for (size_t i = 0; i < SSSP_SAMPLES;) {
    num_sample++;
    NodeId u = hash32(seed) % G.n;
    if (info[u].fl & in_que) {
      sample_dist[i] = info[u].dist;
      i++;
    }
    seed++;
  }
  sort(sample_dist, sample_dist + SSSP_SAMPLES);
  return 1.0 * SSSP_SAMPLES / num_sample * G.n;
}

void SSSP::decompressLayered() {
  for(size_t i=layer;i>0;--i){
    parallel_for(layerOffset[i], layerOffset[i+1], [&](size_t k){
      NodeId u = sortedLayer[k];
      for (size_t j = Gc.offset[u]; j < Gc.offset[u + 1]; j++) {
        NodeId v = Gc.edge[j].v;
        EdgeTy w = Gc.edge[j].w;
        if (info[u].dist >info[v].dist+ w) {
          info[u].dist = info[v].dist + w;
        }
      }
    });
  }
}

void SSSP::decompress() {
  parallel_for(0, Gc.n, [&](size_t u){
    if(G.residual[u]){
      assert(Gc.offset[u+1]-Gc.offset[u]<=2);
      for (size_t j = Gc.offset[u]; j < Gc.offset[u + 1]; j++) {
        NodeId v = Gc.edge[j].v;
        EdgeTy w = Gc.edge[j].w;
        if (info[u].dist >info[v].dist+ w) {
          info[u].dist = info[v].dist + w;
        }
      }
    }
  });
}

void SSSP::relax(size_t sz) {
  if (sparse) {
    size_t qsize[doubling], cnt[doubling];
    qsize[0] = cnt[0] = cnt[1] = 0;
    qsize[1] = MIN_QUEUE;
    int db_len = 2;
    for (size_t s = MIN_QUEUE * 2; s <= max_queue; s *= 2) {
      qsize[db_len] = s;
      cnt[db_len] = 0;
      db_len++;
    }
    int pt = 1;
    auto add = [&](NodeId u) {
      if ((info[u].fl & to_add) ||
          !atomic_compare_and_swap(&info[u].fl, info[u].fl,
                                   info[u].fl | to_add)) {
        return;
      }
      int t_pt = pt;
      size_t pos =
          hash32(u) % (qsize[t_pt] - qsize[t_pt - 1]) + qsize[t_pt - 1];
      while (que[nxt][pos] != UINT_MAX ||
             !atomic_compare_and_swap(&que[nxt][pos], UINT_MAX, u)) {
        pos++;
        if (pos == qsize[t_pt]) {
          pos = qsize[t_pt - 1];
        }
      }
      // The queue should be half occupied when we have EXP_SAMPLES
      size_t len = qsize[t_pt] - qsize[t_pt - 1];
      size_t rate = len / (2 * EXP_SAMPLES);
      if (pos % rate == 0) {
        int ret = fetch_and_add(&cnt[t_pt], 1);
        if (ret + 1 == EXP_SAMPLES && t_pt + 1 < db_len) {
          atomic_compare_and_swap(&pt, t_pt, t_pt + 1);
        }
      }
    };

    auto relax_neighbors = [&](NodeId u, EdgeId _s, EdgeId _e) {
      _s += G.offset[u];
      _e += G.offset[u];
      if (G.symmetrized) {
        EdgeTy temp_dis = info[u].dist;
        for (EdgeId es = _s; es < _e; es++) {
          NodeId v = G.edge[es].v;
          EdgeTy w = G.edge[es].w;
          temp_dis = min(temp_dis, info[v].dist + w);
        }
        if (write_min(&info[u].dist, temp_dis,
                      [](EdgeTy w1, EdgeTy w2) { return w1 < w2; })) {
          add(u);
        }
      }
      for (EdgeId es = _s; es < _e; es++) {
        NodeId v = G.edge[es].v;
        EdgeTy w = G.edge[es].w;
        if (write_min(&info[v].dist, info[u].dist + w,
                      [](EdgeTy w1, EdgeTy w2) { return w1 < w2; })) {
          add(v);
        }
      }
    };
    degree_sampling(sz);
    size_t sum_deg = 0;
    for (size_t i = 0; i < SSSP_SAMPLES; i++) {
      sum_deg += sample_deg[i];
    }
    size_t avg_deg = sum_deg / SSSP_SAMPLES;
    bool super_sparse = false;//(avg_deg <= DEG_THLD);
    EdgeTy th;
    if (algo == rho_stepping) {
      sparse_sampling(sz);
      int rate = min(SSSP_SAMPLES - 1, SSSP_SAMPLES * param / sz);
      th = sample_dist[rate];
    } else if (algo == delta_stepping) {
      th = delta;
      delta += param;
    } else {
      th = UINT_MAX;
    }
    parallel_for(0, sz, [&](size_t i) {
      NodeId f = que[cur][i];
      que[cur][i] = UINT_MAX;
      if (info[f].dist > th) {
        add(f);
=======
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
>>>>>>> upstream/parlaylib
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
<<<<<<< HEAD
  swap(cur, nxt);
  return nxt_sz;
}

void SSSP::reset_timer() { t_all.reset(); }

size_t SSSP::dijkstraResidual(int s) {
  info[s].dist = 0;
  priority_queue<pair<EdgeTy, NodeId>, vector<pair<EdgeTy, NodeId>>,
                 greater<pair<EdgeTy, NodeId>>>
      pq;
  size_t sz = 0;
  set<NodeId> st;
  pq.push(make_pair(info[s].dist, s));
  while (!pq.empty()) {
    pair<EdgeTy, NodeId> dist_and_node = pq.top();
    pq.pop();
    EdgeTy d = dist_and_node.first;
    NodeId u = dist_and_node.second;
    if (info[u].dist < d) continue;
    if(Gr.offset[u+1]==Gr.offset[u]){
      if(st.find(u)==st.end()){
        info[u].dist = d;
        que[cur][sz] = u;
        st.insert(u);
        sz++;
      }else if(d<info[u].dist){
        info[u].dist = d;
      }
    }
    for (size_t j = Gr.offset[u]; j < Gr.offset[u + 1]; j++) {
      NodeId v = Gr.edge[j].v;
      EdgeTy w = Gr.edge[j].w;
      if (info[v].dist > info[u].dist + w) {
        info[v].dist = info[u].dist + w;
        pq.push(make_pair(info[v].dist, v));
      }
    }
  }
  return sz;
}


size_t SSSP::bfs(int s) {
  info[s].dist = 0;
  size_t sz = Gr.offset[s+1]-Gr.offset[s];
  if(sz==0){
    return 0;
  }
  for (size_t j = Gr.offset[s]; j < Gr.offset[s + 1]; j++) {
    NodeId prev_v = s;
    NodeId v = Gr.edge[j].v;
    EdgeTy w = Gr.edge[j].w;
    while(G.residual[v]){
      info[v].dist = w;
      if(Gr.offset[v+1]-Gr.offset[v]<2)break;
      size_t k = Gr.offset[v];
      NodeId u = Gr.edge[k].v;
      if(u==prev_v){
        k++;
        u=Gr.edge[k].v;
      }
      prev_v = v;
      w+=Gr.edge[k].w;
      v=u;
    }
  }
  set<NodeId> st;
  sz = 0;
  for (size_t j = Gc.offset[s]; j < Gc.offset[s + 1]; j++) {
    NodeId v = Gc.edge[j].v;
    if(G.residual[v])continue;
    EdgeTy w = Gc.edge[j].w;
    if(st.find(v)==st.end()){
      st.insert(v);
      info[v].dist = w;
      que[cur][sz] = v;
      sz++;
    }else if(w<info[v].dist){
      info[v].dist = w;
    }
  }
  return sz;
}

void SSSP::sssp(int s, EdgeTy *_dist) {
  if (!G.weighted) {
    fprintf(stderr, "Error: Input graph is unweighted\n");
    exit(EXIT_FAILURE);
  }
  t_all.start();
  cur = 0, nxt = 1;
  if (algo == delta_stepping) {
    delta = param;
  }
  parallel_for(0, que[0].size(), [&](size_t i) { que[0][i] = UINT_MAX; });
  parallel_for(0, que[1].size(), [&](size_t i) { que[1][i] = UINT_MAX; });
  parallel_for(0, info.size(),
               [&](size_t i) { info[i] = Information(INT_MAX / 2, 0); });

  size_t sz = 0;

  if(contracted && G.residual[s]){
    sz = dijkstraResidual(s);
  }else{
    sz=1;
    que[cur][0] = s;
    info[s].dist = 0;
  }
  sparse = true;

  while (sz) {
    relax(sz);
    sz = pack();
    //cout<<sz<<endl;
    if (sz >= G.n / sd_scale) {
      sparse = false;
    } else {
      sparse = true;
    };
  }
  
  if(contracted){
    decompressLayered();
  }
  t_all.stop();
  parallel_for(0, G.n, [&](size_t i) { _dist[i] = info[i].dist; });
=======
>>>>>>> upstream/parlaylib
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
        "Usage: %s [-i input_file] [-f input_file2] [-d input_file3] [-r input_residual] [-p parameter] [-w] [-s] [-v] [-c] [-a] "
        "algorithm]\n"
        "Options:\n"
        "\t-i,\tinput file path\n"
        "\t-p,\tparameter(e.g. delta, rho)\n"
        "\t-w,\tweighted input graph\n"
        "\t-s,\tsymmetrized input graph\n"
        "\t-v,\tverify result\n"
        "\t-c,\tcontracted graph\n"
        "\t-a,\talgorithm: [rho-stepping] [delta-stepping] [bellman-ford]\n",
        argv[0]);
    exit(EXIT_FAILURE);
  }
  char c;
  bool weighted = false;
  bool symmetrized = false;
  bool verify = false;
<<<<<<< HEAD
  bool contract = false;
  size_t param = 1 << 21;
  Algorithm algo = rho_stepping;
  while ((c = getopt(argc, argv, "i:f:d:r:p:a:cwsv")) != -1) {
=======
  size_t param = ULLONG_MAX;
  int algo = rho_stepping;
  char const *FILEPATH = nullptr;
  while ((c = getopt(argc, argv, "i:p:a:wsv")) != -1) {
>>>>>>> upstream/parlaylib
    switch (c) {
      case 'i':
        FILEPATH = optarg;
        break;
      case 'f':
        FILEPATH2 = optarg;
        break;
      case 'd':
        FILEPATH3 = optarg;
        break;
      case 'r':
        FILEPATH4 = optarg;
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
      case 'c':
        contract = true;
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
      default:
        fprintf(stderr, "Error: Unknown option %c\n", optopt);
        exit(EXIT_FAILURE);
    }
  }
  Graph G(weighted, symmetrized);
<<<<<<< HEAD
=======

  printf("Reading graph...\n");
>>>>>>> upstream/parlaylib
  G.read_graph(FILEPATH);
  if (!weighted) {
    printf("Generating edge weights...\n");
    G.generate_weight();
  }

<<<<<<< HEAD
  SSSP solver(G, algo, param);
  int sd_scale = G.m / G.n;
  solver.set_sd_scale(sd_scale);

=======
>>>>>>> upstream/parlaylib
  fprintf(stdout,
          "Running on %s: |V|=%zu, |E|=%zu, param=%zu, num_src=%d, "
          "num_round=%d\n",
          FILEPATH, G.n, G.m, param, NUM_SRC, NUM_ROUND);

<<<<<<< HEAD

  if(contract){
    Graph G2(weighted, symmetrized, contract);
    printf("Info: Reading graph\n");
    G2.read_graph(FILEPATH2);
    if (!weighted) {
      printf("Info: Generating edge weights\n");
      G2.generate_weight();
    }
    Graph G3(weighted, false, contract);
    printf("Info: Reading graph\n");
    G3.read_graph(FILEPATH3);
    if (!weighted) {
      printf("Info: Generating edge weights\n");
      G3.generate_weight();
    }
    Graph G4(weighted, false, contract);
    printf("Info: Reading graph\n");
    G4.read_graph(FILEPATH4);
    if (!weighted) {
      printf("Info: Generating edge weights\n");
      G4.generate_weight();
    }

    sequence<std::pair<size_t, NodeId>> layerMap(G3.n);
    sortedLayer=sequence<NodeId>(G3.n);
    parallel_for(0, G3.n, [&](size_t i){
      layerMap[i] = make_pair(G3.residual[i],i);
    });
    sort(layerMap.begin(), layerMap.end());
    layer = layerMap[G3.n-1].first;
    layerOffset=sequence<size_t>(layer+2);
    layerOffset[1]=0;
    layerOffset[layer+1]=G3.n;
    sortedLayer[0]=layerMap[0].second;
    parallel_for(1, G3.n, [&](size_t i){
      sortedLayer[i]=layerMap[i].second;
      if(layerMap[i].first!=layerMap[i-1].first){
        layerOffset[layerMap[i].first]=i;
      }
    });

    int sd_scale2 = G2.m / G2.n;
    SSSP solver2(G2, algo, param, G3, G4);
    solver2.contracted=true;
    solver2.set_sd_scale(sd_scale2);
    EdgeTy *my_dist2 = new EdgeTy[G2.n];
    for (int v = 0; v < NUM_SRC; v++) {
      int s = hash32(v) % G.n;
      printf("source: %-10d\n", s);
      vector<double> sssp_time;
      vector<double> sssp_time2;
      // first time warmup
      printf("origin\n");
      solver.reset_timer();
      solver.sssp(s, my_dist);
      printf("warmup round (not counted): %f\n", solver.t_all.get_total());
      for (int i = 0; i < NUM_ROUND; i++) {
        solver.reset_timer();
        solver.sssp(s, my_dist);
        sssp_time.push_back(solver.t_all.get_total());

        printf("round %d: %f\n", i + 1, solver.t_all.get_total());
      }
      printf("contract\n");
      solver2.reset_timer();
      solver2.sssp(s, my_dist2);
      printf("warmup round (not counted): %f\n", solver2.t_all.get_total());

      for (int i = 0; i < NUM_ROUND; i++) {
        solver2.reset_timer();
        solver2.sssp(s, my_dist2);
        sssp_time2.push_back(solver2.t_all.get_total());
        printf("round %d: %f\n", i + 1, solver2.t_all.get_total());
      }
      sort(begin(sssp_time), end(sssp_time));
      sort(begin(sssp_time2), end(sssp_time2));
      printf("median running time: %f\t%f\n", sssp_time[(sssp_time.size() - 1) / 2], sssp_time2[(sssp_time2.size() - 1) / 2]);
      printf("average running time: %f\t%f\n",
            accumulate(begin(sssp_time), end(sssp_time), 0.0) / NUM_ROUND,
            accumulate(begin(sssp_time2), end(sssp_time2), 0.0) / NUM_ROUND);
      if (verify) {
        printf("Info: Running verifier\n");
        verifier(s, G, my_dist, my_dist2, contract);
      }
    }
  }else{
    for (int v = 0; v < NUM_SRC; v++) {
      int s = hash32(v) % G.n;
      printf("source: %-10d\n", s);
      vector<double> sssp_time;
      vector<double> sssp_time2;
      // first time warmup
      solver.reset_timer();
      solver.sssp(s, my_dist);
      printf("warmup round (not counted): %f\n", solver.t_all.get_total());

      for (int i = 0; i < NUM_ROUND; i++) {
        solver.reset_timer();
        solver.sssp(s, my_dist);
        sssp_time.push_back(solver.t_all.get_total());

        printf("round %d: %f\n", i + 1, solver.t_all.get_total());
      }
      sort(begin(sssp_time), end(sssp_time));
      sort(begin(sssp_time2), end(sssp_time2));
      printf("median running time: %f\n", sssp_time[(sssp_time.size() - 1) / 2]);
      printf("average running time: %f\n",
            accumulate(begin(sssp_time), end(sssp_time), 0.0) / NUM_ROUND);
      if (verify) {
        printf("Info: Running verifier\n");
        verifier(s, G, my_dist);
      }
    }
=======
  int sd_scale = G.m / G.n;
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
>>>>>>> upstream/parlaylib
  }
  return 0;
}
