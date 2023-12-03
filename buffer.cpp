#include "buffer.h"

Buffer::~Buffer() {
	/*
	Destroy the buffer and free the memory
	*/
	vkDestroyBuffer(gpu->logical_gpu, buffer, nullptr);
	vkFreeMemory(gpu->logical_gpu, memory, nullptr);
}