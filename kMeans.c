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


#define NUM_CLUSTERS 50
#define NUM_THREADS 4
#define ERROR_THRESHOLD 10

/* kdata.idat */
#define MAX_NUMS 201
#define FNAME "kdata.idat"

typedef int bool;
enum { false, true };

struct means{
	long x;
	long y;
};

typedef struct means means;



    int main(int argc, const char* argv[] ){

		/* Shared variables */
		int map [MAX_NUMS][MAX_NUMS];
		means global_means[NUM_CLUSTERS];
        long global_totals[NUM_CLUSTERS];
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
        
        
	/*  Master thread sets up */
        #pragma omp master
        {
            start = clock();
            
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
                   /* printf("\n",x,y,tmp); */

                }
            } while (r != EOF);
            
          /*  printf("\n\n\n\n\n"); */
            
            /* Close File */
            fclose(fp);
        
            
            global_means[0].x = 0;
            global_means[0].y = 0;
            global_totals[0] = 0;
            
            /* Debug */
            printf("Mean Indexs:\n\n[(0,0)");
            
            for(mean_index = 1; mean_index < NUM_CLUSTERS; mean_index++) {
                /* Generate random points to start out as the main points of our clusters */
                
                x = abs(rand()) % MAX_NUMS;
                y = abs(rand()) % MAX_NUMS;
                
                /* Debug */
                printf(",(%d,%d)",x,y);
                
                global_means[mean_index].x = x;
                global_means[mean_index].y = y;
                global_totals[mean_index] = 0;
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
          /* printf("(%d): chunk_size: %d, startY: %d endY: %d\n",id,chunk_size,start_Y,end_Y); */
            
            /* k-Means Portion */
            n = 0;
            do {
                /*Sync Everyone Up to read in new means*/
                #pragma omp barrier
                
                /* Set up for next iteration by copying last interations
                 means to local and reseting counts */
                /* Debug */
                /* printf("{%d} Copying from global to local. Run: %d\n",id,n); */

                int mean_index;
                for (mean_index = 0; mean_index < NUM_CLUSTERS; mean_index++) {
                    /* Set up a local copy for the thread */
                    local_means[mean_index].x = 0;
                    local_means[mean_index].y = 0;
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
                        
                        /* If the current value is not a zero then recluster it */
                        if(map[x][y] != 0){
                            
                            /* Holds Shortest distance to mean recorded
                             and the index of that mean */
                            double shortest_distance = DBL_MAX;
                            int shortest_mean_index = 0;
                            
                            /* Calculate shortest distance to a mean */
                            for (mean_index=1; mean_index < NUM_CLUSTERS; mean_index++) {
                                
                                double x_Distance= abs(last_means[mean_index].x - x);
                                double y_Distance = abs(last_means[mean_index].y -y);
                                
                                /* eucliean_distance = (mean_X - this_Y)^2 + (mean_Y -this_Y)^2 */
                                double euclidian_distance = (pow(x_Distance,2) + pow(y_Distance,2));
                                
                                /* If the distance is shorter, mark it */
                                if (euclidian_distance <= shortest_distance) {
                                    shortest_distance = euclidian_distance;
                                    shortest_mean_index = mean_index;
                                }
                            }
                            
                            /* Now we know what node it is closest to we need to update our 
                             totals for that node */
                            
                            /* Update the means before processing */
                            local_totals[shortest_mean_index] ++;
                            local_means[shortest_mean_index].x += x;
                            local_means[shortest_mean_index].y += y;
                            
                            /*printf("(%d) Mean: %d = %d\n",id,shortest_mean_index,local_totals[shortest_mean_index]); */
                            
                            /* Mark node as belonging to that cluster
                             shortest_mean_index = cluster_number 
                             
                             Also this is a safe operation if I distributed the iteration space
                             correctly.  There should be no two threads who would want to write
                             to the same area.
                             */
                            map[x][y] = shortest_mean_index;
                           /* printf("(%d,%d) = %d\n",x,y,shortest_mean_index); */
                        }
                    }
                }
            
         /*   for(mean_index = 0; mean_index < NUM_CLUSTERS; mean_index++){
                
                #pragma omp barrier
                
                #pragma omp critical
                {
                printf("(%d) mean:(%d,%d) x:%d y:%d total:%d\n",id,last_means[mean_index].x,last_means[mean_index].y,local_means[mean_index].x,local_means[mean_index].y,local_totals[mean_index]);
                }
            }*/
            
            
            /* Synconyze everyone */
            #pragma omp barrier
                /* Debug */
               /* printf("\n\n{%d} Critical Section. Run: %d\n\n",id,n); */
                
                /* Parsing of map done.  Now we need to update to our new means */
                /* Each thread updates its means and cluster */
                #pragma omp critical
                {
                    for(mean_index = 1; mean_index< NUM_CLUSTERS; mean_index++){
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
                    for(mean_index = 1; mean_index< NUM_CLUSTERS; mean_index++){
                        
                        if(global_totals[mean_index] != 0){
                       
                            /* printf("x: %d y: %d total: %d\n",global_means[mean_index].x,global_means[mean_index].y,global_totals[mean_index]); */
                            
                            global_means[mean_index].x = global_means[mean_index].x/(global_totals[mean_index] +1);
                            global_means[mean_index].y = global_means[mean_index].y/(global_totals[mean_index] + 1);
                            global_totals[mean_index] = 0;
                        }
                        
                        /* Debug */
                      /*  printf("(%d,%d),",global_means[mean_index].x,global_means[mean_index].y); */
                        
                        /* We use this Error threshold because of interger roundoff when divising to find our
                        interger divsion.  This should compensate for that.  Also we really dont care about 
                         small changes to the clusters*/
                        if(abs(global_means[mean_index].x - last_means[mean_index].x) < ERROR_THRESHOLD &&
                           abs(global_means[mean_index].y - last_means[mean_index].y) < ERROR_THRESHOLD){
                            mean_stayed_same[mean_index] = true;
                        }else{
                            mean_stayed_same[mean_index] = false;
                        }
                    }
                    /* Debug */
                  /*  printf("]"); */
                    /* Compare New Means to Old Means */
                    finished = and_bool_array(mean_stayed_same,NUM_CLUSTERS);
                    
                    
                    /* Debug */
                    printf("Run: %d NUM_CLUSTERS: %d\nMean Clusters: [",n,NUM_CLUSTERS);
                    
                    for(mean_index = 0; mean_index < NUM_CLUSTERS; mean_index++){
                        printf("(%d,%d),",global_means[mean_index].x,global_means[mean_index].y);
                    }
                    
                    printf("]\n\n");
                }
                n++;
                
               /* Make everyone wait for master to finish */
                #pragma omp barrier
               
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
               
          } while (!finished);
            
        } /* End Parallel Section */

        
       
        
        #pragma omp master
        {
            diff = clock() - start;
            int msec = diff * 1000 / CLOCKS_PER_SEC;
            printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
            
            printf("Printing the Map:\n\n");
            for (y=0; y<MAX_NUMS; y++) {
                for (x=0; x < MAX_NUMS; x++) {
                    printf("(%d)",map[x][y]);
                }
                printf("\n");
            }
        }

        
	}

/* Performs a logical and of a bool array */
int and_bool_array(int array [], int size){
    int x;
    for (x=1; x<size; x++) {
        if(!array[x]){
            return false;
        }
    }
    return true;
}
