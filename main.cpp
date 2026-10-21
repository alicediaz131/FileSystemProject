#include "FileSystem.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <fstream>
using namespace std;


int main(int argc, char* argv[]) {
	

	if (argc < 2) {
        std::cout << "./main " << argv[0] << " <filename>" << std::endl;
        return 0;
    }
	
	string fname;
	fname = argv[1];
	ifstream inputFile(fname);
	

	if (!inputFile.is_open()) {
        std::cout << "error: could not open the file " << fname << std::endl;
        return 0;
    }
	FileSystem fs = FileSystem();
	fs.outputFile.open("output.txt");
	string input;
	while(getline(inputFile, input)){
		//getline(cin, input);
		string token;
		stringstream tokenStream(input);
		vector<string> tokens;

		while (tokenStream >> token) {
			tokens.push_back(token);
		}
		if (tokens.empty()){
			fs.outputFile << "\n";
			continue;
		}
		else if (tokens[0] == "cr")
			fs.create(tokens[1]);
		else if (tokens[0] == "de")
			fs.destroy(tokens[1]);
		else if (tokens[0] == "op")
			fs.open(tokens[1]);
		else if (tokens[0] == "cl"){
			int index = stoi(tokens[1]);
			fs.close(index);
		}
		else if (tokens[0] == "wm"){
			int m = stoi(tokens[1]);
			tokens.erase(tokens.begin());
			tokens.erase(tokens.begin());
			string memstr = tokens[0];
			tokens.erase(tokens.begin());
			for (string tokenS : tokens)
				memstr = memstr + " " + tokenS;
			fs.writeMemory(m, memstr);
			fs.outputFile << memstr.length() << " bytes written to M\n";
		}
		else if (tokens[0] == "rm"){
			int m = stoi(tokens[1]);
			int n = stoi(tokens[2]);
			fs.readMemory(m, n);
		}
		else if (tokens[0] == "sk"){
			int index = stoi(tokens[1]);
			int pos = stoi(tokens[2]);
			fs.seekWithOutput(index, pos);
			//fs.outputFile << "position is " << pos << std::endl;
		}
		else if (tokens[0] == "dr"){
			fs.directory();
		}
		else if (tokens[0] == "in"){
			fs.init();
			fs.outputFile << "system initialized\n";
		}
		else if (tokens[0] == "rd"){
			int index = stoi(tokens[1]);
			int mem = stoi(tokens[2]);
			int count = stoi(tokens[3]);
			fs.readWithOutput(index, mem, count);
			
		}
		else if (tokens[0] == "wr"){
			int index = stoi(tokens[1]);
			int mem = stoi(tokens[2]);
			int count = stoi(tokens[3]);
			fs.writeWithOutput(index, mem, count);
			
		}
		else
			fs.outputFile << "error\n";
	}
	inputFile.close();
	fs.outputFile.close();
	return 0;
}
