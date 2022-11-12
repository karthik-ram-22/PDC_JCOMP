

#ifndef AES_ERRORS
#define AES_ERRORS

#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

enum
{
	NO_ERRORS = 0,
	CANNOT_SAVE,
	FILE_NOT_FOUND
};

class Error
{
public:
	static int Throw(int error_id = -2, bool fatal = true, string error_desc = "")
	{
		switch (error_id)
		{
		case FILE_NOT_FOUND:
			cout << "File not found" << endl;
			break;

		case CANNOT_SAVE:
			cout << "Unable to save file. Check that it is not in use or protected" << endl;
			break;

		default:
			cout << "Undefined error" << endl;
			break;
		}
		if (fatal == true)
			exit(error_id);
		else
			return (error_id);
	}
};

#endif
