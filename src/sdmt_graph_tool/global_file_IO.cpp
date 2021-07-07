#include "global_file_IO.h"

void readlines(MPI_File *in, const int rank, const int size, const int overlap,
               char ***lines, int *nlines){
    // Read files in parallel.
    // By reading the file in, the rank of each worker in the cluster, the number of workers (size), 
    // and the overlap so that the lines can be divided into lines, 
    // the lines finally contain the string each worker has. 
    // nlines contains the number of lines of the string each worker has.
    MPI_Offset filesize;
    MPI_Offset localsize;
    MPI_Offset start;
    MPI_Offset end;
    char *chunk;

    /* figure out who reads what */

    MPI_File_get_size(*in, &filesize);
    localsize = filesize/size;
    start = rank * localsize;
    end   = start + localsize - 1;

    /* add overlap to the end of everyone's chunk... */
    end += overlap;

    /* except the last processor, of course */
    if (rank == size-1) end = filesize;

    localsize =  end - start + 1;

    /* allocate memory */
    chunk = new char[localsize + 1];
    //chunk = malloc( (localsize + 1)*sizeof(char));

    /* everyone reads in their part */
    MPI_File_read_at_all(*in, start, chunk, localsize, MPI_CHAR, MPI_STATUS_IGNORE);
    chunk[localsize] = '\0';

    /*
     * everyone calculate what their start and end *really* are by going 
     * from the first newline after start to the first newline after the
     * overlap region starts (eg, after end - overlap + 1)
     */

    int locstart=0, locend=localsize;
    if (rank != 0) {
        while(chunk[locstart] != '\n') locstart++;
        locstart++;
    }
    if (rank != size-1) {
        locend-=overlap;
        while(chunk[locend] != '\n') locend++;
    }
    localsize = locend-locstart+1;

    /* Now let's copy our actual data over into a new array, with no overlaps */
    // char *data = (char *)malloc((localsize+1)*sizeof(char));
    char *data = new char[localsize+1];
    memcpy(data, &(chunk[locstart]), localsize);
    data[localsize] = '\0';
    delete [] chunk;

    /* Now we'll count the number of lines */
    *nlines = 0;
    for (int i=0; i<localsize; i++){
        if (data[i] == '\n'){
            (*nlines)++;
        }
    }
    /* Now the array lines will point into the data array at the start of each line */
    /* assuming nlines > 1 */
    // *lines = (char **)malloc((*nlines)*sizeof(char *));
    *lines = new char*[*nlines];
    (*lines)[0] = strtok(data,"\n");
    for (int i=1; i<(*nlines); i++){
        (*lines)[i] = strtok(NULL, "\n");
    }
    return;
}

int user_min(const int a, const int b){
    if(a<=b){
        return a;
    }
    else{
        return b;
    }
}

int remainder_handle(const int a, const int b){
    if(a<b){
        return 1;
    }
    else{
        return 0;
    }
}

void readlines(std::ifstream & in, const int rank, const int size,
               std::vector<std::string> & lines, int *nlines){
    // Read files in parallel.
    // By reading the file in, the rank of each worker in the cluster, the number of workers (size), 
    // and the overlap so that the lines can be divided into lines, 
    // the lines finally contain the string each worker has. 
    // nlines contains the number of lines of the string each worker has.
    int filesize = std::count(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), '\n');
    in.clear();
    in.seekg(0, std::ios::beg);
    int localsize = filesize/size;
    int remainder = filesize - localsize * size;
    int start = rank*localsize + user_min(rank, remainder);
    int end   = start + localsize - 1 + remainder_handle(rank, remainder);
    (*nlines) = (end - start + 1);

    std::string str;

    for(int i =0; i<start; i++){
        getline(in, str);
    }

    for(int i = start; i<=end; i++){
        getline(in, str);
        lines.push_back(str);
    }

    return;
}