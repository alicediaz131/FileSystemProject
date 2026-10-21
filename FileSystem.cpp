#include "FileSystem.h"

std::byte D[B][bytes];


	FileSystem::FileSystem() {
		init();
	}

	void FileSystem::create(std::string fname) { //check if file exists, if not: find a free descriptor, then make a file entry into the directory.
		int directoryIndex = findDirectoryEntry(fname);
		if (directoryIndex != -1){
			//std::cout << "error: duplicate name\n";
			outputFile << "error\n";
			return;
		}
		int FDIndex = findFreeFD();
		if (FDIndex == -1){
			//std::cout << "error: too many files\n";
			outputFile << "error\n";
			return;
		}
		writeBytes(1, FDIndex, 0);
		directoryIndex = findFreeDirectoryEntry();
		if (directoryIndex == -1){
			//std::cout << "error: no free directory entry found\n";
			outputFile << "error";
			return;
		}
		seek(0, directoryIndex);
		//writeMemory(FIO_BASE, "    ");
		writeBytesIO(M, FIO_BASE, 0);
		writeMemory(FIO_BASE, fname);
		writeBytesIO(M, FIO_BASE+4, FDIndex);
		write(0, FIO_BASE, 8);
		//std::cout << "success: file " << fname << " created\n";

		
		outputFile << fname << " created\n";
	}

	void FileSystem::destroy(std::string fname) { //check if file exists, if not: find a free descriptor, then make a file entry into the directory.
		int directoryIndex = findDirectoryEntry(fname);
		if (directoryIndex == -1){
			//std::cout << "in destroy fail";
			//std::cout << "error: file does not exist\n";
			outputFile << "error\n";
			return;
		}
		seek(0, directoryIndex+4);	
		read(0, FIO_BASE, 4);
		int FDIndex = readBytesIO<int>(&M[0], FIO_BASE);
		for(int i = 0; i < 4; ++i){
			if(OFT[i].FDIndex == FDIndex){
				//std::cout << "error: cannot destroy an open file\n FDIndex == " << FDIndex << "\n";
				outputFile << "error\n";
				return;
			}
		}
		
		seek(0, directoryIndex);
		for (int i = 0; i < 4; ++i)
			writeBytesIO(M, FIO_BASE+i, '\0');
		writeBytesIO(M, FIO_BASE+4, 0);
		write(0, FIO_BASE, 8);
		freeFileDescriptor(FDIndex);
		//std::cout << "sucess: file " << fname << " destroyed\n";
		outputFile << fname << " destroyed\n";
	}

	
	void FileSystem::seekWithOutput(int openFileIndex, int p){
		if (p < 0 || p > OFT[openFileIndex].fileSize) {
			//std::cout << "error: current position past end of file\n";
			outputFile << "error\n";
			return;
		}

		int oldBlock = OFT[openFileIndex].position / bytes;
		int newBlock = p / bytes;


		if (oldBlock != newBlock && OFT[openFileIndex].position >= 0) {
			int oldDiskBlock = readBytes<int>(1 + (OFT[openFileIndex].FDIndex % bytes), OFT[openFileIndex].FDIndex+oldBlock+4);
			
			if (oldDiskBlock != 0 && OFT[openFileIndex].position < OFT[openFileIndex].fileSize) {
				write_block(oldDiskBlock, OFT[openFileIndex].RWBuffer);
			}
			int newDiskBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * newBlock);

			if (newDiskBlock != 0)
				//copyBlock(&D[newDiskBlock][0], OFT[openFileIndex].RWBuffer);
				read_block(newDiskBlock, OFT[openFileIndex].RWBuffer);
			else 
				std::memset(OFT[openFileIndex].RWBuffer, 0, bytes);
			
		}
		OFT[openFileIndex].position = p;
		outputFile << "position is " << p << std::endl;
	}
	
	void FileSystem::seek(int openFileIndex, int p) {
		if (p < 0 || p > OFT[openFileIndex].fileSize) {
			return;
		}

		int oldBlock = OFT[openFileIndex].position / bytes;
		int newBlock = p / bytes;


		if (oldBlock != newBlock && OFT[openFileIndex].position >= 0) {
			int oldDiskBlock = readBytes<int>(1 + (OFT[openFileIndex].FDIndex % bytes), OFT[openFileIndex].FDIndex+oldBlock+4);
			
			if (oldDiskBlock != 0 && OFT[openFileIndex].position < OFT[openFileIndex].fileSize) {
				write_block(oldDiskBlock, OFT[openFileIndex].RWBuffer);
			}
			int newDiskBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * newBlock);

			if (newDiskBlock != 0)
				read_block(newDiskBlock, OFT[openFileIndex].RWBuffer);
			else 
				std::memset(OFT[openFileIndex].RWBuffer, 0, bytes);
			
		}
		OFT[openFileIndex].position = p;

	}
	

	void FileSystem::open(std::string fname) {
		
		int dir = findDirectoryEntry(fname);
		if (dir == -1) {
			//std::cout << "\nerror: file does not exist\n";
			outputFile << "error\n";
			return;
		}
		seek(0, dir + 4);
		read(0, FIO_BASE, 4);
		int FDIndex = readBytesIO<int>(M, FIO_BASE);
		for (int i = 0; i < 4; ++i){
			if(OFT[i].FDIndex == FDIndex){
				outputFile << "error\n";
				return;
			}
		}
		int OFTIndex = findFreeOFTEntry();
		if (OFTIndex == -1){
			//std::cout << "\nerror: too many files open\n";
			outputFile << "error\n";
			return;
		}
		OFT[OFTIndex].position = 0;
		OFT[OFTIndex].fileSize = readBytes<int>(1, FDIndex);
		OFT[OFTIndex].FDIndex = FDIndex;

		if(OFT[OFTIndex].fileSize == -1){
			findFreeBlock(FDIndex, 0);
			OFT[OFTIndex].fileSize = 0;
			writeBytes(1, FDIndex, 0);
		}
		else
		{
			int firstBlock = readBytes<int>(1, FDIndex + 4);
			if (firstBlock != 0)
    			read_block(firstBlock, OFT[OFTIndex].RWBuffer);
			else
    			std::memset(OFT[OFTIndex].RWBuffer, 0, bytes);
		}
		//std::cout << "success: file " << fname << " opened at index " << OFTIndex << std::endl;
		outputFile << fname << " opened " << OFTIndex << std::endl;
	}

	void FileSystem::close(int OFTIndex){
		if (OFTIndex == 0 || OFT[OFTIndex].FDIndex == -1){
			//std::cout << "cannot close directory";
			outputFile << "error\n";
			return;
		}
		int blockIdx = (OFT[OFTIndex].position) / bytes;
		int diskBlock = readBytes<int>(1, OFT[OFTIndex].FDIndex + 4 + 4 * blockIdx);

		if (diskBlock != 0) {
			write_block(diskBlock, OFT[OFTIndex].RWBuffer);
		}
		writeBytesIO(D[1], OFT[OFTIndex].FDIndex, OFT[OFTIndex].fileSize);
		OFT[OFTIndex].setEntry(-1, -1, -1);
		//std::cout << "success: file " << OFTIndex << " closed\n";
		outputFile << OFTIndex << " closed\n";
	}

	void FileSystem::read(int openFileIndex, int m, int n) {
		int offset;
		
		if(OFT[openFileIndex].FDIndex == -1){
			//std::cout << "in read error\n";
			return;
		}
			
		for(std::size_t i = 0; i < n; ++i){
			offset = OFT[openFileIndex].position % bytes;
			if (OFT[openFileIndex].position >= OFT[openFileIndex].fileSize){
				//std::cout << "error: reading past end of file\n"; //error: current position past end of file
				//outputFile << "error\n";
				return;

			}

			if (offset == 0 && OFT[openFileIndex].position != 0) {
				int oldBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * ((OFT[openFileIndex].position-1) / bytes));
    			write_block(oldBlock, OFT[openFileIndex].RWBuffer);
				int newBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + (4 * OFT[openFileIndex].position / bytes));
				read_block(newBlock, OFT[openFileIndex].RWBuffer);
        	}

			M[m+i] = OFT[openFileIndex].RWBuffer[offset]; //copy the byte from RWbuffer into memory.
			++OFT[openFileIndex].position;
		}
	}

	void FileSystem::readWithOutput(int openFileIndex, int m, int n) {
		int offset;
		if(OFT[openFileIndex].FDIndex == -1){
			//std::cout << "in read error\n";	
			outputFile << "error\n";
			return;
		}
		for(std::size_t i = 0; i < n; ++i){
			offset = OFT[openFileIndex].position % bytes;

			if (OFT[openFileIndex].position >= OFT[openFileIndex].fileSize){
				outputFile << i << " bytes read from file " << openFileIndex << std::endl;
				return;
			}
			if (offset == 0 && OFT[openFileIndex].position != 0) {
				int oldBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * ((OFT[openFileIndex].position-1) / bytes));
    			write_block(oldBlock, OFT[openFileIndex].RWBuffer);
				int newBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + (4 * OFT[openFileIndex].position / bytes));
				read_block(newBlock, OFT[openFileIndex].RWBuffer);
        	}

			M[m+i] = OFT[openFileIndex].RWBuffer[offset]; //copy the byte from RWbuffer into memory.
			++OFT[openFileIndex].position;	
		}
		outputFile << n << " bytes read from file " << openFileIndex << std::endl;
	}

	void FileSystem::read_block(int block, std::byte *buf){
		copyBlock(D[block], buf);
	}

	void FileSystem::write_block(int block, std::byte *buf){
		copyBlock(buf, D[block]);
	}

	void FileSystem::write(int openFileIndex, int m, int n) {

		//int offset;
		for(std::size_t i = 0; i < n; ++i){
			int position = OFT[openFileIndex].position;
			int offset = position % bytes;
			int blockIdx = position / bytes;

			if(position >= 3 * bytes){
				//std::cout << "error: maximum file size reached.\n";
				return;
			}
			
			if (offset == 0) {
            	if (position != 0) { //allocate new block
					int oldBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * (blockIdx - 1));

					write_block(oldBlock, OFT[openFileIndex].RWBuffer);
				}
				int newBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * blockIdx);

				if (newBlock == 0){
					findFreeBlock(OFT[openFileIndex].FDIndex, blockIdx);
					std::memset(OFT[openFileIndex].RWBuffer, 0, bytes);
				}
			}
			
			OFT[openFileIndex].RWBuffer[offset] = M[m+i]; //copy the byte from RWbuffer into memory.

			if (OFT[openFileIndex].fileSize < position + 1){
				++OFT[openFileIndex].fileSize;
				//update filesize in file descriptor
				writeBytes(1, OFT[openFileIndex].FDIndex, OFT[openFileIndex].fileSize);
			}
			++OFT[openFileIndex].position;
			

		}
		
	} 

	void FileSystem::writeWithOutput(int openFileIndex, int m, int n) {

		//int offset;
		for(std::size_t i = 0; i < n; ++i){
			int position = OFT[openFileIndex].position;
			int offset = position % bytes;
			int blockIdx = position / bytes;

			if(position >= 3 * bytes){
				//std::cout << "error: maximum file size reached.\n";
				//outputFile << "error\n";
				outputFile << i << " bytes written to file " << openFileIndex << std::endl;
				return;
			}
			
			if (offset == 0) {
            	if (position != 0) { //allocate new block
					int oldBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * (blockIdx - 1));
					write_block(oldBlock, OFT[openFileIndex].RWBuffer);
				}
				int newBlock = readBytes<int>(1, OFT[openFileIndex].FDIndex + 4 + 4 * blockIdx);

				if (newBlock == 0)
					findFreeBlock(OFT[openFileIndex].FDIndex, blockIdx);
				
				std::memset(OFT[openFileIndex].RWBuffer, 0, bytes);
			}
			
			OFT[openFileIndex].RWBuffer[offset] = M[m+i]; //copy the byte from RWbuffer into memory.

			if (OFT[openFileIndex].fileSize < position + 1){
				++OFT[openFileIndex].fileSize;
				//update filesize in file descriptor
				writeBytes(1, OFT[openFileIndex].FDIndex, OFT[openFileIndex].fileSize);
			}
			++OFT[openFileIndex].position;
		}
		outputFile << n << " bytes written to file " << openFileIndex << std::endl;
	} 

	void FileSystem::writeMemory(int m, std::string s){
		std::size_t slen = s.length();
		for(std::size_t i = 0; i < slen && m < bytes; ++i){
			writeBytesIO(&M[0], m, s[i]);
			++m;
		}
	}



	void FileSystem::readMemory(int m, int n){
		for(std::size_t i = 0; i < n && m < bytes; ++i){
			outputFile << readBytesIO<char>(&M[0], m);
			++m;
		}
		outputFile << std::endl;
	}



	void FileSystem::init() {
		std::byte emptyChar;
		std::memset(&emptyChar, '\0', 1);
		for (std::size_t i = 0; i < B; ++i) {
			for (std::size_t j = 0; j < bytes; ++j) {
				D[i][j] = emptyChar;
			}
		}
		for (std::size_t i = 0; i < bytes; ++i) {
				M[i] = emptyChar;
				I[i] = emptyChar;
				O[i] = emptyChar;
				for (std::size_t j = 0; j < 4; ++j)
					OFT[j].RWBuffer[i] = emptyChar;
		}
		
		for (std::size_t i = 0; i < 4; ++i) {
			OFT[i].setEntry(-1,-1,-1);
		}

		for (std::size_t i = 0; i < 32; i = i + 4) {
			writeBytes(0, i, 1);
		}
		//set all remaining entries in the bytearray to 0
		for (std::size_t i = 32; i < B*4; i = i + 4) {
			writeBytes(0, i, 0);
		}

		for (std::size_t i = 0; i < bytes; i = i+16) {
			for (std::size_t j = 1; j < K; ++j) {
				writeBytes(j, i, -1);
			}
		}
		writeBytes(1, 0, 0);
		writeBytes(1, 4, 7);


		writeBytesIO(OFT[0].RWBuffer, 0, 'D');
		writeBytesIO(OFT[0].RWBuffer, 1, 'i'); 
		writeBytesIO(OFT[0].RWBuffer, 2, 'r');
		writeBytesIO(OFT[0].RWBuffer, 3, '\0');
		writeBytesIO(OFT[0].RWBuffer, 4, 0);
		OFT[0].setEntry(0, bytes, 0);
		

	}

	int FileSystem::findFreeFD(){
		for (std::size_t i = 0; i < bytes*6; i = i + 16)
		{
			if (readBytes<int>(1, i) == -1)
				return i;
		}
		return -1;
	}

	void FileSystem::directory(){
		int savePos = OFT[0].position;
		for (std::size_t i = 8; i < bytes; i = i+8) {
			seek(0, i);
			read(0, DIR_BASE, 8);
			if (readBytesIO<char>(M, DIR_BASE) != 0)
			{
				//print file name and size
				char fname[4];
				for(std::size_t j = 0; j < 4; ++j ) {
					fname[j] = readBytesIO<char>(M, DIR_BASE+j);
				}
				outputFile << fname << " " << readBytes<int>(1, readBytesIO<int>(M, DIR_BASE+4)) << " ";
			}
		}
		outputFile << std::endl;
		OFT[0].position = savePos;
	}

	int FileSystem::findFreeOFTEntry(){
		for(std::size_t i = 0; i < 4; ++i){
			if (OFT[i].position == -1)
				return i;
		}
		return -1;
	}

	int FileSystem::findFreeDirectoryEntry() { 
		
		for (std::size_t i = 0; i < bytes; i = i+8) {
			
			int savePos = OFT[0].position;
			seek(0, i);
			read(0, DIR_BASE, 4);
			OFT[0].position = savePos;
			if (readBytesIO<char>(M, DIR_BASE) == 0)
			{
				return i;
			}
		}
		return -1;
	}
	int FileSystem::findDirectoryEntry(std::string fname) {
		
		for (std::size_t i = 0; i < OFT[0].fileSize; i = i+8) {
			seek(0, i);
			read(0, DIR_BASE, 4);
			if (readBytesIO<char>(M, DIR_BASE) != 0)
			{
				char dirName[4];
				for(std::size_t j = 0; j < 4; ++j ) {
					dirName[j] = readBytesIO<char>(M, DIR_BASE+j);
				}
				if (dirName == fname)
					return i;
			}
		}

		return -1;
	}

void FileSystem::findFreeBlock(int FDIndex, int newBlockCount){
	if (newBlockCount == 4){
		//std::cout << "error: maximum of 3 blocks\n";
		return;
	}
	for (std::size_t i = 4; i < B*4; i += 4){
		if (readBytes<int>(0, i) == 0){
			writeBytes(0, i, 1);
			writeBytes(1, FDIndex + (4* (1 + newBlockCount)), i/4);
			return;
		}
	}
	//std::cout << "failed to find free block\n";
	//printBlock<int>(&D[0][0], 64*4);
}

void FileSystem::freeFileDescriptor(int index){
	for(int i = 0; i < 16; i += 4){
		if (i > 0){
			int bitMapIndex = readBytesIO<int>(&D[1][0], index+i);
			setBitmap(bitMapIndex, 0);
		}
		writeBytes(1, index+i, 0);
	}	
		
}

void FileSystem::setBitmap(int index, int value){
	write(0, index*4, value);
}

void FileSystem::copyBlock(std::byte * src, std::byte * dest){
	for (std::size_t i = 0; i < bytes; ++i){
		dest[i] = src[i];
	}
}

void FileSystem::printFileDescriptors(){
	std::cout << "printing FDs\n";
	for(std::size_t i = 0; i < B; i += 4){
		std::cout << readBytes<int>(1, i) << std::endl;
	}
}

OFTEntry::OFTEntry(): position(-1){
	for (std::size_t i = 0; i < bytes; i += 4){
		FileSystem::writeBytesIO(&RWBuffer[0], i, 0);
	}
}