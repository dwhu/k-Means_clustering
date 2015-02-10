/*

Program kMeans.c
for K Means Lab

*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>


#define NUM_CLUSTERS 40
#define NUM_THREADS 5

/* kdata.idat */
#define MAX_NUMS 201
#define FNAME "kdata.idat"

struct means{
	int x;
	int y;
};

typedef struct means means;



    int main(int argc, const char* argv[] ){

		/* Shared variables */
		int map [MAX_NUMS][MAX_NUMS];
		means means[NUM_CLUSTERS];
        int totals[NUM_CLUSTERS];

		/* Private variables */
		int startY;
		int endY;
		int id;
		int nprocs;
        int n, r;
        int xDim, yDim;
        int localTotals [NUM_CLUSTERS];
		FILE * fp;
        clock_t start, diff;
        
        start = clock();
        
	/*  Master thread sets up */
        #pragma omp master
        {
            fp = fopen(FNAME,"r");
            if(fp == NULL)
            {
                fprintf(stderr, "Unable to read file '%s'\n", FNAME);
                exit(2);
            }
            
            /*
             * Initialize Map of Vars
             *
             */
            
/*            Read off the dimensions of the array */
            fscanf(fp,"%d",&xDim);
            fscanf(fp,"%d",&yDim);
            
            
            int x = 0;
            int y = 0;
            int tmp;
            do{
/*              Reads in one number  */
                r = fscanf(fp,"%d",&tmp);
                
/*                When reaches end of line it should iterate y++ and reset x back to 0;  */
                if ( r != EOF && x < MAX_NUMS && y < MAX_NUMS) {
                    map [x][y] = tmp;
                    x++;
                    /* Debug */
                    printf(" %d ",tmp);
                }else{
                    y++;
                    x = 0;
                    /* Debug */
                    printf("\n",x,y,tmp);

                }
            } while (r != EOF);
            
            /* Close File */
            fclose(fp);
            printf("\n\n\nMean Indexs:\n\n[");
            int meanIndex = 0;
            while(meanIndex < NUM_CLUSTERS) {
                /* Generate random points to start out as the main points of our clusters */
                
                x = rand() % MAX_NUMS;
                y = rand() % MAX_NUMS;
                
                printf("(%d,%d),",x,y);
                
                means[meanIndex].x = x;
                means[meanIndex].y = y;
                totals[meanIndex] = 0;
                meanIndex++;
            }
            printf("]\n\n");
        }
        
/*        Now that everything is set up, initalize threads and run */
        #pragma omp parallel shared(map,means)\
        private (startY,endY,id,nprocs,fp, start, diff,n,xDim,yDim)\
        num_threads(NUM_THREADS)
        {
            nprocs = omp_get_num_threads();
            id = omp_get_thread_num();
            
            #pragma omp barrier
            /* Find out size to divide work evenly */
            int n_mod_nprocs = MAX_NUMS % nprocs,
                chunk_size;
            
            /* Figure out how much work can be done across processes */
            if(id < n_mod_nprocs){
                
                chunk_size = MAX_NUMS /nprocs + 1;
                
                startY = id*chunk_size;
                endY = (id+1)*chunk_size;
                
            }else{
                
                chunk_size = MAX_NUMS/nprocs;
                
                startY = (chunk_size+1)*n_mod_nprocs + (id-n_mod_nprocs)*chunk_size;
                endY = (chunk_size+1)*n_mod_nprocs + (id-n_mod_nprocs+1)*chunk_size;
                
            }
            
            /*  Check in to make sure space allocated correctly */
            printf("(%d): chunk_size: %d, startY: %d endY: %d\n",id,chunk_size,startY,endY);
            
            
        }

        
        diff = clock() - start;
        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        
	}
