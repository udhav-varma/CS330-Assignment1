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
	size = 8*((size + 7)/8);	// Next multiple of 8
	if(size < 24) size = 24;	// Necessary that every block is atleast 24
	unsigned long * ptr = head;
	while(ptr != NULL){
		unsigned long freespace = *(ptr);
		if(freespace >= size){
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
		if(newBlockaddr == MAP_FAILED){
			return NULL;
		}
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
			*(nextFree + 2) = (unsigned long) prevFree;
		}
		if(prevFree != NULL){
			*(prevFree + 1) = (unsigned long) nextFree;
		}
		if(spaceAvailable - size >= 24){
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

int memfree(void *ptr_)
{
	printf("memfree() called\n");
	unsigned long * ptr = (unsigned long*) ptr_;
	ptr -= 1;
	unsigned long size = *ptr;
	unsigned long * iterptr = head;
	while(iterptr != NULL){
		unsigned long sz = *iterptr;
		if(iterptr + (sz/8) == ptr){	// Coalesce with previous block (beginning from iterptr), remove from linked list
			size += sz;
			unsigned long * prev = (unsigned long*) *(iterptr + 2);
			unsigned long * next = (unsigned long*) *(iterptr + 1);
			if(next != NULL){
				*(next + 2) = (unsigned long) prev;
			}
			if(prev != NULL){
				*(prev + 1) = (unsigned long) next;
			}
			if(iterptr == head){
				head = next;
			}
			ptr = iterptr;
			iterptr = next;
		}
		else if(ptr + (size/8) == iterptr){	// Coalesce ptr with the next block from iterptr, remove from linked list
			size += sz;
			unsigned long * prev = (unsigned long*) *(iterptr + 2);
			unsigned long * next = (unsigned long*) *(iterptr + 1);
			if(next != NULL){
				*(next + 2) = (unsigned long) prev;
			}
			if(prev != NULL){
				*(prev + 1) = (unsigned long) next;
			}
			if(iterptr == head){
				head = next;
			}
			iterptr = next;
		}
		else{
			iterptr = (unsigned long*) *(iterptr + 1); 
		}
	}
	*ptr = size;
	*(ptr + 1) = (unsigned long) head;
	*(ptr + 2) = 0;
	if(head != NULL)
		*(head + 2) = (unsigned long) ptr;
	head = ptr;
	return 0;
}	
