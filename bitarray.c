#include <stdlib.h>

#include "bitarray.h"

BitArray * newBitArray(int size) {
	BitArray * array = (BitArray *) malloc(sizeof(BitArray));
	array->size = size;
	array->array = (BitArrayMember *) calloc(size / sizeof(BitArrayMember) + 1,
			sizeof(BitArrayMember));
	return array;
}

void destroyBitArray(BitArray * array) {
	free(array->array);
	free(array);
}
