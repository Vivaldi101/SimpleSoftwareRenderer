#include <Windows.h>
#include <assert.h>		// fixme: remove
#include "shared.h"

#define BYTE_INDEX_16	1		//index of first 16-byte payload
#define BYTE_INDEX_32	18		//index of first 16-byte payload
#define BYTE_INDEX_64	51		//index of first 16-byte payload
#define BYTE_INDEX_128	116		//index of first 16-byte payload
#define BYTE_INDEX_256	245		//index of first 16-byte payload
#define BYTE_INDEX_512	502		//index of first 16-byte payload
#define BYTE_INDEX_1024	1015	//index of first 16-byte payload
#define BYTE_INDEX_2048	2040	//index of first 16-byte payload

#define BLOCK_STATE(data, index)	((data)[(index) - 1])	// check if empty block

#define FREE_BLOCK 0xfb
#define ALLOCATED_BLOCK 0xab

static void InitColumn(ListAllocator *list, int index) {
	int num_rows = list->num_rows;
	for (int i = 0; i < num_rows; ++i) {
		BLOCK_STATE(list->data, index) = FREE_BLOCK;
		index += LIST_ROW_SIZE;
	}
}

void DestroyListAllocator(ListAllocator *list) {
	list->num_rows = 0;
	list->num_bytes = 0;
	VirtualFree(list->data, 0, MEM_RELEASE);
}

void InitListAllocator(ListAllocator **list, size_t num_bytes) {
	*list = (ListAllocator *)VirtualAlloc(0, num_bytes + sizeof(ListAllocator), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	(*list)->num_bytes = num_bytes;
	(*list)->num_rows = (int)num_bytes / LIST_ROW_SIZE;
	(*list)->data = ((byte *)*list) + sizeof(ListAllocator);

	InitColumn(*list, BYTE_INDEX_16);
	InitColumn(*list, BYTE_INDEX_32);
	InitColumn(*list, BYTE_INDEX_64);
	InitColumn(*list, BYTE_INDEX_128);
	InitColumn(*list, BYTE_INDEX_256);
	InitColumn(*list, BYTE_INDEX_512);
	InitColumn(*list, BYTE_INDEX_1024);
	InitColumn(*list, BYTE_INDEX_2048);
}


static int SearchColumn(ListAllocator *list, int index) {
	int num_rows = list->num_rows;

	for (int i = 0; i < num_rows; ++i) {
		if (BLOCK_STATE(list->data, index) == FREE_BLOCK) {
			BLOCK_STATE(list->data, index) = ALLOCATED_BLOCK;

			return index;
		}
		index += LIST_ROW_SIZE;
	}

	return 0;
}

void *Allocate(ListAllocator *list, size_t num_bytes) {
	int index;

	if (num_bytes <= 16) {
		if (index = SearchColumn(list, BYTE_INDEX_16)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 32) {
		if (index = SearchColumn(list, BYTE_INDEX_32)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 64) {
		if (index = SearchColumn(list, BYTE_INDEX_64)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 128) {
		if (index = SearchColumn(list, BYTE_INDEX_128)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 256) {
		if (index = SearchColumn(list, BYTE_INDEX_256)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 512) {
		if (index = SearchColumn(list, BYTE_INDEX_512)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 1024) {
		if (index = SearchColumn(list, BYTE_INDEX_1024)) {
			return &list->data[index];
		}
	} else if (num_bytes <= 2048) {
		if (index = SearchColumn(list, BYTE_INDEX_2048)) {
			return &list->data[index];
		}
	}

	assert(0 && "Couldn't allocate memory");
	return 0;
}

void Free(ListAllocator *list, void **ptr) {
	int index = (int)((byte *)(*ptr) - list->data);
	index = index % LIST_ROW_SIZE;
	*ptr = 0;

	if (index >= 1 || index < list->num_bytes) {
		BLOCK_STATE(list->data, index) = FREE_BLOCK;
		switch (index) {
			case BYTE_INDEX_16: {
				memset(&list->data[index], 0, 16); 
			} break; 
			case BYTE_INDEX_32: {
				memset(&list->data[index], 0, 32); 
			} break; 
			case BYTE_INDEX_64: {
				memset(&list->data[index], 0, 64); 
			} break; 
			case BYTE_INDEX_128: {
				memset(&list->data[index], 0, 128); 
			} break; 
			case BYTE_INDEX_256: {
				memset(&list->data[index], 0, 256); 
			} break; 
			case BYTE_INDEX_512: {
				memset(&list->data[index], 0, 512); 
			} break; 
			case BYTE_INDEX_1024: {
				memset(&list->data[index], 0, 1024); 
			} break; 
			case BYTE_INDEX_2048: {
				memset(&list->data[index], 0, 2048); 
			} break; 
		}
	}
}
