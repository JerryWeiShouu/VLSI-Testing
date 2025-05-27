# VLSI Testing
The course consisted of 7 assignments, where each task incrementally extended a basic PODEM implementation to incorporate and apply the algorithms covered in lectures.

## Contents
- [Assignment 0](#assignment-1)
- [Assignment 1](#Assignment 1)
- [Assignment 2](#Assignment 2)
- [Assignment 3](#Assignment 3)
- [Assignment 4](#Assignment 4)
- [Assignment 5](#Assignment 5)
- [Assignment 6](#Assignment 6)


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
```

**Report:**
[ASS2_311510173.pdf](report/ASS2_311510173.pdf)

## Assignment 3
**Description:** Parallel pattern logic simulator
1. Please pack different number of patterns (1, 4, 8, 16) into a simulation run. Compare CPU times and memory usage on different circuits.
2. Please calculate the average number of gate evaluations for different number of packed patterns. We can collect the number of gate evaluations of each pattern and then average the number of all patterns. DOES this number has correlation with the CPU times? What is the percentage of average gate evaluations over the total number of gates?
3. Please write a compiled code simulator based on a fixed number (16) of parallel patterns. You can implement the "simulator" as a method of CIRCUIT class. In the method, you will output gate evaluation code in a levelized way (like ParallelLogicSim() in psim.cc), but all gates will be scheduled. Save the output code in a .c or .cc file and compile it as a program. Please compare the CPU time and memory usage with corresponding event-driven simulators.
When comparing CPU times, please do not perform any IO functions (e.g. cout), which dramatically slows down the program.
- The parallel logic simulation result by your simulator about c7552 circuit should be named c7552.out. The PI & PO sequence must follow the sequence in podem (PIlist & POlist).
In this assignment, TA will only use combinational circuits to evaluate your program.

**Usage:**
```
1. A new option "-simulator" you should create is as follows :
  ./atpg -simulator [output_name] -input [input_pattern_file] [circuit_name]
  Example:
  ./atpg -simulator c17.cc -input c17.input c17.bench

2. Memory usage examples:
  In command line:
  /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg c17.bench
```

**Report:**
[ASS3_311510173.pdf](report/ASS3_311510173.pdf)

## Assignment 4
**Description:** Fault list generation:  
1. Please re-implement the fault generation function with the checkpoint theorem. How much percentage of faults have been collapsed? Please compare the number of faults.
2. We would like to create a special fault list for bridging faults. To avoid the huge number of all possible faults, we usually use layout to pick adjacent nets for bridging. Since we do not want to create a very complex IFA (inductive fault analysis) framework in this homework, it is assumed that only nets within the same level can have bridging between them. Also, nets in each level are arranged in PODEM's native ordering. Each net can have up to two neighbors according to the ordering.    
For example, say we have nets in this order: n0, n1, n2, n3, ..., nk. The net n2 will only bridge with n1 and n3 in this model. But for n0, it can only bridge with one net: n1. There are two types of bridge faults between two nets. For example, (n0, n1, AND) or (n0, n1, OR). Note that each type of bridge fault will happen when the two nets have opposite logic values. For example, the bridge fault (n0, n1, AND) will happen when (n0=0, n1=1) and (n0=1, n1=0), and this bridge fault will lead to (n0=n1=0).  
Please create a bridging fault list in the PODEM program according to the description above, and print the bridge faults to the specified output file. Note that you need to create a new fault data structure for the new faults.

**Usage:**
```
1. The command for 1. is as follows:
    ./atpg -check_point [circuit_name]
2. The command for 2. is as follows:
    ./atpg -bridging -output [output_file_name] [circuit_name]
```

**Report:**
[ASS4_311510173.pdf](report/ASS4_311510173.pdf)

## Assignment 5
**Description:** Fault Simulation
1. Generate patterns with PODEM, and then run PODEM using the patterns and the "fault simulation only" option to obtain fault coverages. Also, compare the fault coverage reported by the original PODEM program and the one based on checkpoint theorem (implemented in the assignment #4).
Note that you don't need to translate the results of checkpoint faults back to the original full fault list (without collapsing).
2. The fault simulator implemented in the package uses parallel fault simulation algorithm which simulates 16 faults per pass by default. Modify the fault simulation routine to simulate more/fewer faults per pass, say 4, 8, 32, 64, etc. Fault simulator should report the same fault coverage for all these cases.
Create a table comparing the CPU runtimes under different fault numbers. Show each speed-up with respect to the case of simulating just one fault per pass. Use random patterns (one million random patterns) for fault simulation in your comparison.
3. Please implement a parallel fault simulator for the bridging fault model described in assignment #4.

**Usage:**
```
1. Use the command to generate patterns for a circuit by running built-in ATPG:
    ./atpg -output [output_pattern_file] [circuit_name]
2. You need to enroll a new command for 3.:
    ./atpg -bridging_fsim -input [input_pattern_file] [circuit_name]
```

**Report:**
[ASS5_311510173.pdf](report/ASS5_311510173.pdf)

## Assignment 6
**Description:** Combinational test generation
1. Generate test vectors for b17.bench with the PODEM program. Set backtrack limit to 1, 10, 100, 1000, or more and compare number of patterns, fault coverages, CPU run times, and actual backtrack numbers generated by running the program. (target circuit: b17.bench)
2. (Verifying ATPG results) Generate test vectors for b17.bench benchmark circuit with collapsed fault list (checkpoint fault list) and total fault list. Then run the fault simulator on the total fault list with the two sets of generated patterns. Is there any difference? If so, could you explain what've happened? (target circuit: b17.bench)
3. (PODEM Implementation) First, trace the ATPG part of the PODEM program, and try to understand the implementation of the test generation procedure. Then use c17.bench circuit to demonstrate the PODEM procedures (print to stdout by inserting printf or cout). Selected fault list is listed below and you have to exercise most procedures used in the ATPG. It is recommended to use a table for the program outputs to show each step (fault activation, fault propagation, backtrace, forward simulation, backtrack, etc.), and a decision tree will be helpful for viewing the process. (target circuit: c17.bench) fault_list
4. (ATPG with random patterns) Use a random pattern generator as the first stage of a ATPG system. The random pattern generator stops at either a fixed number of patterns (1000 patterns) or a saturated fault coverage (90%). After the random generator, PODEM will be used to target the remaining undetected faults. Please compare the results with original PODEM. Note that the random pattern stage and ATPG stage have to be integrated such that you can measure the corresponding CPU times. (target circuit: b17.bench, s35932_com.bench, s38417_com.bench, s38584_com.bench)
5. (Test generation for bridging faults) Please modify PODEM program to deal with the bridging fault list produced in homework #4-1.-b. Verify your patterns with the fault simulator implemented in assignment #5.

**Usage:**
```
1. Please just output in the original output format of PODEM ATPG.
2. Command for 4.:
    ./atpg -random_pattern -output [output_pattern] [circuit_name]
3. Command for 5.:
    ./atpg -bridging_atpg -output [output_pattern] [circuit_name]
```

**Report:**
[ASS6_311510173.pdf](report/ASS6_311510173.pdf)
