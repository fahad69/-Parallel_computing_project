# MPI Map Reduce Framework for Matrix Multiplication

This project aims to emulate the Map Reduce framework using MPI in C. The objective is to implement matrix multiplication within the Map Reduce framework, where the master process assigns tasks to mappers and reducers. The execution is performed on a Beowulf Cluster, and the output is compared with the result of a serial matrix multiplication program.

## Background

The Map Reduce framework works by splitting the input data to mappers, shuffling the mapper outputs to reducers, and generating the final output. In this project, we utilize MPI to create a master-slave configuration, with one process acting as the master (Process 0) and the remaining processes as mappers or reducers. The assignment of processes to mappers and reducers is dynamic and depends on the number of processes used.

## Project Requirements

- Create square random matrices of sizes greater than or equal to 24 and save them to files.
- The master process assigns the task of splitting the input to the mappers.
- Mappers split the input into key-value pairs and inform the master process after completing their job.
- The master process shuffles the mapper outputs and assigns reduce jobs to reducers.
- Reducers perform their work on the input passed to them and write key-value pairs to a file.
- Reducers notify the master after completing their job.
- The master process converts the key-value pairs to matrix form and writes the result to a file.

## Execution Details

To execute the program, pass the filename of the input files as command-line arguments.

The assignment of processes as mappers and reducers is dynamic and depends on the number of processes used for execution.

## Expected Output

The master process and each mapper or reducer will print informative messages during the execution. For example:

- The master process will print its own information: "Master with process_id <process_num> running on <machine_name>."
- The master process will print the message whenever it assigns a task: "Task <Map/Reduce> assigned to process <process_num>."
- Each mapper or reducer will print the message whenever they are assigned a task: "Process <process_num> received task <Map/Reduce> on <machine_name>."
- The master process will print the message whenever it receives the status of completion of a task: "Process <process_num> has completed task <Map/Reduce>."
- The master process will inform the user when the entire job has been completed.
- The master process will compare the matrix multiplication output with the output of the serial matrix multiplication program and print if the two outputs are the same or not.

Refer to Figure 2 in the project description for a sample output.

## Note

The use of a Beowulf Cluster is mandatory for this project. Ensure that the machine names printed by each process clearly indicate that the program is executing on multiple machines. Failure to demonstrate the project using a Beowulf Cluster will result in a score of 0.

