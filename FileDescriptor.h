#include <iostream>
#include <string>


class FileDescriptor {
	
	int blocks[4]; //first int is the file size (-1 means it's free). 1,2,3 are locations of the blocks on the disk.
};