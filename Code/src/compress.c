#include "compress.h"
#include "quad.h"
uchar getbit(uchar byte, size_t b){
    return (byte >> b) & 1;
}

void setbit(unsigned char *ptr, size_t capa, int bit) {
    if (bit) {
        // Set bit from MSB to LSB (7-0)
        *ptr |= (1 << (7 - capa));
    } else {
        // Clear bit from MSB to LSB (7-0)
        *ptr &= ~(1 << (7 - capa));
    }
}
void check(BitStream* stream){
  // Check if we have written 8 bits and need to advance to the next byte
    if (stream->capa == 8) {
        stream->capa = 0;
        stream->ptr++;
    }
}

size_t pushbits(BitStream* curr, uchar src, size_t nbit) {
    size_t nbitwritten = 0;

    // Write remaining data bits
    size_t remaining_bits = nbit;
    while (remaining_bits > 0) {
        size_t bits_to_write = remaining_bits;
        if (bits_to_write > 8 - curr->capa) {
            bits_to_write = 8 - curr->capa;
        }

        for (size_t i = 0; i < bits_to_write; i++) {
            check(curr);
            int bit = (src >> (remaining_bits - 1 - i)) & 1;
            setbit(curr->ptr, curr->capa, bit);
            curr->capa++;
        }

        remaining_bits -= bits_to_write;
        check(curr);
    }

    return nbitwritten + nbit;
}

size_t pullbits(BitStream* curr, uchar* dest, size_t nbit) {
    *dest = 0;
    
    // Read data bits
    size_t remaining_bits = nbit;
    while (remaining_bits > 0) {
        size_t bits_to_read = (remaining_bits < 8 - curr->capa) ? 
                                remaining_bits : (8 - curr->capa);

        for (size_t i = 0; i < bits_to_read; i++) {
            int bit = getbit(*curr->ptr, 7 - curr->capa);
            *dest = (*dest << 1) | bit;
            curr->capa++;
            check(curr);
        }

        remaining_bits -= bits_to_read;
    }

    return nbit;
}

int encode(uchar* dest, uchar* src, WriteLog* log, int logSize) {
    BitStream stream = {dest, 0};  // Initialize the bitstream
    size_t totalBitsWritten = 0;

    for (int i = 0; i < logSize; i++) {
        uchar value = src[log[i].index];
        size_t nbits;

        // Determine the number of bits to write
        if (log[i].type == 'm') {
            nbits = 8;  // m requires 8 bits
        } else if (log[i].type == 'e') {
            nbits = 2;  // e requires 2 bits
        } else if (log[i].type == 'u') {
            nbits = 1;  // u requires 1 bit
        } else {
            continue;  // Invalid type, skip
        }

        // Push the bits into the bitstream
        totalBitsWritten += pushbits(&stream, value, nbits);
    }

    return totalBitsWritten;
}
