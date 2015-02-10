/*

Program kMeans.c
for K Means Lab

*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <float.h>
#include <limits.h>


#define NUM_CLUSTERS 40
#define NUM_THREADS 50

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
		means global_means[NUM_CLUSTERS];
        int global_totals[NUM_CLUSTERS];
        int finished = 0;

		/* Private variables */
		int start_Y;
		int end_Y;
		int id;
		int n_procs;
        int r;
        int n;
        int mean_index;
        int x;
        int y;
        int tmp;
        int local_totals [NUM_CLUSTERS];
        means last_means [NUM_CLUSTERS];
        means local_means [NUM_CLUSTERS];
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
            fscanf(fp,"%d",&tmp);
            fscanf(fp,"%d",&tmp);
            
            x=0,y=0;
            do{
/*              Reads in one number  */
                r = fscanf(fp,"%d",&tmp);
                
/*                When reaches end of line it should iterate y++ and reset x back to 0;  */
                if ( r != EOF && x < MAX_NUMS && y < MAX_NUMS) {
                    map [x][y] = tmp;
                    x++;
                    /* Debug */
                    /* printf(" %d ",tmp); */
                }else{
                    y++;
                    x = 0;
                    /* Debug */
                   /*  printf("\n",x,y,tmp); */

                }
            } while (r != EOF);
            
            /* Close File */
            fclose(fp);
            
            /* Debug */
            printf("\n\n\nMean Indexs:\n\n[");
            
            for(mean_index = 0; mean_index < NUM_CLUSTERS; mean_index++) {
                /* Generate random points to start out as the main points of our clusters */
                
                x = rand() % MAX_NUMS;
                y = rand() % MAX_NUMS;
                
                /* Debug */
                printf("(%d,%d),",x,y);
                
                global_means[mean_index].x = x;
                global_means[mean_index].y = y;
                global_totals[mean_index] = 0;
                mean_index++;
            }
            /* Debug */
            printf("]\n\n");
        }
        
/*        Now that everything is set up, initalize threads and run */
        #pragma omp parallel shared(map,global_means,global_totals,finished)\
        private (start_Y,end_Y,id,n_procs,n,local_totals,local_means,x,y,mean_index,last_means)\
        num_threads(NUM_THREADS)
        {
            n_procs = omp_get_num_threads();
            id = omp_get_thread_num();
            
            #pragma omp barrier
            /* Find out size to divide work evenly */
            int n_mod_nprocs = MAX_NUMS % n_procs,
                chunk_size;
            
            /* Figure out how much work can be done across processes */
            if(id < n_mod_nprocs){
                
                chunk_size = MAX_NUMS /n_procs + 1;
                
                start_Y = id*chunk_size;
                end_Y = (id+1)*chunk_size;
                
            }else{
                
                chunk_size = MAX_NUMS/n_procs;
                
                start_Y = (chunk_size+1)*n_mod_nprocs + (id-n_mod_nprocs)*chunk_size;
                end_Y = (chunk_size+1)*n_mod_nprocs + (id-n_mod_nprocs+1)*chunk_size;
                
            }
            
            /* Debug */
            /*  Check in to make sure space allocated correctly */
           printf("(%d): chunk_size: %d, startY: %d endY: %d\n",id,chunk_size,start_Y,end_Y);
            
            /* k-Means Portion */
            n = 0;
       /*     do { */
                /*Sync Everyone Up to read in new means*/
                #pragma omp barrier
                
                /* Set up for next iteration by copying last interations
                 means to local and reseting counts */
                /* Debug */
                /* printf("{%d} Copying from global to local. Run: %d\n",id,n); */

                int mean_index;
                for (mean_index = 0; mean_index < NUM_CLUSTERS; mean_index++) {
                    /* Set up a local copy for the thread */
                    local_means[mean_index].x = global_means[mean_index].x;
                    local_means[mean_index].y = global_means[mean_index].y;
                    local_totals[mean_index] = 0;
                    
                    /* Also make a copy of the last round so we can verify
                     at the end if there are any changes after the last
                     iteration */
                    last_means[mean_index].x = global_means[mean_index].x;
                    last_means[mean_index].y = global_means[mean_index].y;
                    
                }
                
                /* Debug */
                /* printf("{%d} Processing Section of Map. Run: %d\n",id,n); */
                
                int x, y;
                /* Rows */
                for (y=start_Y; y<end_Y; y++) {
                    /* Columns */
                    for (x = 0; x < MAX_NUMS; x++) {
                        
                        /* Holds Shortest distance to mean recorded 
                         and the index of that mean */
                        int shortest_distance = INT_MAX;
                        int shortest_mean_index;
                        
                        /* If the current value is not a zero then recluster it */
                        if(map[x][y] != 0){
                            /* Calculate shortest distance to a mean */
                            for (mean_index=1; mean_index < NUM_CLUSTERS; mean_index++) {
                                
                                
                                int x_Distance= last_means[mean_index].x - x;
                                x_Distance *= x_Distance;
                                int y_Distance = last_means[mean_index].y -y;
                                y_Distance *= y_Distance;
                                
                                /* eucliean_distance = (mean_X - this_Y)^2 + (mean_Y -this_Y)^2 */
                                int euclidian_distance = x_Distance + y_Distance;
                                
                                /* If the distance is shorter, mark it */
                                if (euclidian_distance < shortest_distance) {
                                    shortest_distance = euclidian_distance;
                                    shortest_mean_index = mean_index;
                                }
                            }
                            
                            /* Now we know what node it is closest to we need to update our 
                             totals for that node */
                            
                            /* Update the means before processing */
                            local_totals[shortest_mean_index] ++;
                            local_means[shortest_mean_index].x = x;
                            local_means[shortest_mean_index].y = y;
                            
                            /* Mark node as belonging to that cluster
                             shortest_mean_index = cluster_number 
                             
                             Also this is a safe operation if I distributed the iteration space
                             correctly.  There should be no two threads who would want to write
                             to the same area.
                             */
                            map[x][y] = shortest_mean_index;
                        }
                    }
                }
                
                /* Debug */
               /* printf("\n\n{%d} Critical Section. Run: %d\n\n",id,n); */
                
                /* Parsing of map done.  Now we need to update to our new means */
                /* Each thread updates its means and cluster */
                #pragma omp critical
                {
                    for(mean_index = 0; mean_index< NUM_CLUSTERS; mean_index++){
                        global_means[mean_index].x += local_means[mean_index].x;
                        global_means[mean_index].y += local_means[mean_index].y;
                        global_totals[mean_index] += local_totals[mean_index];
                    }
                }
                
                /* Synconyze everyone */
                #pragma omp barrier
                
                /* Let master see if we are done */
                #pragma omp master
                {
                    /* Debug */
                /*    printf("\nRound %d New Means:\n\n[",n); */
                    
                    int mean_stayed_same [NUM_CLUSTERS];
                    /* Update Global Means */
                    for(mean_index = 0; mean_index< NUM_CLUSTERS; mean_index++){
                        
                        if(global_totals[mean_index] != 0){
                            global_means[mean_index].x = global_means[mean_index].x/global_totals[mean_index];
                            global_means[mean_index].y += global_means[mean_index].y/global_totals[mean_index];
                            global_totals[mean_index] = 0;
                        }
                        
                        /* Debug */
                      /*  printf("(%d,%d),",global_means[mean_index].x,global_means[mean_index].y); */
                        
                        if(global_means[mean_index].x == last_means[mean_index].x &&
                           global_means[mean_index].y == last_means[mean_index].y){
                            mean_stayed_same[mean_index] = 1;
                        }else{
                            mean_stayed_same[mean_index] = 0;
                        }
                    }
                    /* Debug */
                  /*  printf("]"); */
                    /* Compare New Means to Old Means */
                    finished = and_bool_array(mean_stayed_same,NUM_CLUSTERS);
                }
                n++;
                
                
                
                /* Debug */
                /* Print the Map */
             /*   printf("Printing the Map:\n\n");
                for (y=0; y<MAX_NUMS; y++) {
                    for (x=0; x < MAX_NUMS; x++) {
                        printf("%d",map[x][y]);
                    }
                    printf("\n");
                }
            */
     /*       } while (!finished); */
            
        } /* End Parallel Section */

        
        diff = clock() - start;
        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        
        #pragma omp master
        {
            printf("Printing the Map:\n\n");
            for (y=0; y<MAX_NUMS; y++) {
                for (x=0; x < MAX_NUMS; x++) {
                    printf("%d",map[x][y]);
                }
                printf("\n");
            }
        }

        
	}

/* Performs a logical and of a bool array */
int and_bool_array(int array [], int size){
    int x;
    for (x=0; x<size; x++) {
        if(!array[x]){
            return 0;
        }
    }
    return 1;
}
