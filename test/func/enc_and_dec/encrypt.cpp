/*
    This program encrypts a given input file using a simple XOR encryption method.
    and writes the encrypted data to a new file with the same name but with an ".enc" extension.

    the encryptFile makes a file with .enc extension and writes the encrypted data to it.
    and also returns the encrypted data as a vector of unsigned char.

    the decryptData function takes the encrypted data which is a vector of unsigned char and the key used for encryption
    and returns the decrypted data as a vector of unsigned char.

*/

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

vector<unsigned char> encryptFile(const string& inputFilePath, const string& outputFilePath, char key)
{
    ifstream inputFile(inputFilePath, ios::binary);
    ofstream outputFile(outputFilePath, ios::binary);
    vector<unsigned char> encryptedData;

    if (!inputFile.is_open() || !outputFile.is_open())
    {
        cerr << "Error opening file!" << endl;
        return encryptedData;
    }

    char byte;
    while (inputFile.get(byte))
    {
        byte ^= key;
        outputFile.put(byte);
        encryptedData.push_back(byte);
    }

    inputFile.close();
    outputFile.close();

    return encryptedData;
}

vector<unsigned char> decryptData(const vector<unsigned char>& encryptedData, char key)
{
    vector<unsigned char> decryptedData(encryptedData.size());
    for (size_t i = 0; i < encryptedData.size(); ++i)
    {
        decryptedData[i] = encryptedData[i] ^ key;
    }
    return decryptedData;
}


int main(int argc, char* argv[])
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (argc < 2)
    {
        cerr << "Also provide a <input_file>" << endl;
        return 1;
    }

    string inputFilePath = argv[1];
    string outputFilePath = inputFilePath + ".enc";

    vector<unsigned char> encryptedData, decryptedData;
    string decryptedFilePath = "decrypted" + inputFilePath.substr(inputFilePath.find_last_of('.'));
    char key = 0x3F;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    encryptedData = encryptFile(inputFilePath, outputFilePath, key);
    cout << "File encrypted successfully." << endl;

//     decryptedData = decryptData(encryptedData, key);

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//     ofstream decryptedFile(decryptedFilePath, ios::binary);
//     if (!decryptedFile.is_open())
//     {
//         cerr << "Error opening decrypted file!" << endl;
//         return 1;
//     }

//     decryptedFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
//     decryptedFile.close();

//     cout << "File decrypted successfully." << endl;

    return 0;
}
