# VLSI Testing
The course consisted of 7 assignments, where each task incrementally extended a basic PODEM implementation to incorporate and apply the algorithms covered in lectures.

## Assignment 0
**Description:**  
Get yourself familiar with the data structure of the podem test generation package (though we do not use the test generation part yet). 
Please use the circuit parser to read benchmark circuits and then output the following attributes of a circuit. 
Choose benchmark circuits according to different criteria including combinational, sequential, optimized ,and non-optimized, larger or small circuits, etc. 

**Usage:**
```
./atpg -ass0 [circuit_name]
Example:
./atpg -ass0 c17.bench
```

**Report:**
[ASS0_311510173.pdf](report/ASS0_311510173.pdf)


## Assignment 1
**Description:**  
Given a benchmark, one primary input (PI) and a primary output (PO) , please list and count all possible paths connecting the given PI and PO.
The test circuit will only be combinational circuits, so there's no Flip-Flop (DFF) nor loop in the circuit.
A path is a list of connected gates, and all listed paths have the same PI and PO gates.

**Usage:**
```
./atpg -path -start [PI] -end [PO] [circuit_name]

Example:
Input:
./atpg -path -start G3 -end PO_G16 c17.bench

Output:
G3 net17 G16 PO_G16
G3 net14 net18 G16 PO_G16
The paths from G3 to PO_G16: 2
```

**Report:**
[ASS1_311510173.pdf](report/ASS1_311510173.pdf)

## Assignment 2
**Description:**  
1. Run the program as a logic simulator on circuits c17.bench and c7552.bench.
- To verify the correctness, please use c17.input as a set of input vectors for c17.bench and their corresponding correct output responses from c17.output. Also the c7552.input (inputs) and c7552.output (outputs) are available for c7552.bench.
2. Single Pattern Logic Simulator  
    1. Generate 100, 1000, 10000 or more random patterns, and apply the generated patterns to logic simulation of combinational benchmarks. Please compare CPU times and memory usage of different circuits in your report.  
        - To measure memory usage, please check the suggestions.
        - You will need to write a small routine for the random number generator. Please be careful that you can't simply take the last bit out of any random functions such as rand(), because it's not a random bit at all. It is recommended (but not required) using GSL. Please read the explanation page for generating pseudo-random numbers.  
    2. In our current implementation of the logic simulator, we use different tables for AND, OR, and NOT operations. Please revise the program such that we use CPU instructions to compute the results directly. Note that we have to consider unknown values (X). To handle unknowns, we would need at least two bits to represent a 3-value variable (for 0, 1, and X). Also, please generate random patterns with Xs, so that you can test your new program.

**Usage:**
```
1. The commands for 2.-a. are as follows:
./atpg -pattern -num {integer} -output [output_name] [circuit_name] (To generate patterns w/o unknown)
./atpg -pattern -num {integer} -unknown -output [output_name] [circuit_name] (To generate patterns w/ unknown)
Example:
./atpg -pattern -num 100 -output c17.input c17.bench

2. The command for 2.-b. is as follows:
./atpg -mod_logicsim -input [input_name] [circuit_name]
Example:
./atpg -mod_logicsim -input c17.input c17.bench

3. Memory usage example:
  - In command line:
  /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg c17.bench
  - In main.cc, add:
  #include<sys/types.h>
  #include<unistd.h>
  int pid=(int) getpid();
  char buf[1024];
  sprintf(buf, "cat /proc/%d/statm",pid);
  system(buf);
  - In main.cc, add:
  system("ps aux | grep atpg");
```

**Report:**
[ASS2_311510173.pdf](report/ASS2_311510173.pdf)

## Assignment 3
**Description:**  
1. Run the program as a logic simulator on circuits c17.bench and c7552.bench.
- To verify the correctness, please use c17.input as a set of input vectors for c17.bench and their corresponding correct output responses from c17.output. Also the c7552.input (inputs) and c7552.output (outputs) are available for c7552.bench.
2. Single Pattern Logic Simulator  
    1. Generate 100, 1000, 10000 or more random patterns, and apply the generated patterns to logic simulation of combinational benchmarks. Please compare CPU times and memory usage of different circuits in your report.  
        - To measure memory usage, please check the suggestions.
        - You will need to write a small routine for the random number generator. Please be careful that you can't simply take the last bit out of any random functions such as rand(), because it's not a random bit at all. It is recommended (but not required) using GSL. Please read the explanation page for generating pseudo-random numbers.  
    2. In our current implementation of the logic simulator, we use different tables for AND, OR, and NOT operations. Please revise the program such that we use CPU instructions to compute the results directly. Note that we have to consider unknown values (X). To handle unknowns, we would need at least two bits to represent a 3-value variable (for 0, 1, and X). Also, please generate random patterns with Xs, so that you can test your new program.

**Usage:**
```
1. The commands for 2.i. are as follows:
./atpg -pattern -num {integer} -output [output_name] [circuit_name] (To generate patterns w/o unknown)
./atpg -pattern -num {integer} -unknown -output [output_name] [circuit_name] (To generate patterns w/ unknown)
Example:
./atpg -pattern -num 100 -output c17.input c17.bench

2. The command for 2.ii. is as follows:
./atpg -mod_logicsim -input [input_name] [circuit_name]
Example:
./atpg -mod_logicsim -input c17.input c17.bench

3. Memory usage example:
  - In command line:
  /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg c17.bench
  - In main.cc, add:
  #include<sys/types.h>
  #include<unistd.h>
  int pid=(int) getpid();
  char buf[1024];
  sprintf(buf, "cat /proc/%d/statm",pid);
  system(buf);
  - In main.cc, add:
  system("ps aux | grep atpg");
```

**Report:**
[ASS2_311510173.pdf](report/ASS2_311510173.pdf)
