#include "buffer.h"

Buffer::~Buffer() {
	/*
	Destroy the buffer and free the memory
	*/
	vkDestroyBuffer(device, buffer, nullptr);
	vkFreeMemory(device, memory, nullptr);
}