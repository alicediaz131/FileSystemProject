#include "FileDescriptor.h"
class Directory {
	
};


class DirectoryEntry {
	char symbolicName[4]; //a file name that starts with 0 means the file is free
	int fdIndex; //in range 1 to d-1, used to index into file descriptors kept in D[1] through D[k-1]
	//directory is implemented as just a single ordinary file. files are only able to have 3 blocks of 512 bytes. ints are 4 bytes, and 4 char is 4 bytes, so 3*512/8 = 192 files max
};