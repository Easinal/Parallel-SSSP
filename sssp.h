#pragma once
#include "graph.h"
#include "hashbag.h"
#include "parlay/internal/get_time.h"
#include <queue>  
using namespace std;
using namespace parlay;

constexpr int NUM_SRC = 20;
constexpr int NUM_ROUND = 5;

constexpr size_t LOCAL_QUEUE_SIZE = 4096;
constexpr size_t DEG_THLD = 20;
constexpr size_t SSSP_SAMPLES = 1000;

bool change = false;


enum Algorithm { rho_stepping = 0, delta_stepping, bellman_ford };

class SSSP {
 protected:
  Graph &G;
  bool sparse;
  int sd_scale;
  size_t frontier_size;
  hashbag<NodeId> bag;
  sequence<EdgeTy> dist;
  sequence<NodeId> frontier;
  sequence<atomic<bool>> in_frontier;
  sequence<atomic<bool>> in_next_frontier;

  void add_to_bag(NodeId v) {
    if (!in_frontier[v] &&
        compare_and_swap(&in_next_frontier[v], false, true)) {
      bag.insert(v);
    }
  }
  void decompressLayered() {
    for(size_t i=G.layer-1;i>0;--i){
      if(G.layerOffset[i+1]-G.layerOffset[i]>1000){
        parallel_for(G.layerOffset[i], G.layerOffset[i+1], [&](size_t k){
          if(dist[G.edge[G.offset[k]].v]!=DIST_MAX){
            for (size_t j = G.offset[k]; j < G.offset[k + 1]; j++) {
              NodeId v = G.edge[j].v;
              EdgeTy w = G.edge[j].w;
              if (dist[k] >dist[v]+ w) {
                dist[k] = dist[v] + w;
              }
            }
          }
        });
      }else{
        for(size_t k = G.layerOffset[i]; k<G.layerOffset[i+1];++k){
          if(dist[G.edge[G.offset[k]].v]!=DIST_MAX){
            for (size_t j = G.offset[k]; j < G.offset[k + 1]; j++) {
              NodeId v = G.edge[j].v;
              EdgeTy w = G.edge[j].w;
              if (dist[k] >dist[v]+ w) {
                dist[k] = dist[v] + w;
              }
            }
          }
        }
      }
    }
  }
  size_t estimate_size() {
    static uint32_t seed = 10086;
    size_t hits = 0;
    for (size_t i = 0; i < SSSP_SAMPLES; i++) {
      NodeId u = hash32(seed) % G.n;
      if (in_frontier[u]) {
        hits++;
      }
      seed++;
    }
    return hits * G.n / SSSP_SAMPLES;
  }

  size_t sparse_relax() {
    // static uint32_t seed = 353442899;
    // size_t sum_deg = 0;
    // for (size_t i = 0; i < SSSP_SAMPLES; i++) {
    // NodeId u = frontier[hash32(seed) % size];
    // sum_deg += G.offset[u + 1] - G.offset[u];
    // seed++;
    //}
    // size_t avg_deg = sum_deg / SSSP_SAMPLES;
    // bool super_sparse = (avg_deg <= DEG_THLD);
    bool super_sparse = true; //true
    tb1.start();
    EdgeTy th = get_threshold();
    tb1.stop();
    tb2.start();
    parallel_for(0, frontier_size, [&](size_t i) {
      NodeId f = frontier[i];
      in_frontier[f] = false;
      if (dist[f] > th) {
        add_to_bag(f);
      } else {
        size_t _n = G.offset[f + 1] - G.offset[f];
        // if(change)cout<<f<<" with neighbor"<<_n<<endl;
        if (super_sparse && _n < LOCAL_QUEUE_SIZE) {
          NodeId local_queue[LOCAL_QUEUE_SIZE];
          size_t front = 0, rear = 0;
          local_queue[rear++] = f;
          while (front < rear && rear < LOCAL_QUEUE_SIZE) {
            NodeId u = local_queue[front++];
            size_t deg = G.offset[u + 1] - G.offset[u];
            if (deg >= LOCAL_QUEUE_SIZE || dist[u] > th) {
              add_to_bag(u);
              continue;
            }
            if (G.symmetrized) {
              EdgeTy temp_dis = dist[u];
              for (EdgeId es = G.offset[u]; es < G.offset[u + 1]; es++) {
                NodeId v = G.edge[es].v;
                EdgeTy w = G.edge[es].w;
                temp_dis = min(temp_dis, dist[v] + w);
              }
              write_min(&dist[u], temp_dis,
                        [](EdgeTy w1, EdgeTy w2) { return w1 < w2; });
            }
            for (EdgeId es = G.offset[u]; es < G.offset[u + 1]; es++) {
              NodeId v = G.edge[es].v;
              EdgeTy w = G.edge[es].w;
              if (write_min(&dist[v], dist[u] + w,
                            [](EdgeTy w1, EdgeTy w2) { return w1 < w2; })) {
                if (rear < LOCAL_QUEUE_SIZE) {
                  local_queue[rear++] = v;
                } else {
                  add_to_bag(v);
                }
              }
            }
          }
          for (size_t j = front; j < rear; j++) {
            add_to_bag(local_queue[j]);
          }
        } else {
          // if (G.symmetrized) {
          // size_t deg = G.offset[f + 1] - G.offset[f];
          // auto _dist = delayed_seq<EdgeTy>(deg, [&](size_t es) {
          // NodeId v = G.edge[G.offset[f] + es].v;
          // EdgeTy w = G.edge[G.offset[f] + es].w;
          // return dist[v] + w;
          //});
          // EdgeTy temp_dist = *min_element(_dist);
          // write_min(&dist[f], temp_dist,
          //[](EdgeTy w1, EdgeTy w2) { return w1 < w2; });
          //}
          blocked_for(
              G.offset[f], G.offset[f + 1], BLOCK_SIZE,
              [&](size_t, size_t _s, size_t _e) {
                if (G.symmetrized) {
                  EdgeTy temp_dist = dist[f];
                  for (EdgeId es = _s; es < _e; es++) {
                    NodeId v = G.edge[es].v;
                    EdgeTy w = G.edge[es].w;
                    temp_dist = min(temp_dist, dist[v] + w);
                  }
                  if (write_min(&dist[f], temp_dist,
                                [](EdgeTy w1, EdgeTy w2) { return w1 < w2; })) {
                    add_to_bag(f);
                  }
                }
                for (EdgeId es = _s; es < _e; es++) {
                  NodeId v = G.edge[es].v;
                  EdgeTy w = G.edge[es].w;
                  if (write_min(&dist[v], dist[f] + w,
                                [](EdgeTy w1, EdgeTy w2) { return w1 < w2; })) {
                    add_to_bag(v);
                  }
                }
              });
        }
      }
    });
    swap(in_frontier, in_next_frontier);
    tb2.stop();
    return bag.pack_into(make_slice(frontier));
  }

  size_t dense_relax() {
    while (estimate_size() >= G.n / sd_scale) {
      tb3.start();
      EdgeTy th = get_threshold();
      tb3.stop();
      tb4.start();
      parallel_for(0, G.n, [&](NodeId u) {
        if (in_frontier[u]) {
          in_frontier[u] = false;
          if (dist[u] > th) {
            in_next_frontier[u] = true;
          } else {
            // if (G.symmetrized) {
            // size_t deg = G.offset[u + 1] - G.offset[u];
            // auto _dist = delayed_seq<EdgeTy>(deg, [&](size_t es) {
            // NodeId v = G.edge[G.offset[u] + es].v;
            // EdgeTy w = G.edge[G.offset[u] + es].w;
            // return dist[v] + w;
            //});
            // EdgeTy temp_dist = *min_element(_dist);
            // write_min(&dist[u], temp_dist,
            //[](EdgeTy w1, EdgeTy w2) { return w1 < w2; });
            //}
            // if(change)cout<<u<<" with neighbor"<<G.offset[u+1]-G.offset[u]<<endl;
            blocked_for(G.offset[u], G.offset[u + 1], BLOCK_SIZE,
                        [&](size_t, size_t _s, size_t _e) {
                          if (G.symmetrized) {
                            EdgeTy temp_dist = dist[u];
                            for (size_t es = _s; es < _e; es++) {
                              NodeId v = G.edge[es].v;
                              EdgeTy w = G.edge[es].w;
                              temp_dist = min(temp_dist, dist[v] + w);
                            }
                            if (write_min(&dist[u], temp_dist,
                                          [](EdgeTy w1, EdgeTy w2) {
                                            return w1 < w2;
                                          })) {
                              if (!in_next_frontier[u]) {
                                in_next_frontier[u] = true;
                              }
                            }
                          }
                          for (size_t es = _s; es < _e; es++) {
                            NodeId v = G.edge[es].v;
                            EdgeTy w = G.edge[es].w;
                            if (write_min(&dist[v], dist[u] + w,
                                          [](EdgeTy w1, EdgeTy w2) {
                                            return w1 < w2;
                                          })) {
                              if (!in_frontier[v] && !in_next_frontier[v]) {
                                in_next_frontier[v] = true;
                              }
                            }
                          }
                        });
          }
        }
      });
      swap(in_frontier, in_next_frontier);
      tb4.stop();
    }
    return count(in_frontier, true);
  }

  void sparse2dense() {
    // parallel_for(0, frontier_size, [&](size_t i) {
    // NodeId u = frontier[i];
    // assert(in_frontier[u] == true);
    //  in_frontier[u] = true;
    //});
  }

  void dense2sparse() {
    auto identity = delayed_seq<NodeId>(G.n, [&](NodeId i) { return i; });
    pack_into_uninitialized(identity, in_frontier, frontier);
  }

  bool topDown(NodeId s){
    frontier_size=0;
    priority_queue<pair<EdgeTy, NodeId>, vector<pair<EdgeTy, NodeId>>,
                greater<pair<EdgeTy, NodeId>>>
    pq;
    priority_queue<pair<EdgeTy, NodeId>, vector<pair<EdgeTy, NodeId>>,
                greater<pair<EdgeTy, NodeId>>>
    pq2;
    pq2.push(make_pair(dist[s], s));
    while (!pq2.empty()) {
      pair<EdgeTy, NodeId> dist_and_node = pq2.top();
      pq2.pop();
      EdgeTy d = dist_and_node.first;
      NodeId u = dist_and_node.second;
      if (dist[u] < d) continue;
      if(in_frontier[u] == false){
        in_frontier[u]=true;
        frontier[frontier_size]=u;
        frontier_size++;
      }
      for (size_t j = G.offset[u]; j < G.offset[u + 1]; j++) {
        NodeId v = G.edge[j].v;
        EdgeTy w = G.edge[j].w;
        if (dist[v] > dist[u] + w) {
          dist[v] = dist[u] + w;
          if(G.residualFlag[v]==0){
            pq.push(make_pair(dist[v], v));
          }
          else{
            pq2.push(make_pair(dist[v], v));
          }
        }
      }
    }
    if(pq.empty()) {
      // if(G.layerOffset[1]>0.01*G.n) return false;
      // return true;
      return false;
    }
    parallel_for(0, frontier_size, [&](NodeId i) {
      in_frontier[frontier[i]]=false;
    });
    frontier_size = 0;
    while (!pq.empty()) {
      pair<EdgeTy, NodeId> dist_and_node = pq.top();
      pq.pop();
      NodeId u = dist_and_node.second;
      if(in_frontier[u] == true){
        continue;
      }
      in_frontier[u]=true;
      frontier[frontier_size]=u;
      frontier_size++;
    }
    return true;
  }

  bool backTrack(){
    priority_queue<pair<EdgeTy, NodeId>, vector<pair<EdgeTy, NodeId>>,
                  greater<pair<EdgeTy, NodeId>>> pq2;
    for(size_t i=0;i<frontier_size;++i)pq2.push(make_pair(dist[frontier[i]],frontier[i]));
    while (!pq2.empty()) {
      pair<EdgeTy, NodeId> dist_and_node = pq2.top();
      pq2.pop();
      EdgeTy d = dist_and_node.first;
      NodeId u = dist_and_node.second;
      if (dist[u] < d) continue;
      for (size_t j = G.offset[u]; j < G.offset[u + 1]; j++) {
        NodeId v = G.edge[j].v;
        EdgeTy w = G.edge[j].w;
        if (dist[v] > dist[u] + w) {
          dist[v] = dist[u] + w;
          pq2.push(make_pair(dist[v], v));
        }
      }
    }
    return false;
  }
  void reverseSearch(){
    change=true;
    init();
    sparse = false;
    // cout<<"Reverse "<<G.rm<<" "<<frontier_size<<" "<<count(in_frontier, true)<<endl;
    swap(G.edge, G.reverseEdge);
    swap(G.m,G.rm);
    swap(G.offset, G.reverseOffset);
    loop();
    // backTrack();
    change=false;
    swap(G.edge, G.reverseEdge);
    swap(G.m,G.rm);
    swap(G.offset, G.reverseOffset);
  }
  
  void loop(){
    int round = 0;
    while (frontier_size) {
      // internal::timer t;
      // printf("(Round %d, size: %zu, %d)\n", round, frontier_size, sparse);
      if (sparse) {
        frontier_size = sparse_relax();
      } else {
        frontier_size = dense_relax();
      }
      // printf("frontier_size: %ld \t relax: %f \t ",frontier_size, t.next_time());
      t_trans.start();
      bool next_sparse = (frontier_size < G.n / sd_scale) ? true : false;
      if (sparse && !next_sparse) {
        sparse2dense();
      } else if (!sparse && next_sparse) {
        dense2sparse();
      }
      round++;
      // if(round % 1000 == 0) {
      //   t.next("time");
      // }
      // printf("pack: %f\n", t.next_time());
      sparse = next_sparse;
      t_trans.stop();
    }
    printf("%d ", round);
  }
  

  function<void()> init;
  function<EdgeTy()> get_threshold;

 public:
  internal::timer tb1;
  internal::timer tb2;
  internal::timer tb3;
  internal::timer tb4;
  internal::timer t_decompress;
  internal::timer t_init;
  internal::timer t_trans;
  double break_time = 0;
  double decompress_time = 0;
  SSSP() = delete;
  SSSP(Graph &_G) : G(_G), bag(G.n) {
    dist = sequence<EdgeTy>::uninitialized(G.n);
    frontier = sequence<NodeId>::uninitialized(G.n);
    in_frontier = sequence<atomic<bool>>::uninitialized(G.n);
    in_next_frontier = sequence<atomic<bool>>::uninitialized(G.n);
  }
  sequence<EdgeTy> sssp(NodeId s) {
    if (!G.weighted) {
      fprintf(stderr, "Error: Input graph is unweighted\n");
      exit(EXIT_FAILURE);
    }
    t_init.start();
    init();
    parallel_for(0, G.n, [&](NodeId i) {
      dist[i] = numeric_limits<EdgeTy>::max() / 2;
      in_frontier[i] = in_next_frontier[i] = false;
    });
    if(G.contracted)s=G.sortedLayer[s];
    dist[s]=0;
    bool dcep=true;
    if(G.contracted && G.layer>1 && G.residualFlag[s]!=0){
      dcep = topDown(s);
    }else{
      frontier_size = 1;
      frontier[0] = s;
      in_frontier[s] = true;
    }
    sparse = false;
    t_init.stop();
    if(dcep && frontier_size)loop();
    t_decompress.reset();
    t_decompress.start();
    if(G.contracted){
      if(dcep){
        if(frontier_size==0){
          parallel_for(0, G.n, [&](NodeId i) {
            if(dist[i]!=numeric_limits<EdgeTy>::max() / 2 && G.reverseOffset[i]!=G.reverseOffset[i+1]){
              add_to_bag(i);
              in_frontier[i]=true;
            }
            in_next_frontier[i] = false;
          });
          frontier_size = bag.pack_into(make_slice(frontier));
        }
        if(frontier_size >  2000)decompressLayered();
        else reverseSearch();
      }else{
        reverseSearch();
      }
    }
    t_decompress.stop();
    decompress_time+=t_decompress.total_time();
    // printf("%f %f %f %f %f %f %f -> %f \n", tb1.total_time(), tb2.total_time(), tb3.total_time(), tb4.total_time(), t_decompress.total_time(), t_init.total_time(), t_trans.total_time(),
    //                                         tb1.total_time()+tb2.total_time()+tb3.total_time()+tb4.total_time()+t_decompress.total_time()+t_init.total_time()+t_trans.total_time());
    // printf("sparse get threshold: %f\n", tb1.total_time());
    // printf("sparse get frontier: %f\n", tb2.total_time());
    // printf("dense get threshold: %f\n", tb3.total_time());
    // printf("dense get frontier: %f\n", tb4.total_time());
    // printf("decompress: %f\n", t_decompress.total_time());
    return dist;
  }

  void set_sd_scale(int x) { sd_scale = x; }
};  

class Rho_Stepping : public SSSP {
  size_t rho;
  size_t radius_range;
  double x;
  uint32_t seed;

 public:
  Rho_Stepping(Graph &_G, size_t _rho = 1 << 20, size_t _radius_range=0, double _x=1) 
  : SSSP(_G), rho(_rho), radius_range(_radius_range), x(_x){
    seed = 0;
    init = []() {};
    get_threshold = [&]() {
      //cerr<<"rho = "<<rho<<endl;
      if (frontier_size <= rho) {
        if (sparse) {
          return DIST_MAX;
          if(G.contracted){
            auto _dist = delayed_seq<EdgeTy>(
                frontier_size, [&](size_t i) { return dist[frontier[i]]+G.radius[frontier[i]][radius_range]; });
            return *max_element(_dist);
          }
          auto _dist = delayed_seq<EdgeTy>(
              frontier_size, [&](size_t i) { return dist[frontier[i]]; });
          return *max_element(_dist);
        } else {
          return DIST_MAX;
        }
      }
      EdgeTy sample_dist[SSSP_SAMPLES + 1];
      for (size_t i = 0; i <= SSSP_SAMPLES; i++) {
        if (sparse) {
          NodeId v = frontier[hash32(seed + i) % frontier_size];
          sample_dist[i] = dist[v];
        } else {
          NodeId v = hash32(seed + i) % G.n;
          if (in_frontier[v]) {
            sample_dist[i] = dist[v];
          } else {
            sample_dist[i] = DIST_MAX;
          }
        }
      }
      seed += SSSP_SAMPLES + 1;
      size_t id = rho * SSSP_SAMPLES / frontier_size ;
      sort(sample_dist, sample_dist + SSSP_SAMPLES + 1);
      return sample_dist[id];
    };
  }
};

class Delta_Stepping : public SSSP {
  EdgeTy delta;
  size_t radius_range;
  double x;
  EdgeTy thres;

 public:
  Delta_Stepping(Graph &_G, EdgeTy _delta = 1 << 15, size_t _radius_range = 5, double _x = 1)
      : SSSP(_G), delta(_delta), radius_range(_radius_range), x(_x) {
        // cout<<x<<endl;
    init = [&]() { thres = 0; };
    get_threshold = [&]() {
      /*
      if(G.contracted){
        auto _dist = delayed_seq<EdgeTy>(
            frontier_size, [&](size_t i) { return dist[frontier[i]]+G.radius[frontier[i]][radius_range]; });
        auto _min_dist = *min_element(_dist);
        auto _max_dist = *max_element(_dist);
        // thres = _min_dist + (_max_dist - _min_dist) * x;
        double k=thres/204007562.0;
        k=4*k*k-4*k+1;
        thres = _min_dist + (_max_dist - _min_dist) * (x+(1-x)*k);
        cout<<thres<<"\t";
        return thres;
      }
      */
      thres += delta;
      // if(change)thres += 10*delta;
      // cout<<thres<<"\t";
      return thres;
    };
  }
};

class Bellman_Ford : public SSSP {
 public:
  Bellman_Ford(Graph &_G) : SSSP(_G) {
    init = []() {};
    get_threshold = []() { return DIST_MAX; };
  }
};
