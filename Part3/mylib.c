#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define ERROR(...) fprintf(stderr, __VA_ARGS__)

unsigned long * head = NULL;
const unsigned long memblock = 4*(1ull << 20);	// 4 MB

void *memalloc(unsigned long size)
{
	printf("memalloc() called\n");
	size += 8;
	size = 8*((size + 7)/8);
	if(size < 24) size = 24;
	// ERROR("Make a block of size: %ld\n", size);
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
	// ERROR("%p\n", ptr);
	if(ptr == NULL){	// Not enough space
		unsigned long sizeofNewBlock = memblock*((size + memblock - 1)/memblock);	// Ceiling of size/memblock
		unsigned long * newBlockaddr = (unsigned long*) mmap(NULL, sizeofNewBlock, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
		// ERROR("New block of sz: %ld created at %p\n", sizeofNewBlock, newBlockaddr);
		if(sizeofNewBlock < size + 24) size = sizeofNewBlock;	// Padding (if necessary)
		*newBlockaddr = size;
		void * returnPointer = (void*) (newBlockaddr + 1);
		if(sizeofNewBlock - size >= 24){
			unsigned long * endPointer = newBlockaddr + (size/8);
			// ERROR("%p %ld %p\n", newBlockaddr, (size/8), endPointer);
			*endPointer = sizeofNewBlock - size;
			*(endPointer + 2) = 0;
			*(endPointer + 1) = (unsigned long) head;
			if(head != NULL)
				*(head + 2) = (unsigned long) endPointer;
			head = endPointer;
		}
		// ERROR("%p %p %ld\n", returnPointer, head, *((unsigned long*) head));
		// ERROR("returning %p\n", returnPointer);
		// ERROR("Succesful Allocation of %ld at %p\n", size, newBlockaddr);
		return returnPointer;
	}	
	else{	// Enough space at ptr block
		unsigned long spaceAvailable = *(ptr);
		unsigned long * nextFree = (unsigned long*) *(ptr + 1);
		unsigned long * prevFree = (unsigned long*) *(ptr + 2);
		// ERROR("Found space %ld at %p\n", spaceAvailable, ptr);
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
		// ERROR("returninig %p\n", returnPointer);
		return returnPointer;
	}
	return NULL;
}

void printLinkedList()
{
	unsigned long* ptr = head;
	int iter = 0;
	while(ptr != NULL){
		iter++;
		ERROR("Pointer: %p, size: %ld, next: %p, prev: %p\n", ptr, *ptr, (unsigned long*)*(ptr + 1), (unsigned long*)*(ptr + 2));
		ptr = (unsigned long*) *(ptr + 1);
		if(iter == 7) break;
	}
}

int memfree(void *ptr_)
{
	printf("memfree() called\n");
	unsigned long * ptr = (unsigned long*) ptr_;
	ptr -= 1;
	unsigned long size = *ptr;
	// ERROR("Attempt to deallocate %ld bytes at %p\n", size, ptr);
	// ERROR("Before calling: ");
	// printLinkedList();
	unsigned long * iterptr = head;
	int iter = 0;
	while(iterptr != NULL){
		iter++;
		// if(iter > 0 && iter%1000 == 0) ERROR("Iterations: %d\n", iter);
		if(iter == 10000){
			exit(0);
		}
		unsigned long sz = *iterptr;
		if(iterptr + (sz/8) == ptr){	// Coalesce with previous block (beginning from iterptr), remove from linked list
			// ERROR("Found previous block at %p to coalesce with %p\n", iterptr, ptr);
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
			// ERROR("Found next block at %p to coalesce with %p\n", iterptr, ptr);
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
	// ERROR("After Calling: \n");
	// printLinkedList();
	return 0;
}	

// #define NUM 4
// #define _1GB (1024*1024*1024)

// //Handling large allocations
// int main()
// {
// 	char *p[NUM];
// 	char *q = 0;
// 	int ret = 0;
// 	int a = 0;

// 	for(int i = 0; i < NUM; i++)
// 	{
// 		p[i] = (char*)memalloc(_1GB);
// 		if((p[i] == NULL) || (p[i] == (char*)-1))
// 		{
// 			printf("1.Testcase failed\n");
// 			return -1;
// 		}

// 		for(int j = 0; j < _1GB; j++)
// 		{
// 			p[i][j] = 'a';
// 		}
// 	}
// 	// printLinkedList();
// 	// memfree(p[0]);
// 	// ERROR("After\n");
// 	// printLinkedList();
// 	for(int i = 0; i < NUM - 1; i++)
// 	{
// 		ret = memfree(p[i]);
// 		if(ret != 0)
// 		{
// 			printf("2.Testcase failed\n");
// 			return -1;
// 		}
// 		printf("done\n");
// 	}
// 	// printLinkedList();
// 	memfree(p[3]);
// 	printf("Testcase passed\n");
// 	return 0;
// }

