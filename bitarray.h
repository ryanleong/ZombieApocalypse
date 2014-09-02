/*
 * bitarray.h
 *
 *  Created on: Sep 1, 2014
 *      Author: adam
 */

#ifndef BITARRAY_H_
#define BITARRAY_H_

#include <stdbool.h>

typedef unsigned int BitArrayMember;

typedef struct BitArray {
	int size;
	BitArrayMember * array;
} BitArray;

BitArray * newBitArray(int size);

void destroyBitArray(BitArray * array);

#define testBit(bitArray, index) (((bitArray)->array[(index) / sizeof(BitArrayMember)] & (1<<((index) % sizeof(BitArrayMember))))!=0)
#define setBit(bitArray, index) (bitArray)->array[(index) / sizeof(BitArrayMember)] |= (1<<((index) % sizeof(BitArrayMember)))
#define clearBit(bitArray,index) (bitArray)->array[(index) / sizeof(BitArrayMember)] &= ~(1<<((index) % sizeof(BitArrayMember)))

#endif /* BITARRAY_H_ */
