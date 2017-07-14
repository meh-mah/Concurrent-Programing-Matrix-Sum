*Short description of the applications
The program computes a sum of matrix elements, min value, max value and their corsponding position in the matrix in parallel using Pthreads. The matrix is filled with random numbers between 0 to 100 using rand() method. The main thread prints the final results include the time taken to finish the task. The program uses the concept of "bag of tasks", and each thread takes a row and performs computation. Taking a row is in critical section to prevent two threads from working on same row. After finishing the calculation on each row the thread will update the shared variables (min, max, sum, positions) this is also in critical section to prevent concurrent write.

*Description of command-line parameters
Default value for matrix size is 10000 and number of threads is 10. These can be changed from command line as described below.

*Instructions to build and to run the application:
$ gcc –Wall matrixSum.c –o matrixSum.out –lpthread
$ ./matrixSum.out <matrix size> <number of threads>
