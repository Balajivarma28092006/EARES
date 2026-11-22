# EARES Algorithm

## Prerequisites

- **Compiler:** `g++` supporting C++17 or later
- **Build Tool:** `make`

## Building and Running

Compile and run the project using the included `Makefile`:

```bash
# Compile and execute the program
make

# Clean compiled object files and executable
make clean
```

Alternatively, compile and run manually:

```bash
g++ -Wall -std=c++17 main.cpp config.cpp deadline.cpp energyPartitioning.cpp workflow.cpp -o main.exe
./main.exe
```

## Full Sample Output

### 1. Task Prioritization & Critical Path

Tasks are arranged in descending order of URank.

```text
Task    ETAvg   MaxETworst  URank
t1      13.00   19.88       108.00
t4      12.67   20.99       80.00
t3      14.33   23.20       80.00
t2      16.67   23.20       77.00
t5      11.67   16.53       69.00
t6      12.67   19.88       63.33
t9      16.67   24.30       44.33
t7      11.00   18.77       42.67
t8      10.00   17.65       35.67
t10     14.67   25.40       14.67

Task scheduling order: t1 t4 t3 t2 t5 t6 t9 t7 t8 t10

Critical Path: t1 t2 t9 t10
Critical Path Length: 108.00
```

### 2. Deadline Distribution

Tasks are listed in descending URank order.

```text
Task    EST     LFT     EET     Deadline
t1      0.00    13.43   13.43   13.43
t4      22.00   39.67   17.67   39.67
t3      25.00   39.33   14.33   39.33
t2      35.21   52.43   17.21   52.43
t5      24.00   55.43   31.43   55.43
t6      27.00   51.67   24.67   51.67
t9      68.43   85.64   17.21   85.64
t7      62.33   79.57   17.24   79.57
t8      66.67   85.57   18.91   85.57
t10     96.57   110.00  13.43   110.00

Scaling factor: 0.03
```

### 3. Energy Assignment & VM Allocation

Tasks are listed in descending URank order.

```text
Energy assignment and Vm assigning

Task    d(ti)   Egiven  vm      f       E(ti)   R(ti)
t1      13.43   7.3212  vm3     0.80    7.2274  0.9944
t4      39.67   8.8805  vm2     1.00    5.9200  0.9984
t3      39.33   12.4391 vm1     1.00    9.1300  0.9981
t2      52.43   12.9005 vm1     1.00    10.7900 0.9980
t5      55.43   8.5722  vm2     0.91    8.4710  0.9965
t6      51.67   7.1852  vm3     0.79    7.1170  0.9944
t9      85.64   11.2596 vm2     1.00    8.8800  0.9980
t7      79.57   8.0170  vm1     1.00    5.8100  0.9985
t8      85.57   8.4548  vm1     0.99    4.0759  0.9987
t10     110.00  12.5788 vm2     1.00    5.1800  0.9985

E(G) = 72.6012
R(G) = 0.9738
SL(G) = 89.25
```

### 4. Summary Metrics

| Metric | Description | Value |
|---|---|---:|
| `E(G)` | Total energy consumed by the workflow | 72.6012 |
| `R(G)` | Overall workflow reliability | 0.9738 |
| `SL(G)` | Total Schedule Length / Makespan | 89.25 |

---
