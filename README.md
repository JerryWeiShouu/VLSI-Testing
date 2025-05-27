# VLSI Testing
The course consisted of 7 assignments, where each task incrementally extended a basic PODEM implementation to incorporate and apply the algorithms covered in lectures.

## Assignment 0
### Description:
Get yourself familiar with the data structure of the podem test generation package (though we do not use the test generation part yet). 
Please use the circuit parser to read benchmark circuits and then output the following attributes of a circuit. 
Choose benchmark circuits according to different criteria including combinational, sequential, optimized ,and non-optimized, larger or small circuits, etc.

### Usage:
```
./atpg -ass0 [circuit_name]
Example:
./atpg -ass0 c17.bench
```

### Report:
[ASS0_311510173.pdf](report/ASS0_311510173.pdf)


## Assignment 1
### Description:
Given a benchmark, one primary input (PI) and a primary output (PO) , please list and count all possible paths connecting the given PI and PO.
The test circuit will only be combinational circuits, so there's no Flip-Flop (DFF) nor loop in the circuit.
A path is a list of connected gates, and all listed paths have the same PI and PO gates.

### Usage:
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

### Report:
[ASS1_311510173.pdf](report/ASS1_311510173.pdf)
