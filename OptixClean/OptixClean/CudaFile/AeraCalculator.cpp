#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>
#include <cuda.h>

#include "nvrtc.h"

#include <nvrtc_helper.h>
#include "device_launch_parameters.h"

void cudamain(int argc, char *argv[]) {
	int nDevices;
	cudaGetDeviceCount(&nDevices);
	for (int i = 0; i < nDevices; i++) {
		cudaDeviceProp prop;
		cudaGetDeviceProperties(&prop, i);
		printf("Device Number: %d\n", i);
		printf("  Device name: %s\n", prop.name);
		printf("  Memory Clock Rate (KHz): %d\n", prop.memoryClockRate);
		printf("  Memory Bus Width (bits): %d\n", prop.memoryBusWidth);
		printf("  Peak Memory Bandwidth (GB/s): %f\n\n", 2.0*prop.memoryClockRate*(prop.memoryBusWidth / 8) / 1.0e6);
	}

	char *ptx, *kernel_file = "D:\\OpenglProject\\FovMultilightTest\\NeatFW\\CudaFile\\Aera_kernel.cu";
	size_t ptxSize;
	compileFileToPTX(kernel_file, argc, argv, &ptx, &ptxSize, 0);
	CUmodule module = loadPTX(ptx, argc, argv);

	CUfunction kernel_addr;
	checkCudaErrors(cuModuleGetFunction(&kernel_addr, module, "boundCalc"));


	// Print the vector length to be used, and compute its size
	int numElements = 50000;
	size_t size = numElements * sizeof(float);
	printf("[Vector addition of %d elements]\n", numElements);

	// Allocate the host input vector A
	float *h_A = (float *)malloc(size);

	// Allocate the host input vector B
	float *h_B = (float *)malloc(size);

	// Allocate the host output vector C
	float *h_C = (float *)malloc(size);

	// Verify that allocations succeeded
	if (h_A == NULL || h_B == NULL || h_C == NULL)
	{
		fprintf(stderr, "Failed to allocate host vectors!\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the host input vectors
	for (int i = 0; i < numElements; ++i)
	{
		h_A[i] = rand() / (float)RAND_MAX;
		h_B[i] = rand() / (float)RAND_MAX;
	}

	// Allocate the device input vector A
	CUdeviceptr d_A;
	checkCudaErrors(cuMemAlloc(&d_A, size));

	// Allocate the device input vector B
	CUdeviceptr d_B;
	checkCudaErrors(cuMemAlloc(&d_B, size));

	// Allocate the device output vector C
	CUdeviceptr d_C;
	checkCudaErrors(cuMemAlloc(&d_C, size));


	// Copy the host input vectors A and B in host memory to the device input vectors in
	// device memory
	printf("Copy input data from the host memory to the CUDA device\n");
	checkCudaErrors(cuMemcpyHtoD(d_A, h_A, size));
	checkCudaErrors(cuMemcpyHtoD(d_B, h_B, size));


	// Launch the Vector Add CUDA Kernel
	int threadsPerBlock = 256;
	int blocksPerGrid = (numElements + threadsPerBlock - 1) / threadsPerBlock;
	printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
	dim3 cudaBlockSize(threadsPerBlock, 1, 1);
	dim3 cudaGridSize(blocksPerGrid, 1, 1);

	void *arr[] = { (void *)&d_A, (void *)&d_B, (void *)&d_C, (void *)&numElements };
	checkCudaErrors(cuLaunchKernel(kernel_addr,
		cudaGridSize.x, cudaGridSize.y, cudaGridSize.z, /* grid dim */
		cudaBlockSize.x, cudaBlockSize.y, cudaBlockSize.z, /* block dim */
		0, 0, /* shared mem, stream */
		&arr[0], /* arguments */
		0));
	checkCudaErrors(cuCtxSynchronize());


	// Copy the device result vector in device memory to the host result vector
	// in host memory.
	printf("Copy output data from the CUDA device to the host memory\n");
	checkCudaErrors(cuMemcpyDtoH(h_C, d_C, size));

	// Verify that the result vector is correct
	for (int i = 0; i < numElements; ++i)
	{
		if (fabs(h_A[i] + h_B[i] - h_C[i]) > 1e-5)
		{
			fprintf(stderr, "Result verification failed at element %d!\n", i);
			exit(EXIT_FAILURE);
		}
	}

	printf("Test PASSED\n");

	// Free device global memory
	checkCudaErrors(cuMemFree(d_A));
	checkCudaErrors(cuMemFree(d_B));
	checkCudaErrors(cuMemFree(d_C));

	// Free host memory
	free(h_A);
	free(h_B);
	free(h_C);
}