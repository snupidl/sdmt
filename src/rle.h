/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
 */
#ifndef RLE_H_
#define RLE_H_


#include<iostream>
using namespace std;
#include<cstdlib>  


class rle
{
    protected:
	int* in_value;
	int dim;

    public:
	rle(int*, int);
        int compress();
        int decompress();
};

rle::rle(int* in, int dim){
    in_value = in;
    dim = dim;

}

int rle::compress()
{
    int key = in_value[0];
    int freq=1;
    int* out_value;
    int out_dim = 0;

    for (int i=1; i<dim; i++){

	if (i==dim-1){
	    out_value[out_dim]=key;
	    out_dim++;
	    out_value[out_dim]=freq;
	    out_dim++;
	}

	else if (key==in_value[i]){
	    freq++;
	}	

	else {
	    out_value[out_dim]=key;
	    out_dim++;
	    out_value[out_dim]=freq;
	    out_dim++;
	    freq=1;
	}
    }

    in_value = out_value;
    return out_dim;

}



int rle::decompress()
{
    int key;
    int freq;
    int index;
    int* out_value;
    int out_dim = 0;

    for (int i=0; i<dim; i+=2){

	key = in_value[i];
	freq = in_value[i+1];

	index = 1;
	while(index<=freq){
		out_value[out_dim]=key;
		out_dim++;
		index++;
	}

    }

    in_value = out_value;
    return out_dim;

}



#endif
