import sys
import time
import os
import random

try:
    import matplotlib.pyplot as plt
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False

sys.path.append(os.path.abspath("../build"))
import vectordb as vector_db

random.seed(42)

DIM = 128
NUM_POINTS = 200000
NUM_QUERIES = 200 # Kept moderate to ensure Linear doesn't take forever
store = vector_db.VectorStore()

print(f"Generating {NUM_POINTS} vectors...")
for i in range(NUM_POINTS):
    vec = [random.random() for _ in range(DIM)]
    store.save(vector_db.VectorPoint(i, vec))

print("Building Index...")
store.rebuildIndex()

query_batch = [vector_db.VectorPoint(-1, [random.random() for _ in range(DIM)]) for _ in range(NUM_QUERIES)]

print("\nCalculating Ground Truth (findNearestLinear)...")
start_gt = time.time()
ground_truth_dists = []
for q in query_batch:
    res = store.findNearestLinear(q)
    ground_truth_dists.append(q.distanceto(res))
linear_total_ms = (time.time() - start_gt) * 1000

limits = [ 100,  500, 1000, 5000,10000,20000,-1]

# Initialize lists for plotting
latencies = []
errors = []

print(f"\n{'Limit':<6} | {'Algo Gain':<10} | {'Para Gain':<10} | {'Total Gain':<10} | {'Error %':<8}")
print("-" * 65)

for limit in limits:
    # 1. Single Threaded KD
    t0 = time.time()
    for q in query_batch:
        _ = store.findNearestkd(q, max_nodes=limit)
    single_time = (time.time() - t0) * 1000

    # 2. Parallel KD
    t1 = time.time()
    parallel_res = store.findNearestParallel(query_batch, max_nodes=limit)
    parallel_time = (time.time() - t1) * 1000
    
    # 3. Accuracy Calculation
    total_err = 0
    for i in range(NUM_QUERIES):
        approx_dist = query_batch[i].distanceto(parallel_res[i])
        true_dist = ground_truth_dists[i]
        total_err += (approx_dist - true_dist) / true_dist if true_dist > 0 else 0
    
    avg_error_pct = (total_err / NUM_QUERIES) * 100
    
    # Speedup Calculations
    algo_speedup = linear_total_ms / single_time
    hardware_speedup = single_time / parallel_time
    total_speedup = linear_total_ms / parallel_time

    # Populate lists for plotting (using average latency per query in microseconds)
    avg_parallel_latency_us = (parallel_time * 1000) / NUM_QUERIES
    latencies.append(avg_parallel_latency_us)
    errors.append(avg_error_pct)

    print(f"{limit:<6} | {algo_speedup:<10.1f}x | {hardware_speedup:<10.1f}x | {total_speedup:<10.1f}x | {avg_error_pct:<8.4f}%")

if HAS_PLOT:
    plt.figure(figsize=(10, 6))
    plt.plot(latencies, errors, marker='o', color='r', linewidth=2)
    plt.title('ANN Trade-off: Latency vs Error Rate (Parallel Search)')
    plt.xlabel('Average Latency per Query (microseconds)')
    plt.ylabel('Mean Relative Error (%)')
    plt.grid(True, which="both", ls="-")
    plt.savefig('../assets/pareto_frontier.png')
    print("\n[SUCCESS] Pareto Frontier saved to pareto_frontier.png")