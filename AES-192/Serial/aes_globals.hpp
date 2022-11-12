
#include "aes_types.hpp"
#include "aes_msg.hpp"
#include <string>

string plain_filename;
string output_filename;
string key_filename;

// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

Msg plain;
// Store the key
unsigned char key[4][4];
// Stores a block of the flat file
unsigned char block[4][4];
// Store round keys
unsigned char round_key[4][60];
// Number of rounds of the cipher. It is initialized to zero.
int Nr = 0;
// Number of 32-bit words of the key. It is initialized to zero.
int Nk = 0;

// plain - 128-bit array with plain text.
// key - 128-bit array with the key.
// state - 128-bit array for intermediate states.
unsigned char in[16];
unsigned char out[16];
unsigned char state[4][4];
