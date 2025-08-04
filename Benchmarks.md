# Server Performance Benchmark Analysis

## Test Configuration
Command used for benchmarking:
```bash
wrk -t4 -c400 -d30s http://localhost:8003
```
- `-t4`: 4 threads
- `-c400`: 400 concurrent connections
- `-d30s`: 30 second test duration

## Benchmark Results Comparison

| Metric               | Hamza Branch             | Feature/cgi Branch       | Improvement |
|----------------------|--------------------------|--------------------------|-------------|
| **Requests/sec**     | 92.29                   | 1762.05                 | **19.1x**   |
| **Avg Latency**      | 976.47ms ± 575.35ms     | 225.99ms ± 25.61ms      | **4.3x**    |
| **Total Requests**   | 2,773 (with 161 fails)  | 52,929 (all successful)  | **19.1x**   |
| **Throughput**       | 98.32KB/s               | 6.03MB/s                | **63x**     |
| **Timeout Errors**   | 2,445                   | 0                       | **Fixed**   |
| **Error Responses**  | 161                     | 0                       | **Fixed**   |

## Key Improvements
1. **19x increase** in request handling capacity
2. **4x reduction** in average response time
3. **Complete elimination** of timeout and error responses
4. **63x improvement** in data throughput
5. **Stable performance** (lower stdev in latency)
