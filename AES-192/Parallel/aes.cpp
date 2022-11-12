
#include <iostream>
#include <omp.h>
#include <sys/time.h>
#include "aes_errors.hpp"
#include "aes_types.hpp"
#include "aes_msg.hpp"
#include "aes_globals.hpp"

using namespace std;

// Macro that gets the product of {02} and the input argument in module {1b}
#define xtime(x) ((x << 1) ^ (((x >> 7) & 1) * 0x1b))

// Tabla S-Box
int getSBoxValue(unsigned char num)
{
	int sbox[256] = {
		// 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,	 // 0
		0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,	 // 1
		0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,	 // 2
		0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,	 // 3
		0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,	 // 4
		0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,	 // 5
		0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,	 // 6
		0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,	 // 7
		0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,	 // 8
		0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,	 // 9
		0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,	 // A
		0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,	 // B
		0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,	 // C
		0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,	 // D
		0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,	 // E
		0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}; // F
	return sbox[(int)num];
}

// Table for round constant, array starts at 1 instead of 0
int Rcon[255] = {
	// 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
	0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
	0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
	0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
	0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
	0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
	0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
	0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
	0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
	0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
	0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
	0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
	0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
	0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
	0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
	0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb};

// This function generates the round keys from the initial key.
void keyExpansion()
{
	int i, j;
	unsigned char temp[4], k;

	// The first subkey is the initial key
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			round_key[i][j] = key[i][j];
		}
	}

	// We generate the rest of round subkeys
	while (j < (Nr * 4))
	{

		// We copy the previous column
		for (i = 0; i < 4; i++)
		{
			temp[i] = round_key[i][j - 1];
		}

		if (i % Nk == 0)
		{
			// column rotword
			k = temp[0];
			temp[0] = temp[1];
			temp[1] = temp[2];
			temp[2] = temp[3];
			temp[3] = k;

			// Column SubBytes
			temp[0] = getSBoxValue(temp[0]);
			temp[1] = getSBoxValue(temp[1]);
			temp[2] = getSBoxValue(temp[2]);
			temp[3] = getSBoxValue(temp[3]);

			// Current Column XOR Round Constant
			temp[0] = temp[0] ^ Rcon[i / Nk];
		}

		else if (Nk > 6 && i % Nk == 4)
		{
			// SubBytes to Column
			temp[0] = getSBoxValue(temp[0]);
			temp[1] = getSBoxValue(temp[1]);
			temp[2] = getSBoxValue(temp[2]);
			temp[3] = getSBoxValue(temp[3]);
		}
		round_key[0][j] = round_key[0][j - Nk] ^ temp[0];
		round_key[1][j] = round_key[1][j - Nk] ^ temp[1];
		round_key[2][j] = round_key[2][j - Nk] ^ temp[2];
		round_key[3][j] = round_key[3][j - Nk] ^ temp[3];
		j++;
	}
}

// AddRoundKey function of the AES standard
void AddRoundKey(int round)
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			state[i][j] ^= round_key[i][round * 4 + j];
		}
	}
}

// AddRoundKey function with block parameter of the AES standard
void AddRoundKey(int round, unsigned char block[4][4])
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			block[i][j] ^= round_key[i][round * 4 + j];
		}
	}
}

// SubBytes function of the AES standard
void SubBytes()
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			state[i][j] = getSBoxValue(state[i][j]);
		}
	}
}

// SubBytes function with block parameter of the AES standard
void SubBytes(unsigned char block[4][4])
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			block[i][j] = getSBoxValue(block[i][j]);
		}
	}
}

// ShiftRows function of the AES standard
void ShiftRows()
{
	unsigned char temp;

	// Rotate first row 1 columns to left
	temp = state[1][0];
	state[1][0] = state[1][1];
	state[1][1] = state[1][2];
	state[1][2] = state[1][3];
	state[1][3] = temp;

	// Rotate second row 2 columns to left
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;

	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;

	// Rotate third row 3 columns to left
	temp = state[3][0];
	state[3][0] = state[3][3];
	state[3][3] = state[3][2];
	state[3][2] = state[3][1];
	state[3][1] = temp;
}

// ShiftRows function with block parameter of the AES standard
void ShiftRows(unsigned char block[4][4])
{
	unsigned char temp;

	// Rotate first row 1 columns to left
	temp = block[1][0];
	block[1][0] = block[1][1];
	block[1][1] = block[1][2];
	block[1][2] = block[1][3];
	block[1][3] = temp;

	// Rotate second row 2 columns to left
	temp = block[2][0];
	block[2][0] = block[2][2];
	block[2][2] = temp;

	temp = block[2][1];
	block[2][1] = block[2][3];
	block[2][3] = temp;

	// Rotate third row 3 columns to left
	temp = block[3][0];
	block[3][0] = block[3][3];
	block[3][3] = block[3][2];
	block[3][2] = block[3][1];
	block[3][1] = temp;
}

// MixColumns function of the AES standard
void MixColumns()
{
	int i;
	unsigned char Tmp, Tm, t;
	for (i = 0; i < 4; i++)
	{
		t = state[0][i];
		Tmp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
		Tm = state[0][i] ^ state[1][i];
		Tm = xtime(Tm);
		state[0][i] ^= Tm ^ Tmp;
		Tm = state[1][i] ^ state[2][i];
		Tm = xtime(Tm);
		state[1][i] ^= Tm ^ Tmp;
		Tm = state[2][i] ^ state[3][i];
		Tm = xtime(Tm);
		state[2][i] ^= Tm ^ Tmp;
		Tm = state[3][i] ^ t;
		Tm = xtime(Tm);
		state[3][i] ^= Tm ^ Tmp;
	}
}

// MixColumns function with block parameter of the AES standard
void MixColumns(unsigned char block[4][4])
{
	int i;
	unsigned char Tmp, Tm, t;
	for (i = 0; i < 4; i++)
	{
		t = block[0][i];
		Tmp = block[0][i] ^ block[1][i] ^ block[2][i] ^ block[3][i];
		Tm = block[0][i] ^ block[1][i];
		Tm = xtime(Tm);
		block[0][i] ^= Tm ^ Tmp;
		Tm = block[1][i] ^ block[2][i];
		Tm = xtime(Tm);
		block[1][i] ^= Tm ^ Tmp;
		Tm = block[2][i] ^ block[3][i];
		Tm = xtime(Tm);
		block[2][i] ^= Tm ^ Tmp;
		Tm = block[3][i] ^ t;
		Tm = xtime(Tm);
		block[3][i] ^= Tm ^ Tmp;
	}
}

void cipher(unsigned char block[4][4])
{
	// We assume key length of 128 bits
	Nr = 192;
	// We calculate the number of rounds and the 32-bit words of the key.
	Nk = Nr / 32;
	Nr = Nk + 6;

	// First AddRoundKey, before the rounds
	AddRoundKey(0, block);

	// The first Nr-1 rounds are identical
	for (int i = 1; i < Nr; i++)
	{
		SubBytes(block);
		ShiftRows(block);
		MixColumns(block);
		AddRoundKey(i, block);
	}

	// The last round does not have the MixColumns function
	SubBytes(block);
	ShiftRows(block);
	AddRoundKey(Nr, block);
}

// Load the nbloq block into the state array and return true if
// has reached the end of the file or false otherwise
bool carry_block(Msg &archivo, unsigned char block[4][4], int nblock)
{
	int bytes_count = nblock * 16;
	bool end = false;

	// We load the bits into the array
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (bytes_count < archivo.GetSize())
			{
				block[i][j] = archivo.GetByte(bytes_count);
				bytes_count++;
			}
			else
			{
				block[i][j] = 0x00;
				end = true;
			}
			cout<<block[i][j]<<" ";
		}
		cout<<"\n";
	}
	return end;
}

// Save the nbloq block in the cipher array
void save_block(unsigned char *dest, unsigned char block[4][4], int nblock)
{
	int byte_inic = nblock * 16;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			dest[byte_inic] = block[i][j];
			byte_inic++;
			cout << endl<< block[i][j];
		}
	}
}

void show_hex(unsigned char *ex, int t1)
{
	for (int i = 0; i < t1; i++)
	{
		printf("%x ", ex[i]);
	}
}

int main(int argc, char **argv)
{
	struct timeval begin, end;
    	gettimeofday(&begin, 0);
	Msg key_file;
	Msg plain_file;
	Msg output_file;
	string output;
	unsigned char *ex;
	int i, j, k, tid, tids, t1, block_count, block_max, nthreads;
	cout << "Number of Arguements: " << argc << endl;
	if (argc != 5)
	{
		cout << "Enter the file name to be encrypted: ";
		cin >> plain_filename;
		cout << "enter the name of key file: ";
		cin >> key_filename;
		cout << "enter the name of output file: ";
		cin >> output_filename;
		cout << "Enter the number of execution threads: (<= 0 -> Automatic): ";
		cin >> nthreads;
	}
	else
	{
		plain_filename = argv[1];
		key_filename = argv[2];
		output_filename = argv[3];
		nthreads = atoi(argv[4]);
	}

	// We assume key length of 192 bits
	Nr = 192;
	// We calculate the number of rounds and the 32-bit words of the key.
	Nk = Nr / 32;
	Nr = Nk + 6;

	// We open the file to encrypt
	plain_file.LoadFromFile(plain_filename);
	// We open the key
	key_file.LoadFromFile(key_filename);
	// We copy the file to encrypt as output
	output_file.LoadFromFile(plain_filename);

	// We set the number of threads
	if (nthreads > 0)
	{
		omp_set_num_threads(nthreads);
	}

	// We load the key into the array
	k = 0;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			key[i][j] = key_file.GetByte(k);
			k++;
		}
	}

	// We reserve space for the output stream
	t1 = ((plain_file.GetSize() / 24) + 1) * 24;
	ex = new unsigned char[t1];

	// We expand the key to obtain the subkeys of each round
	keyExpansion();

	// block counter
	block_max = t1 / 24;

#pragma omp parallel private(state, block_count, tid, tids) shared(ex, plain_file, block_max)
	{
		block_count = tid = omp_get_thread_num();
		tids = omp_get_num_threads();

		if (tid == 0)
		{
			cout << "Running the encryptor with " << tids << " threads" << endl;
		}
		while (block_count < block_max)
		{
			carry_block(plain_file, state, block_count);
			cipher(state);
			save_block(ex, state, block_count);
			block_count += tids;
		}
	}

	cout << "Saving File from " << t1 << " bytes..." << endl;
	// If you uncomment the line below, you can see the encryption result on the screen
	//show_hex(output,size);
	output_file.SetMessage(ex, t1);
	output_file.SaveToFile(output_filename);
	gettimeofday(&end, 0);
    	long seconds = end.tv_sec - begin.tv_sec;
    	long microseconds = end.tv_usec - begin.tv_usec;
    	double elapsed = seconds + microseconds*1e-6;
	printf("Time measured: %.3f seconds.\n", elapsed);
	return 0;
}
