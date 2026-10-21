//#include "FileSystem.cpp"
#ifndef SIZECONSTS
#define SIZECONSTS
static const int B = 64;
static const int bytes = 512;
static const int K = 7;
static const int DIR_BASE = 256;
static const int FIO_BASE = 384;
#endif

#ifndef FSYS_H
#define FSYS_H

#include <cstring>
#include <fstream>
#include <iostream>
//k = ^d/32^ + 1.

extern std::byte D[B][bytes];

class OFTEntry {
	public:
	std::byte RWBuffer[bytes]; //can have up to 512 bytes in a single entry.
	int position; //ranges from -1 or 0 to 1536 (file size is 3 blocks)
	int fileSize;
	int FDIndex; //index
	
	OFTEntry();
	void print(){
		std::cout << "Position: " << position;
		std::cout << "\nFile Size: " << fileSize;
		std::cout << "\nFile Descriptor Index: " << FDIndex << std::endl;
	}
	void setEntry(int pos, int size, int index){
		position = pos;
		fileSize = size;
		FDIndex = index;
	}

};

class FileSystem {
	public:
	FileSystem();
	
	void create(std::string fname);
	void destroy(std::string fname);
	void open(std::string fname);
	void close(int OFTIndex);
	void read(int openFileIndex, int position, int n);
	void write(int openFileIndex, int position, int n);
	void seek(int openFileIndex, int p);
	void readWithOutput(int openFileIndex, int position, int n);
	void writeWithOutput(int openFileIndex, int position, int n);
	void seekWithOutput(int openFileIndex, int p);
	void directory();
	void init();
	std::byte I[bytes]; //holds what a user wants to type into the buffer stored in Memory
	std::byte O[bytes]; //holds what a user wants to read from the buffer stored in Memory
	std::byte M[bytes]; //holds the current block from Disk that's being read from or written to
	std::ofstream outputFile;
	OFTEntry OFT [4];
	void printBlock(std::byte * block);
	int findFreeOFTEntry();
	int findDirectoryEntry(std::string fname);
	int findFreeDirectoryEntry();
	void findFreeBlock(int FDIndex, int newBlockCount);
	void copyBlock(std::byte * src, std::byte * dest);
	void writeMemory(int m, std::string s);
	void readMemory(int m, int n);
	int findFreeFD();
	void printFileDescriptors();
	void freeFileDescriptor(int index);
	void setBitmap(int index, int value);
	void read_block(int block, std::byte *buf);
	void write_block(int block, std::byte *buf);

	/*template <typename T>
	void writeBytes(size_t offset, const T& value) {
		std::memcpy(&D[offset], &value, sizeof(T));
	}


	template <typename T> 
	T readBytes(size_t offset) const {
		T value;
		std::memcpy(&value, &D[offset], sizeof(T));
		return value;
	}*/

	template <typename T>
	T readBytes(std::size_t block, std::size_t offset) {
		T data;
		std::memcpy(&data, &D[block][offset], sizeof(data));
		return data;
	}
	template <typename T>
	void writeBytes(std::size_t block, std::size_t offset, const T& value){
		std::memcpy(&D[block][offset], &value, sizeof(T));
	}

	template <typename T>
	static T readBytesIO(std::byte * block, std::size_t offset) {
		T data;
		std::memcpy(&data, &block[offset], sizeof(data));
		return data;
	}
	template <typename T>
	static void writeBytesIO(std::byte * block, std::size_t offset, const T& value){
		std::memcpy(&block[offset], &value, sizeof(value));
	}
	template <typename T>
	void printBlock(std::byte * block) {
		for(std::size_t i = 0; i < bytes; i = i+sizeof(T)) {
			std::cout << std::endl << readBytesIO<T>(block, i);//static_cast<int>(block[i]);
		}
	}

	template <typename T>
	void printBlock(std::byte * block, int n) {
		for(std::size_t i = 0; i < n; i = i+sizeof(T)) {
			std::cout << std::endl << readBytesIO<T>(block, i);//static_cast<int>(block[i]);
		}
	}

};


/*
class DirectoryEntry {
	char * symbolicName[4]; //a file name that starts with 0 means the file is free
	int * fdIndex; //in range 1 to d-1, used to index into file descriptors kept in D[1] through D[k-1]
	//directory is implemented as just a single ordinary file. files are only able to have 3 blocks of 512 bytes. ints are 4 bytes, and 4 char is 4 bytes, so 3*512/8 = 192 files max
};



*/



#endif