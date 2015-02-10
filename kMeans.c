/*

Program kMeans.c
for K Means Lab

*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>


#define NUM_CLUSTERS 40

/* kdata.idat */
#define MAX_NUMS 201
#define FNAME "kdata.idat"

struct means{
	int x;
	int y;
};

typedef struct means means;



    int main(){

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
                    printf(" %d ",x,y,tmp);
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
                
                printf("(%d,%d),\n",x,y);
                
                means[meanIndex].x = x;
                means[meanIndex].y = y;
                totals[meanIndex] = 0;
                meanIndex++;
            }
            printf("]\n\n");
        }
        
/*        Now that everything is set up, initalize threads and run */
        #pragma omp parallel shared(map,means)\
        private (startY,endY,id,nprocs,fp, start, diff,n,xDim,yDim)
        {
            nprocs = omp_get_num_threads();
            id = omp_get_thread_num();
        }

        
        diff = clock() - start;
        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        
	}
