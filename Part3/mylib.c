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
	unsigned long * prev = NULL;
	while(ptr != NULL){
		unsigned long * freespace = *(ptr);
		if(freespace <= size){
			break;
		}
		else{
			unsigned long * nextaddr = (unsigned long*)(*(ptr + 1));
			prev = ptr;
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
			*(endPointer + 1) = 0;
			*(endPointer + 2) = (unsigned long) prev;
			if(prev != NULL)
				*(prev + 1) = (unsigned long) endPointer;
		}
		return returnPointer;
	}	
	else{	// Enough space at ptr block
		unsigned long spaceAvailable = *(ptr);
		unsigned long * nextFree = (unsigned long*) *(ptr + 1);
		if(spaceAvailable < size + 24) size = spaceAvailable;	// Padding
		*(ptr) = size;
		void * returnPointer = (void*) (ptr + 1);
		if(nextFree - size >= 24){
			unsigned long* endPointer = ptr + (size/8);
			*endPointer = spaceAvailable - size;
			*(endPointer + 1) = (unsigned long) nextFree;
			*(endPointer + 2) = (unsigned long) prev;
			if(prev != NULL){
				*(prev + 1) = (unsigned long) endPointer;
			}
			if(nextFree != NULL){
				*(nextFree + 2) = (unsigned long) endPointer;
			}
		}
		else{
			if(nextFree != NULL){
				*(nextFree + 2) = (unsigned long) prev;
			}
			if(prev != NULL){
				*(prev + 1) = (unsigned long) nextFree;
			}
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
