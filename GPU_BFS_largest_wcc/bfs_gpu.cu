/**
 File name: bfs_gpu.cu
 Author: Yuede Ji, Runzhou Han
**/
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <set>
#include <string.h>
#include <queue>
//#include "translator_json_csr.h"
#include "wtime.h"
#include "graph.h"

#define THREADS_PER_BLOCK 256
#define BLOCKS_PER_GRID 32768
#define STAND_THREADS 256
#define STAND_BLOCKS 256
#define THREAD_BIN_SIZE 32

#define INF (int)(1<<30)

#define visited_color 1
#define unvisited_color 0
#define frontier_color 2

typedef int bit_type;

const char output_file[] = "bfs_result.txt";
const char time_detail[] = "time_detail.csv";


int pivot_selection_first(
        graph *g, 
        index_t vertex_count)
{
    long int max_out_degree = 0;
    index_t index = 0;
    for(index_t j=0; j<vertex_count; ++j)
    {
        long int temp_out_degree = g->beg_pos[j+1] - g->beg_pos[j];
        if(temp_out_degree > max_out_degree)
        {
            max_out_degree = temp_out_degree;
            index = j;
        }
    }

    printf("first_pivot = %d, out_degree = %d\n", index, max_out_degree);
    return index;
}

__global__ void bfs_sync_color_top_down_first(
        index_t *d_adj_list, 
        index_t *d_beg_pos, 
	index_t *d_adj_list_r,
	index_t *d_beg_pos_r,
        bit_type *d_vertex_status, 
        bool *d_change, 
        index_t *d_vertex_count,
        index_t *d_level,
        index_t *d_frontier_queue,
        index_t *d_thread_bin)
{
    index_t id = blockIdx.x*blockDim.x + threadIdx.x;
    const index_t THDS_COUNT=blockDim.x*gridDim.x;
    index_t vertex_count = *d_vertex_count;
    index_t level = *d_level;
//    index_t bin_offset = 0;
//    index_t id = tid;
//    index_t begin_offset = 0;
//    if(id != 0)
//        begin_offset = tid * THREAD_BIN_SIZE;
    while(id < vertex_count)
    {
        if(d_vertex_status[id] == level)
        {
            for(index_t i=d_beg_pos[id]; i<d_beg_pos[id+1]; ++i)
            {
                index_t w = d_adj_list[i];

                ///atomic operation to guarantee only thread has w
                if(d_vertex_status[w] == unvisited_color)
                {
                    d_vertex_status[w] = level + 1;
                    *d_change = true;
                }
            }
	    for(index_t i=d_beg_pos_r[id]; i<d_beg_pos_r[id+1]; ++i)
            {
                index_t w = d_adj_list_r[i];

                ///atomic operation to guarantee only thread has w
                if(d_vertex_status[w] == unvisited_color)
                {
                    d_vertex_status[w] = level + 1;
                    *d_change = true;
                }
            }
        }
        id += THDS_COUNT;
    }
}

__global__ void bfs_bottom_up_first(
        bit_type *d_vertex_status, 
        index_t *d_adj_list, 
        index_t *d_beg_pos,
        index_t *d_adj_list_r,
        index_t *d_beg_pos_r, 
        bool *d_change, 
        index_t *d_vertex_count,
        index_t *d_level)
{
    index_t id = blockIdx.x*blockDim.x + threadIdx.x;
    const index_t THDS_COUNT=blockDim.x*gridDim.x;
    index_t vertex_count = *d_vertex_count;
    index_t level = *d_level;
    while(id < vertex_count)
    {
        if(d_vertex_status[id] == 0)
        {
            for(index_t i=d_beg_pos[id]; i<d_beg_pos[id+1]; ++i)
            {
                index_t w = d_adj_list[i];
                if(d_vertex_status[w] == level)
                {
                    d_vertex_status[id] = level + 1;
                    *d_change = true;
                    break;
                   // printf("%d\n", id);
                }
            }
	    for(index_t i=d_beg_pos_r[id]; i<d_beg_pos_r[id+1]; ++i)
            {
                index_t w = d_adj_list_r[i];
                if(d_vertex_status[w] == level)
                {
                    d_vertex_status[id] = level + 1;
                    *d_change = true;
                    break;
                   // printf("%d\n", id);
                }
            }

        }
        id += THDS_COUNT;
    }
}


int main(int args, char **argv)
{
    srand(time(NULL));
	cudaSetDevice(0);
    printf("args = %d\n", args);
    if(args != 5)
    {
        printf("Usage: ./scc <fw_beg_filename> <fw_csr_filename> <source>\n");
        exit(-1);
    }

    char *fw_beg_filename = argv[1];
    char *fw_csr_filename = argv[2];
    char *bw_beg_filename = argv[3];
    char *bw_csr_filename = argv[4];
    //index_t root = atoi(argv[5]);

    graph *g = new graph(fw_beg_filename, fw_csr_filename);
    graph *g_r = new graph(bw_beg_filename, bw_csr_filename);
    const index_t vertex_count = g->vert_count + 1;
    const index_t edge_count = g->edge_count;
    
	bit_type *vertex_status = (bit_type *)malloc(sizeof(bit_type)*vertex_count);
    index_t *frontier_queue = (index_t*)malloc(sizeof(index_t)*vertex_count);
    index_t root = 1;
    //index_t root = pivot_selection_first(g,vertex_count);	   
	//------------------------------------------------------------------------
    //Deciding how many blocks to be used   
    index_t number_of_blocks = 1;
    //index_t number_of_threads_per_block = vertex_count;
    
    if(vertex_count > THREADS_PER_BLOCK)
    {
        number_of_blocks = (index_t)ceil(vertex_count/(double)THREADS_PER_BLOCK);
        if(number_of_blocks > BLOCKS_PER_GRID)
            number_of_blocks = BLOCKS_PER_GRID;
    }
    
//    printf("blocks = %d, threads = %d\n", number_of_blocks, number_of_threads_per_block);
    //------------------------------------------------------------------------
    //allocating auxiliars in CPU
    for(index_t i = 0 ; i < vertex_count ; ++i)
    {
        vertex_status[i] = 0;//no colors 
    }
    //index_t root = pivot_selection_first(g, vertex_count);
    vertex_status[root] = 1;
    //------------------------------------------------------------------------
    //Allocating GPU memory:
    index_t *d_adj_list_reverse;
    cudaMalloc((void**) &d_adj_list_reverse, sizeof(index_t)*edge_count);
    cudaMemcpy( d_adj_list_reverse, g_r->csr, sizeof(index_t)*edge_count, cudaMemcpyHostToDevice);

    index_t *d_beg_pos_reverse;
    cudaMalloc((void**) &d_beg_pos_reverse, sizeof(index_t)*(vertex_count + 1));
    cudaMemcpy( d_beg_pos_reverse, g_r->beg_pos, sizeof(index_t)*(vertex_count + 1), cudaMemcpyHostToDevice);

    index_t *d_adj_list_direct;
    cudaMalloc((void**) &d_adj_list_direct, sizeof(index_t)*edge_count);
    cudaMemcpy( d_adj_list_direct, g->csr, sizeof(index_t)*edge_count, cudaMemcpyHostToDevice);

	index_t *d_beg_pos_direct;
    cudaMalloc((void**) &d_beg_pos_direct, sizeof(index_t)*(vertex_count + 1));
    cudaMemcpy( d_beg_pos_direct, g->beg_pos, sizeof(index_t)*(vertex_count + 1), cudaMemcpyHostToDevice);

	bit_type *d_vertex_status;
	cudaMalloc((void**) &d_vertex_status, sizeof(bit_type)*vertex_count);
    cudaMemcpy( d_vertex_status, vertex_status, sizeof(index_t)*vertex_count, cudaMemcpyHostToDevice);

    index_t *d_vertex_count;
    cudaMalloc((void **) &d_vertex_count, sizeof(index_t));
    cudaMemcpy(d_vertex_count, &(vertex_count), sizeof(index_t), cudaMemcpyHostToDevice);
    
    index_t offset = 0;
    index_t *d_offset;
    cudaMalloc((void **) &d_offset, sizeof(index_t));
    cudaMemcpy(d_offset, &(offset), sizeof(index_t), cudaMemcpyHostToDevice);

    index_t *d_frontier_queue;
    cudaMalloc((void **) &d_frontier_queue, sizeof(index_t) * vertex_count);
    
    index_t *d_thread_bin;
    cudaMalloc((void **) &d_thread_bin, sizeof(index_t) * STAND_BLOCKS * STAND_THREADS * THREAD_BIN_SIZE);
    cudaMemcpy(d_vertex_count, &(vertex_count), sizeof(index_t), cudaMemcpyHostToDevice);
    //CPU & GPU shared variable
    
    index_t * level;
    index_t * d_level;
    cudaHostAlloc((void **) &level, sizeof(index_t), cudaHostAllocMapped);
    cudaHostGetDevicePointer(&d_level, level, 0);
    *level = 0;
    
    bool * change;
    bool * d_change;
    cudaHostAlloc((void **) &change, sizeof(bool), cudaHostAllocMapped);
    cudaHostGetDevicePointer(&d_change, change, 0);
    *change = true;

    FILE * fp_time = fopen(time_detail, "w");
    double time = wtime();
    while(*change)
    {
        (*level) ++;
        *change = false;
        double temp_time_beg = wtime();
	if(*level<3)
	{        
        bfs_sync_color_top_down_first<<<STAND_BLOCKS, STAND_THREADS>>>(
                d_adj_list_direct, 
                d_beg_pos_direct,
		d_adj_list_reverse,
		d_beg_pos_reverse, 
                d_vertex_status, 
                d_change, 
                d_vertex_count,
                d_level,
                d_frontier_queue,
                d_thread_bin);
	}
	else{
           bfs_bottom_up_first<<<STAND_BLOCKS, STAND_THREADS>>>(
                d_vertex_status,
                d_adj_list_direct,
                d_beg_pos_direct, 
                d_adj_list_reverse, 
                d_beg_pos_reverse, 
                d_change, 
                d_vertex_count,
                d_level);
	}
 
        cudaDeviceSynchronize();
        double temp_time = wtime() - temp_time_beg;
        fprintf(fp_time, "%d, %g\n", *level, temp_time*1000);
    }
    double bfs_fw_time = wtime() - time;
    fclose(fp_time);
    printf("depth = %d\n", *level);
    printf("bfs time = %g (ms)\n", bfs_fw_time * 1000);
    cudaMemcpy(vertex_status, d_vertex_status, sizeof(index_t)*vertex_count, cudaMemcpyDeviceToHost);
    index_t largest_wcc = 0;
    for(index_t i=0; i<vertex_count; i++)
    {
        if(vertex_status[i] != 0)
        {
                largest_wcc++;
        }
    }
    printf("largest wcc = %d\n", largest_wcc); 
   FILE * fp_out = fopen(output_file, "w");
    for(index_t i=0; i<vertex_count; ++i)
    {
        fprintf(fp_out, "%d %d\n", i, vertex_status[i]);
    }
    fclose(fp_out);

	free(vertex_status);
    cudaFree(d_adj_list_direct);
    cudaFree(d_beg_pos_direct);
    cudaFree(d_adj_list_reverse);
    cudaFree(d_beg_pos_reverse);
	cudaFree(d_vertex_status);
	cudaFree(d_change);
	return 0;
}

