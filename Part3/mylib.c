#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

unsigned long * head = NULL;
const unsigned long memblock = 4*(1ull << 20);	// 4 MB

void *memalloc(unsigned long size)
{
	printf("memalloc() called\n");
	size += 8;
	size = 8*((size + 7)/8);
	unsigned long * ptr = head;
	while(ptr != NULL){
		unsigned long * freespace = *(ptr);
		if(freespace <= size){
			break;
		}
		else{
			unsigned long * nextaddr = (unsigned long*)(*(ptr + 1));
			ptr = nextaddr;
		}
	}
	if(ptr == NULL){	// Not enough space
		unsigned long sizeofNewBlock = memblock*((size + memblock - 1)/memblock);	// Ceiling of size/memblock
		unsigned long * newBlockaddr = (unsigned long*) mmap(NULL, sizeofNewBlock, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
		if(sizeofNewBlock < size + 24) size = sizeofNewBlock;	// Padding (if necessary)
		*newBlockaddr = size;
		void * returnPointer = (void*) (newBlockaddr + 1);
		if(sizeofNewBlock - size >= 24){
			unsigned long * endPointer = newBlockaddr + (size/8);
			*endPointer = sizeofNewBlock - size;
			*(endPointer + 2) = 0;
			*(endPointer + 1) = (unsigned long) head;
			if(head != NULL)
				*(head + 2) = (unsigned long) endPointer;
			head = endPointer;
		}
		return returnPointer;
	}	
	else{	// Enough space at ptr block
		unsigned long spaceAvailable = *(ptr);
		unsigned long * nextFree = (unsigned long*) *(ptr + 1);
		unsigned long * prevFree = (unsigned long*) *(ptr + 2);
		if(spaceAvailable < size + 24) size = spaceAvailable;	// Padding
		*(ptr) = size;
		void * returnPointer = (void*) (ptr + 1);
		if(nextFree != NULL){
			*(nextFree + 2) = prevFree;
		}
		if(prevFree != NULL){
			*(prevFree + 1) = nextFree;
		}
		if(nextFree - size >= 24){
			unsigned long* endPointer = ptr + (size/8);
			*endPointer = spaceAvailable - size;
			*(endPointer + 2) = 0;
			*(endPointer + 1) = (unsigned long) head;
			if(head != NULL)
				*(head + 2) = (unsigned long) endPointer;
			head = endPointer;
		}
		return returnPointer;
	}
	return NULL;
}

int memfree(void *ptr)
{
	printf("memfree() called\n");

	return 0;
}	
