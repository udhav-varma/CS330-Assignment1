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
	
	return NULL;
}

int memfree(void *ptr)
{
	printf("memfree() called\n");
	return 0;
}	
