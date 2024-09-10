#include <iostream>
#include <fstream>
#include <openssl/aes.h>
#include <openssl/rand.h>

void handleErrors() {
    std::cerr << "An error occurred." << std::endl;
    exit(EXIT_FAILURE);
}

void generateRandomKey(unsigned char* key, int keySize) {
    if (!RAND_bytes(key, keySize)) {
        handleErrors();
    }
}

void aesEncrypt(const unsigned char* key, const std::string& inputFile, const std::string& outputFile, const std::string& lengthFile) {
    // Create AES encryption key
    AES_KEY encKey;
    if (AES_set_encrypt_key(key, 128, &encKey) < 0) {
        handleErrors();
    }

    // Open input and output files
    std::ifstream inFile(inputFile, std::ios::binary | std::ios::ate);  // Open at end to get file size
    std::ofstream outFile(outputFile, std::ios::binary);

    if (!inFile.is_open() || !outFile.is_open()) {
        handleErrors();
    }

    // Get the size of the input file
    std::streampos inputFileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);  // Reset file pointer to beginning

    // Read input file and encrypt
    unsigned char inBuffer[16];
    unsigned char outBuffer[16];
    int bytesRead;

    while ((bytesRead = inFile.read(reinterpret_cast<char*>(inBuffer), 16).gcount()) > 0) {
        // Pad the last block if it's not 16 bytes
        if (bytesRead < 16) {
            memset(inBuffer + bytesRead, 16 - bytesRead, 16 - bytesRead);
        }

        AES_encrypt(inBuffer, outBuffer, &encKey);
        outFile.write(reinterpret_cast<char*>(outBuffer), 16);
    }

    // Save the input file length to the length file
    std::ofstream lengthFileStream(lengthFile);
    if (!lengthFileStream.is_open()) {
        handleErrors();
    }
    lengthFileStream << inputFileSize;
    lengthFileStream.close();

    // Clean up
    inFile.close();
    outFile.close();
}

int main() {
    const int KEY_SIZE = 16; // 128-bit key
    unsigned char key[KEY_SIZE];

    // Generate random AES key
    generateRandomKey(key, KEY_SIZE);

    // Save the key to file
    std::ofstream keyFile("aes.key", std::ios::binary);
    keyFile.write(reinterpret_cast<char*>(key), KEY_SIZE);
    keyFile.close();

    // Encrypt the file and save the length
    aesEncrypt(key, "payload.bin", "payload_encrypted.bin", "payload_length.txt");

    std::cout << "Encryption complete. Encrypted file: payload_encrypted.bin, Key file: aes.key, Length file: payload_length.txt" << std::endl;

    return 0;
}
