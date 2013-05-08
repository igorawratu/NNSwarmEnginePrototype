#ifndef FILEREADER_H
#define FILEREADER_H

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class FileReader
{
public:
    FileReader(){}
    ~FileReader(){}

    string readText(char* filename)
    {
        ifstream file(filename);
        if(file)
        {
            string fileDat;
            file.seekg(0, file.end);
            fileDat.resize(file.tellg());
            file.seekg(0, file.beg);
            file.read(&fileDat[0], fileDat.size());
            file.close();
            return fileDat;
        }
        else
        {
            cerr << "Error with reading file " << filename << endl;
            return "Error reading file";
        }
    }

private:
    FileReader(const FileReader& other);
    const FileReader& operator=(const FileReader& other);
};

#endif