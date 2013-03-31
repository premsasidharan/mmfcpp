#include <stdio.h>
#include <stdint.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

#include <histogram.h>
#include <cuda_gl_interop.h>

texture<uchar2, 2, cudaReadModeElementType> tex_y;
texture<uchar4, 2, cudaReadModeElementType> tex_uv;

__device__ int dev_max[3];

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

__global__ void comp_histogram(GLint* hist_r, GLint* hist_g, GLint* hist_b, int w, int h)
{
    int x, y;
    uint8_t r_c, g_c, b_c;
    uint8_t y_c, u_c, v_c;

    __shared__ int temp_hist_r[256];
    __shared__ int temp_hist_g[256];
    __shared__ int temp_hist_b[256];
 
    if (blockIdx.x == 0)
    {
        hist_r[1+(2*threadIdx.x)] = 0;
        hist_g[1+(2*threadIdx.x)] = 0;
        hist_b[1+(2*threadIdx.x)] = 0;

        hist_r[(2*threadIdx.x)] = (threadIdx.x == 0)?0:threadIdx.x;
        hist_g[(2*threadIdx.x)] = (threadIdx.x == 0)?0:threadIdx.x;
        hist_b[(2*threadIdx.x)] = (threadIdx.x == 0)?0:threadIdx.x;
    }

    __syncthreads();

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
            y_c = tex2D(tex_y, x, y).x;
            u_c = tex2D(tex_uv, x/2, y).y;
            v_c = tex2D(tex_uv, x/2, y).w;

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

    atomicAdd(&hist_r[1+(2*threadIdx.x)], temp_hist_r[threadIdx.x]);
    atomicAdd(&hist_g[1+(2*threadIdx.x)], temp_hist_g[threadIdx.x]);
    atomicAdd(&hist_b[1+(2*threadIdx.x)], temp_hist_b[threadIdx.x]);
}

__global__ void get_max(GLint* hist_r, GLint* hist_g, GLint* hist_b)
{
    int i;
    GLint* hist;
    __shared__ GLint max_hist[3];

    hist = (threadIdx.x == 0)?hist_r:((threadIdx.x == 1)?hist_g:hist_b);
    max_hist[threadIdx.x] = 0;

    __syncthreads();
    
    for (i = 1; i < 256; i++)
    {
        if (hist[1+(2*i)] > max_hist[threadIdx.x])
        {
            max_hist[threadIdx.x] = hist[1+(2*i)];
        }
    }

    __syncthreads();

    dev_max[threadIdx.x] = max_hist[threadIdx.x];
}

void print_cuda_device_info();

void compute_histogram(unsigned int* texture, unsigned int* hist_obj, int* hist_max, int width, int height)
{
    cudaError_t err;
    GLint* dev_hist[3] = {0, 0, 0};
    cudaArray* array[3] = {0, 0, 0};
    cudaGraphicsResource* res[3] = {0, 0, 0};

    for (int i = 0; i < 3; i++)
    {
        cudaGLRegisterBufferObject(hist_obj[i]);
        cudaGLMapBufferObject((void **)&dev_hist[i], hist_obj[i]);
    }

    err = cudaGraphicsGLRegisterImage(&res[0], texture[0], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsReadOnly);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsGLRegisterImage Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }

    err = cudaGraphicsGLRegisterImage(&res[1], texture[1], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsReadOnly);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsGLRegisterImage Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }

    cudaGraphicsMapResources(2, res);
    for (int i = 0; i < 2; i++)
    {
        err = cudaGraphicsSubResourceGetMappedArray(&array[i], res[i], 0, 0);
        if (err != cudaSuccess)
        {
            printf("cudaGraphicsSubResourceGetMappedArray Failed: %s", cudaGetErrorString(cudaGetLastError()));
            exit(0);
        }
    }

    cudaChannelFormatDesc y_chan_desc = cudaCreateChannelDesc<uchar2>();
    if (cudaBindTextureToArray(&tex_y, array[0], &y_chan_desc) != cudaSuccess) {
	    printf("Failed to bind y texture: %s\n", cudaGetErrorString(cudaGetLastError()));
	    exit(0);
    }
    cudaChannelFormatDesc uv_chan_desc = cudaCreateChannelDesc<uchar4>();
    if (cudaBindTextureToArray(&tex_uv, array[1], &uv_chan_desc) != cudaSuccess) {
	    printf("Failed to bind uv texture: %s\n", cudaGetErrorString(cudaGetLastError()));
	    exit(0);
    }

    comp_histogram<<<64, 256>>>(dev_hist[0], dev_hist[1], dev_hist[2], width, height);
    cudaThreadSynchronize();
    get_max<<<1, 3>>>(dev_hist[0], dev_hist[1], dev_hist[2]);

    cudaMemcpyFromSymbol(hist_max, dev_max, 3*sizeof(int));

    cudaUnbindTexture(&tex_y);
    cudaUnbindTexture(&tex_uv);

    cudaGraphicsUnmapResources(2, res);

    cudaGraphicsUnregisterResource(res[0]);
    cudaGraphicsUnregisterResource(res[1]);
    
    for (int i = 0; i < 3; i++)
    {
        cudaGLUnmapBufferObject(hist_obj[i]);
        cudaGLUnregisterBufferObject(hist_obj[i]);
    }
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

