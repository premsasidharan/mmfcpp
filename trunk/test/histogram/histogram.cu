#include <stdio.h>
#include <stdint.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

/*
red = y+1.5958*v;
green = y-0.39173*u-0.81290*v;
blue = y+2.017*u;*/

texture<uint8_t, 2, cudaReadModeElementType> tex_y;
texture<uint8_t, 2, cudaReadModeElementType> tex_u;
texture<uint8_t, 2, cudaReadModeElementType> tex_v;

__device__ inline uint8_t red(uint8_t y, uint8_t v)
{
    return (uint8_t) (y+1.5958*v);
}

__device__ inline uint8_t green(uint8_t y, uint8_t u, uint8_t v)
{
    return (uint8_t) (y-0.39173*u-0.81290*v);
}

__device__ inline uint8_t blue(uint8_t y, uint8_t u)
{
    return (uint8_t) (y+2.017*u);
}

__global__ void comp_histogram(uint32_t* hist_r, uint32_t* hist_g, uint32_t* hist_b, int w, int h)
{
    int x, y;
    uint8_t r_c, g_c, b_c;
    uint8_t y_c, u_c, v_c;

    __shared__ uint32_t temp_hist_r[256];
    __shared__ uint32_t temp_hist_g[256];
    __shared__ uint32_t temp_hist_b[256];


    temp_hist_r[threadIdx.x] = 0;
    temp_hist_g[threadIdx.x] = 0;
    temp_hist_b[threadIdx.x] = 0;

    __syncthreads();

    y = blockIdx.x;
    while (y < h)
    {
        x = threadIdx.x;
        while (x < w)
        {
            y_c = tex2D(tex_y, x, y);
            u_c = tex2D(tex_u, x/2, y/2);
            v_c = tex2D(tex_v, x/2, y/2);

            r_c = red(y_c, v_c);
            g_c = green(y_c, u_c, v_c);
            b_c = blue(y_c, u_c);

            atomicAdd(&temp_hist_r[r_c], 1);
            atomicAdd(&temp_hist_g[g_c], 1);
            atomicAdd(&temp_hist_b[b_c], 1);    

            x += blockDim.x;      
        }
        y += gridDim.x;
    }

    __syncthreads();

    atomicAdd(&hist_r[threadIdx.x], temp_hist_r[threadIdx.x]);
    atomicAdd(&hist_g[threadIdx.x], temp_hist_g[threadIdx.x]);
    atomicAdd(&hist_b[threadIdx.x], temp_hist_b[threadIdx.x]);
}

void print_cuda_device_info();

int main(int argc, char** argv)
{
    uint32_t host_hist_r[256];
    uint32_t host_hist_g[256];
    uint32_t host_hist_b[256];

    if (argc != 4)
    {
        printf("\nFailed !!!\n");
        return 0;
    }

	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	const char* file_path = argv[1];
	
    cudaArray* dev_src = 0;
    cudaArray* dev_src_u = 0;
    cudaArray* dev_src_v = 0;

    uint32_t* dev_hist_r = 0;
    uint32_t* dev_hist_g = 0;
    uint32_t* dev_hist_b = 0;
 
    int y_size = width*height;
    int uv_size = (y_size>>2);
	int yuv_size = y_size+(uv_size<<1);
    uint8_t* host_src_mem = new uint8_t[yuv_size];

    cudaChannelFormatDesc y_chan_desc = cudaCreateChannelDesc<uint8_t>();	
    cudaChannelFormatDesc u_chan_desc = cudaCreateChannelDesc<uint8_t>();	
    cudaChannelFormatDesc v_chan_desc = cudaCreateChannelDesc<uint8_t>();

    cudaMallocArray(&dev_src, &y_chan_desc, width, height);
    cudaMallocArray(&dev_src_u, &u_chan_desc, width>>1, height>>1);
    cudaMallocArray(&dev_src_v, &v_chan_desc, width>>1, height>>1);

    cudaMalloc((void **)&dev_hist_r, 256*sizeof(uint32_t));
    cudaMalloc((void **)&dev_hist_g, 256*sizeof(uint32_t));
    cudaMalloc((void **)&dev_hist_b, 256*sizeof(uint32_t));

    tex_y.addressMode[0] = cudaAddressModeWrap;
    tex_y.addressMode[1] = cudaAddressModeWrap;
    tex_y.filterMode = cudaFilterModePoint;
    tex_y.normalized = false;

    if (cudaBindTextureToArray(&tex_y, dev_src, &y_chan_desc) != cudaSuccess) {
	    printf("Failed to bind y texture: %s\n", cudaGetErrorString(cudaGetLastError()));
	    return 0;
    }

    tex_u.addressMode[0] = cudaAddressModeWrap;
    tex_u.addressMode[1] = cudaAddressModeWrap;
    tex_u.filterMode = cudaFilterModePoint;
    tex_u.normalized = false;

    if (cudaBindTextureToArray(&tex_u, dev_src_u, &u_chan_desc) != cudaSuccess) {
	    printf("Failed to bind u texture: %s\n", cudaGetErrorString(cudaGetLastError()));
	    return 0;
    }

    tex_v.addressMode[0] = cudaAddressModeWrap;
    tex_v.addressMode[1] = cudaAddressModeWrap;
    tex_v.filterMode = cudaFilterModePoint;
    tex_v.normalized = false;

    if (cudaBindTextureToArray(&tex_v, dev_src_v, &v_chan_desc) != cudaSuccess) {
	    printf("Failed to bind v texture: %s\n", cudaGetErrorString(cudaGetLastError()));
	    return 0;
    }

    int frame = 0;
	FILE* file = fopen(file_path, "r");
	
    while (1)
    {
	    if (0 == fread(host_src_mem, 1, yuv_size, file))
        {
            break;
        }

        if (cudaMemcpyToArray(dev_src, 0, 0, host_src_mem, y_size, cudaMemcpyHostToDevice) != cudaSuccess)
        {
		    printf("1 Failed cudaMemcpyToArray: %s\n", cudaGetErrorString(cudaGetLastError()));
		    return 0;
        }
        if (cudaMemcpyToArray(dev_src_u, 0, 0, &host_src_mem[y_size], uv_size, cudaMemcpyHostToDevice) != cudaSuccess)
        {
		    printf("2 Failed cudaMemcpyToArray: %s\n", cudaGetErrorString(cudaGetLastError()));
		    return 0;
        }
        if (cudaMemcpyToArray(dev_src_v, 0, 0, &host_src_mem[y_size+uv_size], uv_size, cudaMemcpyHostToDevice) != cudaSuccess)
        {
		    printf("3 Failed cudaMemcpyToArray: %s\n", cudaGetErrorString(cudaGetLastError()));
		    return 0;
        }

        cudaMemset(dev_hist_r, 0, 256*sizeof(uint32_t));
        cudaMemset(dev_hist_g, 0, 256*sizeof(uint32_t));
        cudaMemset(dev_hist_b, 0, 256*sizeof(uint32_t));

        comp_histogram<<<64, 256>>>(dev_hist_r, dev_hist_g, dev_hist_b, width, height);

        cudaThreadSynchronize();

        cudaMemcpy(host_hist_r, dev_hist_r, 256*sizeof(uint32_t), cudaMemcpyDeviceToHost);
        cudaMemcpy(host_hist_g, dev_hist_g, 256*sizeof(uint32_t), cudaMemcpyDeviceToHost);
        cudaMemcpy(host_hist_b, dev_hist_b, 256*sizeof(uint32_t), cudaMemcpyDeviceToHost);

        printf("\nFrame: %d ", frame++);
        for (int i = 0; i < 256; i++)
        {
            printf("%d:(%u %u %u), ", i, host_hist_r[i], host_hist_g[i], host_hist_b[i]);
        }
    }
    fclose(file);
        
    cudaUnbindTexture(&tex_v);
    cudaUnbindTexture(&tex_u);
    cudaUnbindTexture(&tex_y);

    cudaFree(dev_hist_r); dev_hist_r = 0; 
    cudaFree(dev_hist_g); dev_hist_g = 0; 
    cudaFree(dev_hist_b); dev_hist_b = 0;  
 
    cudaFree(dev_src_v); dev_src_v = 0;  
    cudaFree(dev_src_u); dev_src_u = 0;   
    cudaFree(dev_src); dev_src = 0;
    
    delete [] host_src_mem;
    host_src_mem = 0;

    return 0;
}

void print_cuda_device_info()
{
    int count = 0;
    cudaDeviceProp prop;

    cudaGetDeviceCount(&count);

    printf("\nCUDA Device Count: %d", count); 
    for (int i = 0; i < count; i++)
    {
        cudaGetDeviceProperties(&prop, i);
        printf("\nDevice: %d", i);
        printf("\nName: %s", prop.name);
        printf("\nRevision: Major: %d, Minor: %d", prop.major, prop.minor);
        printf("\nWarp Size: %d", prop.warpSize);
        printf("\nMemory Bus width: %d", prop.memoryBusWidth);
        printf("\nMemory Clock Rate: %d", prop.memoryClockRate);
        printf("\nConcurrent Kernels: %d", prop.concurrentKernels);
        printf("\nMultiprocessor count: %d", prop.multiProcessorCount);
        printf("\nTotal Global Memory: %d", (int)prop.totalGlobalMem);
        printf("\nTotal Constant Memory: %d", (int)prop.totalConstMem);
        printf("\nShared Memory per Block: %d", (int)prop.sharedMemPerBlock);
        printf("\nMax grid dimensions: (%d, %d, %d)", prop.maxGridSize[0], 
                                                       prop.maxGridSize[1], 
                                                       prop.maxGridSize[2]);
        printf("\nMax threads per block: %d", prop.maxThreadsPerBlock);
        printf("\nMax threads dimensions: (%d, %d, %d)\n", prop.maxThreadsDim[0], 
                                                       prop.maxThreadsDim[1], 
                                                       prop.maxThreadsDim[2]);
    }
}

