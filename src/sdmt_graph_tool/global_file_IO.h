#ifndef GLOBAL_FILE_IO_H
#define GLOBAL_FILE_IO_H

#include "mpi.h"
// #include <cstring>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string.h>

/************************************
 * Information                      *
 ************************************/
// Global-file-read function.

// not used function
void readlines(MPI_File *in, const int rank, const int size, const int overlap,
               char ***lines, int *nlines);

// prefered version.
// read a file (in) parallely.
// the output is lines (each line to string vector) and nlines (the number of lines) 
// input: in - input file stream
//        rank - rank or ID of the worker
//        size - mpi cluster size of the worker
//        (out) lines - transform the text file to the string vector
//        (out) nlines - the number of lines(edges)
void readlines(std::ifstream & in, const int rank, const int size,
               std::vector<std::string> & lines, int *nlines);
#endif