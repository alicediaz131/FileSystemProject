class FileSystem {

	void create(char * fname) { //check if file exists, if not: find a free descriptor, then make a file entry into the directory.

		//use seek starting from position 0, and repeat until EOF is reached. if a file descriptor has the same exact name, return an error: duplicate name
		//find a free file descriptor index i from D[1] to D[k-1]. if no free one exit with error: too many files
		//if you find a file descriptor with a -1, change it to 0 (because a new empty file uses no space)
		//now that you've found a file, you have to find a free entry in the directory where we can record the new file.
		//if the name field is 0, overwrite it with fname, overwrite index field with i, exit with success: file name created
		//if EOF is reached, exit with error: no free directory entry found
	}

	void destroy(File) {

	}
};