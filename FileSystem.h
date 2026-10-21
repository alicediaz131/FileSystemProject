#include "FileSystem.cpp"
static const int B = 64;
static const int bytes = 512;
//k = ^d/32^ + 1.

class FileSystem {

	int D[B][bytes];
	void create();
	void destroy();
	void open();
	void close();
	void read();
	void write();
	void seek();
	void directory();

};
