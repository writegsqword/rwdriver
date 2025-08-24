#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

typedef struct {
	unsigned short magic;
	unsigned int length;
	unsigned long long va;
	char buffer[1];
} image_data;

int map(image_data* buffer, unsigned int size) {
	void* (*NtConvertBetweenAuxiliaryCounterAndPerformanceCounter)(void*, void*, void*, void*);
	*(void**)&NtConvertBetweenAuxiliaryCounterAndPerformanceCounter = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");

	if (!NtConvertBetweenAuxiliaryCounterAndPerformanceCounter) {
		printf("NtConvertBetweenAuxiliaryCounterAndPerformanceCounter not found\n");
		return 1;
	}

	unsigned short magic = *(unsigned short*)buffer->buffer;

	unsigned int status = 0;
	NtConvertBetweenAuxiliaryCounterAndPerformanceCounter(0, &buffer, &status, 0);

	if (*(unsigned short*)buffer->buffer == magic) {
		printf("Failed to communicate with the mapper\n");
		return 1;
	}
	else if (buffer->buffer[0]) {
		printf("Manual mapping failed:\n\t%s\n", buffer->buffer);
		return 1;
	}

	printf("DriverEntry returned 0x%X\n", status);
	return 0;
}

int main(int argc, const char** argv) {
	if (argc < 2) {
		printf("Usage: umap VA\n");
		return 1;
	}

	unsigned va = strtoull(argv[1], NULL, 16);

	int status = 0;
	unsigned int size = 0x1000;

	image_data* buffer = malloc(size + sizeof(image_data) + 0xFF);
	if (buffer) {
		buffer->magic = 0x6789;
		buffer->length = size;
		buffer->va = va;

		status = map(buffer, size);
		printf("%s", buffer->buffer);

		free(buffer);
	}
	else {
		printf("Failed to allocate buffer of size 0x%X\n", size);
		status = 1;
	}

	//fclose(file);
	return status;
}