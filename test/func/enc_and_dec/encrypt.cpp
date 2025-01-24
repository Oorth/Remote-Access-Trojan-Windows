#include <iostream>
#include <fstream>
#include <vector>

void encryptFile(const std::string& inputFilePath, const std::string& outputFilePath, char key)
{
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    std::ofstream outputFile(outputFilePath, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open())
    {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    char byte;
    while (inputFile.get(byte))
    {
        byte ^= key;
        outputFile.put(byte);
    }

    inputFile.close();
    outputFile.close();
}

void decryptFile(const std::string& inputFilePath, const std::string& outputFilePath, char key)
{
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    std::ofstream outputFile(outputFilePath, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    char byte;
    while (inputFile.get(byte))
    {
        byte ^= key;
        outputFile.put(byte);
    }

    inputFile.close();
    outputFile.close();
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Also provide a <input_file>" << std::endl;
        return 1;
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = inputFilePath + ".enc";
    std::string decryptedFilePath = "decrypted" + inputFilePath.substr(inputFilePath.find_last_of('.'));
    char key = 0xAA;

    encryptFile(inputFilePath, outputFilePath, key);
    std::cout << "File encrypted successfully." << std::endl;

    decryptFile(outputFilePath, decryptedFilePath, key);
    std::cout << "File decrypted successfully." << std::endl;

    return 0;
}
