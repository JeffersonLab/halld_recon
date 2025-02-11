
// This is basically copied from HDEVIO.cc in the DAQ library of 
// sim-recon. It has been modified to remove references to the
// HDEVIO class. The swap_bank, swap_segment, and swap_tagsegment 
// routines were also modified to assume the inputs were of the
// native endianess and that we are swapping to the opposite.

#include <string.h>


#include <iostream>
using namespace std;

#include "hdbyte_swapout.h"



//---------------------------------
// swap_block
//---------------------------------
void swap_block_out(uint16_t *inbuff, uint16_t len, uint16_t *outbuff)
{
	for(uint32_t i=0; i<len; i++, inbuff++, outbuff++){
		uint16_t inword = *inbuff;  // copy word to allow using same buffer for input and output
		uint8_t *inptr  = (uint8_t*)&inword;
		uint8_t *outptr = (uint8_t*)&outbuff[1];
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
	}
}

//---------------------------------
// swap_block
//---------------------------------
void swap_block_out(uint32_t *inbuff, uint32_t len, uint32_t *outbuff)
{
	for(uint32_t i=0; i<len; i++, inbuff++, outbuff++){
		uint32_t inword = *inbuff;  // copy word to allow using same buffer for input and output
		uint8_t *inptr  = (uint8_t*)&inword;
		uint8_t *outptr = (uint8_t*)&outbuff[1];
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
	}
}

//---------------------------------
// swap_block
//---------------------------------
void swap_block_out(uint64_t *inbuff, uint64_t len, uint64_t *outbuff)
{
	for(uint32_t i=0; i<len; i++, inbuff++, outbuff++){
		uint64_t inword = *inbuff;  // copy word to allow using same buffer for input and output
		uint8_t *inptr  = (uint8_t*)&inword;
		uint8_t *outptr = (uint8_t*)&outbuff[1];
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;

		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
		*(--outptr) = *inptr++;
	}
}

//---------------------------------
// swap_bank
//---------------------------------
uint32_t swap_bank_out(uint32_t *outbuff, uint32_t *inbuff, uint32_t len)
{
	/// n.b. This was modified from the original which assumed we were 
	/// swapping from wrong endianess for the process to the right one.
	/// This routine is being used by hdl3 to do the opposite.
	///
	/// Swap an EVIO bank. If the bank contains data, it is automatically
	/// swapped according to it's type. If the bank is a container of other
	/// containers, then this repeatedly calls the swapper methods for the
	/// appropriate container type (bank, tagsegment, segment). This means
	/// that this method will be recursive in the cases where it is a bank
	/// of banks.

	if(len < 2){
		cerr << "Attempt to swap bank with len<2" << endl;
		return 0;
	}
	
	// Swap length and header words
	swap_block_out(inbuff, 2, outbuff);
	uint32_t bank_len = inbuff[0];
	if((bank_len+1) > len){
		cerr << "WARNING: Bank length word exceeds valid words in buffer (" << bank_len+1 << " > " << len << ")" << endl;
		return 0;
	}
	
	uint32_t type = (inbuff[1]>>8) & 0xFF;
	uint32_t Nwords = bank_len - 1; // number of 32bit payload words
	uint32_t Nswapped = 2;
	switch(type){
		case 0x0a:  // 64 bit unsigned int
		case 0x08:  // 64 bit double
		case 0x09:  // 64 bit signed int
			swap_block_out((uint64_t*)&inbuff[2], Nwords/2, (uint64_t*)&outbuff[2]);
			Nswapped += Nwords;
			break;
		case 0x01:  // 32 bit unsigned int
		case 0x02:  // 32 bit float
		case 0x0b:  // 32 bit signed int
			swap_block_out(&inbuff[2], Nwords, &outbuff[2]);
			Nswapped += Nwords;
			break;
		case 0x05:  // 16 bit unsigned int
		case 0x04:  // 16 bit signed int
			swap_block_out((uint16_t*)&inbuff[2], Nwords*2, (uint16_t*)&outbuff[2]);
			Nswapped += Nwords;
			break;
		case 0x00:  // 32 bit unknown (not swapped)
		case 0x07:  // 8 bit unsigned int
		case 0x06:  // 8 bit signed int
			memcpy((uint8_t*)&outbuff[2], (uint8_t*)&inbuff[2], Nwords*sizeof(uint32_t));
			Nswapped += Nwords;
			break;
		case 0x0c:
			while(Nswapped < (Nwords+2)){
				uint32_t N = swap_tagsegment_out(&outbuff[Nswapped], &inbuff[Nswapped], (Nwords+2)-Nswapped);
				if(N == 0) return Nswapped;
				Nswapped += N;
			}
			break;
		case 0x0d:
		case 0x20:
			while(Nswapped < (Nwords+2)){
				uint32_t N = swap_segment_out(&outbuff[Nswapped], &inbuff[Nswapped], (Nwords+2)-Nswapped);
				if(N == 0) return Nswapped;
				Nswapped += N;
			}
			break;
		case 0x0e:
		case 0x10:
			while(Nswapped < (Nwords+2)){
				uint32_t N = swap_bank_out(&outbuff[Nswapped], &inbuff[Nswapped], (Nwords+2)-Nswapped);
				if(N == 0) return Nswapped;
				Nswapped += N;
			}
			break;
		default:
			cerr << "WARNING: unknown bank type (0x" << hex << type << dec << ")" << endl;
			return 0;
			break;
	}

	return Nswapped;
}

//---------------------------------
// swap_tagsegment
//---------------------------------
uint32_t swap_tagsegment_out(uint32_t *outbuff, uint32_t *inbuff, uint32_t len)
{
	/// Swap an EVIO tagsegment. 

	if(len < 1){
		cerr << "Attempt to swap segment with len<1" << endl;
		return 0;
	}
	
	// Swap header/length word
	swap_block_out(inbuff, 1, outbuff);
	uint32_t bank_len = inbuff[0] & 0xFFFF;
	if((bank_len) > len){
		cerr << "Tag Segment length word exceeds valid words in buffer (" << bank_len << " > " << len << ")" << endl;
		return 0;
	}
	
	uint32_t type = (inbuff[0]>>16) & 0x0F;
	uint32_t Nwords = bank_len;
	uint32_t Nswapped = 1;
	switch(type){
		case 0x0a:  // 64 bit unsigned int
		case 0x08:  // 64 bit double
		case 0x09:  // 64 bit signed int
			swap_block_out((uint64_t*)&inbuff[1], Nwords/2, (uint64_t*)&outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x01:  // 32 bit unsigned int
		case 0x02:  // 32 bit float
		case 0x0b:  // 32 bit signed int
			swap_block_out(&inbuff[1], Nwords, &outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x05:  // 16 bit unsigned int
		case 0x04:  // 16 bit signed int
			swap_block_out((uint16_t*)&inbuff[1], Nwords*2, (uint16_t*)&outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x00:  // 32 bit unknown (not swapped)
		case 0x07:  // 8 bit unsigned int
		case 0x06:  // 8 bit signed int
			memcpy((uint8_t*)&outbuff[1], (uint8_t*)&inbuff[1], Nwords*sizeof(uint32_t));
			Nswapped += Nwords;
			break;
	}

	return Nswapped;
}

//---------------------------------
// swap_segment
//---------------------------------
uint32_t swap_segment_out(uint32_t *outbuff, uint32_t *inbuff, uint32_t len)
{
	/// Swap an EVIO segment. 
	///
	/// n.b. This was modified from the original which assumed we were 
	/// swapping from wrong endianess for the process to the right one.
	/// This routine is being used by hdl3 to do the opposite.

	if(len < 1){
		cerr << "Attempt to swap segment with len<1" << endl;
		return 0;
	}
	
	// Swap header/length word
	swap_block_out(inbuff, 1, outbuff);
	uint32_t bank_len = inbuff[0] & 0xFFFF;
	if((bank_len) > len){
		cerr << "Segment length word exceeds valid words in buffer (" << bank_len << " > " << len << ")" << endl;
		return 0;
	}
	
	uint32_t type = (inbuff[0]>>16) & 0x3F;
	uint32_t Nwords = bank_len;
	uint32_t Nswapped = 1;
	switch(type){
		case 0x0a:  // 64 bit unsigned int
		case 0x08:  // 64 bit double
		case 0x09:  // 64 bit signed int
			swap_block_out((uint64_t*)&inbuff[1], Nwords/2, (uint64_t*)&outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x01:  // 32 bit unsigned int
		case 0x02:  // 32 bit float
		case 0x0b:  // 32 bit signed int
			swap_block_out(&inbuff[1], Nwords, &outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x05:  // 16 bit unsigned int
		case 0x04:  // 16 bit signed int
			swap_block_out((uint16_t*)&inbuff[1], Nwords*2, (uint16_t*)&outbuff[1]);
			Nswapped += Nwords;
			break;
		case 0x00:  // 32 bit unknown (not swapped)
		case 0x07:  // 8 bit unsigned int
		case 0x06:  // 8 bit signed int
			memcpy((uint8_t*)&outbuff[1], (uint8_t*)&inbuff[1], Nwords*sizeof(uint32_t));
			Nswapped += Nwords;
			break;
	}

	return Nswapped;
}
