/*

Program kMeans.c
for K Means Lab

*/

#include <stdio.h>
#include <stdlib.h>
//#include <omp.h>
#include <time.h>


#define NUM_CLUSTERS 40

// kdata.idat
#define MAX_NUMS 201
#define FNAME "kdata.idat"

// // med_kdata.idat
// #define MAX_NUMS 2000
// #define FNAME "med_kdata.idat"

struct means{
	int x;
	int y;
	int count;
};

typedef struct means means;



	int main(){

		/* Shared variables */
		int map [MAX_NUMS][MAX_NUMS];
		means means[NUM_CLUSTERS];

		/* Private variables */
		int startY;
		int endY;
		int id;
		int nprocs;
        int n;
		FILE * fp;
        clock_t start, diff;
        
        start = clock();

		#pragma omp parallel shared(map,means)\
		private (startY,endY,id,nprocs,fp, start, diff,n)
		{
			nprocs = omp_get_num_threads();
			id = omp_get_thread_num();
		}
        
        #pragma omp master
        {
//            fp = open(FNAME,"r");
//            if(fp = NULL)
//            {
//                fprint(stderr, "Unable to read file '%s'\n", FNAME);
//                exit(2);
//            }
//            
//            n = 0;
//            do{
//                r =
//            }while (r != EOF)
        }

        
        diff = clock() - start;
        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds", msec/1000, msec%1000);
        





	}