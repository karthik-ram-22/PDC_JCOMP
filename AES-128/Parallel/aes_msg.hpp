
#ifndef AES_MSG
#define AES_MSG

#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "aes_errors.hpp"
#include "aes_types.hpp"

using namespace std;

class Msg
{
public:
	Msg();
	~Msg();

	int LoadFromFile(string, bool);
	int SaveToFile(bool);
	int SaveToFile(string, bool);
	void SetMessage(unsigned char *, int);
	int GetSize();
	unsigned char GetByte(int);
	int GetBit(int);

private:
	unsigned char *msg_datastream;
	string msg_filename;
	int msg_size;
};

Msg::Msg()
{
	msg_size = -1;
	msg_filename = "";
	msg_datastream = NULL;
}

Msg::~Msg()
{
	if (msg_datastream != NULL)
	{
		delete[] msg_datastream;
	}
}

int Msg::LoadFromFile(string filename, bool critical = true)
{
	// We open the file
	ifstream file(filename.c_str(), ios::binary);
	if (!file.is_open())
	{
		Error::Throw(FILE_NOT_FOUND, critical);
		return -1;
	}

	// We save the name of the file
	msg_filename = filename;
	file.seekg(0, ios::end); // Go to the end of the file
	msg_size = file.tellg(); // See position to know size
	file.seekg(0, ios::beg);

	// We size the msg_datastream to save the data stream
	if (msg_datastream != NULL)
		delete[] msg_datastream;

	msg_datastream = new unsigned char[msg_size + msg_final_mark.length()];
	// We read the file and save it in msg_datastream
	file.read((char *)msg_datastream, msg_size);

	// Add end of file mark
	for (int i = 0; i < (int)msg_final_mark.length(); i++)
	{
		msg_datastream[msg_size + i] = msg_final_mark[i];
	}
	// We update the size of the message
	msg_size += msg_final_mark.length();

	file.close();

	cout << "File " << msg_filename << " saved size " << msg_size << " bytes" << endl;
	return 0;
}

int Msg::SaveToFile(bool critical = false)
{
	if (msg_filename == "")
		msg_filename = "ciphered.txt";
	ofstream file(msg_filename.c_str(), ios::binary);
	if (!file)
	{
		Error::Throw(CANNOT_SAVE, critical);
		return -1;
	}
	file.write((char *)msg_datastream, msg_size);
	file.close();

	cout << "File " << msg_filename << " saved size " << msg_size << " bytes" << endl;

	return 0;
}

int Msg::SaveToFile(string filename, bool critical = false)
{
	msg_filename = filename;

	ofstream file(msg_filename.c_str(), ios::binary);
	if (!file)
	{
		Error::Throw(CANNOT_SAVE, critical);
		return -1;
	}
	file.write((char *)msg_datastream, msg_size);
	file.close();

	cout << "File " << msg_filename << " saved size: " << msg_size << " bytes" << endl;
	return 0;
}

int Msg::GetBit(int pos)
{
	int byte_pos;
	unsigned char byte_value;
	int bit_pos;
	int bit_value[8];
	int i, exp;

	// We position ourselves on the correct byte and on the correct bit within that byte
	byte_pos = (int)pos / 8;
	bit_pos = (int)pos % 8;
	byte_value = msg_datastream[byte_pos];

	// We convert the corresponding byte to a sequence of 8 bits and store them in the vector bitvalue
	for (i = 7; i >= 0; i--)
	{
		exp = (int)pow(2.0, i);
		if (byte_value >= exp)
		{
			bit_value[i] = 1;
			byte_value -= exp;
		}
		else
		{
			bit_value[i] = 0;
		}
	}
	return bit_value[bit_pos];
}

unsigned char
Msg::GetByte(int pos)
{
	return msg_datastream[pos];
}

int Msg::GetSize()
{
	return msg_size;
}

void Msg::SetMessage(unsigned char *data, int tam)
{
	// If the stream was busy we delete it
	if (msg_datastream != NULL)
		delete[] msg_datastream;

	// We copy the bytes to the stream
	msg_datastream = new unsigned char[tam];
	memcpy(msg_datastream, data, tam);
	// We update the msg_size
	msg_size = tam;
}

#endif
