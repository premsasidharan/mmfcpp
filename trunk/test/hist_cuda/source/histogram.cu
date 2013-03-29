#include <stdio.h>
#include <stdint.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

#include <histogram.h>
#include <cuda_gl_interop.h>

texture<uint8_t, 2, cudaReadModeElementType> tex_y;
texture<uint8_t, 2, cudaReadModeElementType> tex_u;
texture<uint8_t, 2, cudaReadModeElementType> tex_v;

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

    atomicAdd(&hist_r[1+(2*threadIdx.x)], temp_hist_r[threadIdx.x]);
    atomicAdd(&hist_g[1+(2*threadIdx.x)], temp_hist_g[threadIdx.x]);
    atomicAdd(&hist_b[1+(2*threadIdx.x)], temp_hist_b[threadIdx.x]);
}

__global__ void get_max(GLint* hist_r, GLint* hist_g, GLint* hist_b/*, GLint* max*/)
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

    /*if (threadIdx.x == 0)
    {
        *max = 0;
        for (i = 0 ; i < 3; i++)
        {
            if (max_hist[i] > *max)
            {
                *max = max_hist[i];
            }
        }
    }*/
}

void print_cuda_device_info();

void compute_histogram(unsigned int* texture, unsigned int* hist_obj, int* hist_max, int width, int height)
{
    GLint* dev_hist_r = 0;
    GLint* dev_hist_g = 0;
    GLint* dev_hist_b = 0;

    cudaGLRegisterBufferObject(hist_obj[0]);
    cudaGLMapBufferObject((void **)&dev_hist_r, hist_obj[0]);
    cudaGLRegisterBufferObject(hist_obj[1]);
    cudaGLMapBufferObject((void **)&dev_hist_g, hist_obj[1]);
    cudaGLRegisterBufferObject(hist_obj[2]);
    cudaGLMapBufferObject((void **)&dev_hist_b, hist_obj[2]);

    cudaError_t err;
    cudaGraphicsResource* res[3] = {0, 0, 0};
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
    err = cudaGraphicsGLRegisterImage(&res[2], texture[2], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsReadOnly);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsGLRegisterImage Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }

    cudaGraphicsMapResources(3, res);

    cudaArray* y_array = 0;
    cudaArray* u_array = 0;
    cudaArray* v_array = 0;
    err = cudaGraphicsSubResourceGetMappedArray(&y_array, res[0], 0, 0);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsSubResourceGetMappedArray Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }
    err = cudaGraphicsSubResourceGetMappedArray(&u_array, res[1], 0, 0);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsSubResourceGetMappedArray Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }
    err = cudaGraphicsSubResourceGetMappedArray(&v_array, res[2], 0, 0);
    if (err != cudaSuccess)
    {
        printf("cudaGraphicsSubResourceGetMappedArray Failed: %s", cudaGetErrorString(cudaGetLastError()));
        exit(0);
    }

    cudaChannelFormatDesc y_chan_desc = cudaCreateChannelDesc<uint8_t>();
    cudaChannelFormatDesc u_chan_desc = cudaCreateChannelDesc<uint8_t>();
    cudaChannelFormatDesc v_chan_desc = cudaCreateChannelDesc<uint8_t>();
	if (cudaBindTextureToArray(&tex_y, y_array, &y_chan_desc) != cudaSuccess) {
		printf("Failed to bind y texture: %s\n", cudaGetErrorString(cudaGetLastError()));
		exit(0);
	}
	if (cudaBindTextureToArray(&tex_u, u_array, &u_chan_desc) != cudaSuccess) {
		printf("Failed to bind y texture: %s\n", cudaGetErrorString(cudaGetLastError()));
		exit(0);
	}
	if (cudaBindTextureToArray(&tex_v, v_array, &v_chan_desc) != cudaSuccess) {
		printf("Failed to bind y texture: %s\n", cudaGetErrorString(cudaGetLastError()));
		exit(0);
	}

    comp_histogram<<<64, 256>>>(dev_hist_r, dev_hist_g, dev_hist_b, width, height);
    cudaThreadSynchronize();
    get_max<<<1, 3>>>(dev_hist_r, dev_hist_g, dev_hist_b);

    cudaMemcpyFromSymbol(hist_max, dev_max, 3*sizeof(int));

    cudaUnbindTexture(&tex_y);
    cudaUnbindTexture(&tex_u);
    cudaUnbindTexture(&tex_v);

    cudaGLUnmapBufferObject(hist_obj[0]);
    cudaGLUnmapBufferObject(hist_obj[1]);
    cudaGLUnmapBufferObject(hist_obj[2]);

    cudaGLUnregisterBufferObject(hist_obj[0]);
    cudaGLUnregisterBufferObject(hist_obj[1]);
    cudaGLUnregisterBufferObject(hist_obj[2]);

    cudaGraphicsUnmapResources(3, res);
    cudaGraphicsUnregisterResource(res[0]);
    cudaGraphicsUnregisterResource(res[1]);
    cudaGraphicsUnregisterResource(res[2]);
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

