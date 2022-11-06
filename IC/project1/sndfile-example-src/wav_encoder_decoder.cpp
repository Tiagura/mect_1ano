#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include "BitStream.h"

using namespace std;

int main(int argc, char *argv[]) {

    if(argc < 3) {
		cerr << "Usage: wav_cp <file_to_encode> <file_decoded>" << endl;
		return 1;
	}

    //check if is a txt file
    string file_to_encode = argv[1];
    string file_decoded = argv[2];
    string file_extension = file_to_encode.substr(file_to_encode.find_last_of(".") + 1);
    if(file_extension != "txt"){
        cerr << "Error: file to encode is not a txt file" << endl;
        return 1;
    }

    //check if output file is a txt file
    file_extension = file_decoded.substr(file_decoded.find_last_of(".") + 1);
    if(file_extension != "txt"){
        cerr << "Error: file decoded is not a txt file" << endl;
        return 1;
    }

    //add " to the file name and pass to const char*
    const char* file_to_encode_c = file_to_encode.c_str();
    const char* file_decoded_c = file_decoded.c_str();

    BitStream bsEncode(file_to_encode_c, 'r');	//read from file
	bsEncode.encoder("encodedFile.txt", 8);	//encode bin to chars
    BitStream bsDecode("encodedFile.txt", 'r');	//read from file
    bsDecode.decoder(file_decoded_c);	//decode chars to bin

    //open files to compare
    fstream file1, file2;
    file1.open(file_to_encode_c, ios::in);
    file2.open(file_decoded_c, ios::in);
    
    //compare file_to_encode and file_decoded
    char ch1, ch2;
    file1.seekg(0, ios::beg);
    file2.seekg(0, ios::beg);
    while(file1 >> ch1 && file2 >> ch2){
        if(ch1 != ch2){
            cerr << "Error: file to encode and file decoded are different" << endl;
            return 1;
        }

    }

    //if file_to_encode and file_decoded still have chars
    if(file1 >> ch1 || file2 >> ch2){
        cerr << "Error: file to encode and file decoded are different" << endl;
        return 1;
    }

    cout << "Files are the same" << endl;

    return 0;
}










