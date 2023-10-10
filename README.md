# CPU Cache Simulator

### Simulation of multi-level memory: 
- One CPU register
- Three levels of fully associative cache (L1, L2, L3)
- Memory (or RAM)

> The 'sample' file provides 11 hex address values which should output a total of 3,471 cycles.

### Usage
```
make
./cache
```
```
Usage: ./cache [-l] [-f]  
  -l: Displays all occupied cache lines (file mode only)  
  -f: Input file
```
