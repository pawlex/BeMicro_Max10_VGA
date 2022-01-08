#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>

#define PROG_VERSION "1.0"

/* bin2ihex
 * Converts between binary and a subset of Intel-format HEX file.
 * Ryan O'Malley, 2017
 */

// Enum for detected filetype, with "NoFile" being an error case.
enum FileTypeEnum { NoFile, BinFile, HexFile };

// The following type just lets me access high and low bytes of the 16-bit address.
union AddressBytes {
    uint8_t Bytes[2];
    uint16_t Full;
};

int HandleBinFile(std::ifstream &InputFile, std::ofstream &OutputFile, int dataSize) {
    char inputByte;
    AddressBytes startingAddress;
    startingAddress.Full = 0;
    unsigned char checkSum; // Two's complement of sum of all bytes on a line.
    while (InputFile.read(&inputByte,1)) {
        // Start the checksum with everything but the bytes we're writing.
        checkSum = dataSize + startingAddress.Bytes[0] + startingAddress.Bytes[1] + inputByte;
        // Print some data according to the file format specs.
        OutputFile << ':';
        OutputFile << std::hex << std::setw(2) << std::setfill('0') << dataSize;
        OutputFile << std::hex << std::setw(4) << std::setfill('0') << startingAddress.Full;
        OutputFile << std::hex << std::setw(2) << std::setfill('0') << 0;
        OutputFile << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)inputByte;
        // If the data size is greater than 1, print the other data bytes.
        for (int i = 1; i < dataSize; ++i) {
            if (InputFile.read(&inputByte,1)) {
                checkSum += inputByte;
                OutputFile << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)inputByte;
            }
            else // pad the unused space with zeroes.
                OutputFile << std::hex << std::setw(2) << std::setfill('0') << 0;
        }
        checkSum = (~checkSum) + 1; // Wrap up the checksum with two's complement
        OutputFile << std::hex << std::setw(2) << std::setfill('0') << (int)checkSum << std::endl;
        startingAddress.Full += dataSize;
    }
    OutputFile << ":00000001ff" << std::endl; // EOF - Always the same
    return 1; // 1 is success.
}

int HandleHexFile(std::ifstream &InputFile, std::ofstream &OutputFile) {
    uint8_t outputByte;
    int dataSize;
    AddressBytes startingAddress;
    unsigned char checkSum; // The calculated checksum.
    unsigned char thisCheckSum; // The checksum read from the file.
    char dataString[5];
    int lineNum = 1;
    while (InputFile.get(dataString,2)) { // Remember that get() reads n-1 characters.
        if (dataString[0] != ':') break;
        // Get the data size
        InputFile.get(dataString,3);
        dataSize = strtoul(dataString,NULL,16);
        if (dataSize == 0) return 1; // EOF hit, done writing.
        // Get the starting address
        InputFile.get(dataString,5);
        startingAddress.Full = strtoul(dataString,NULL,16);
        // Calculate part of the checksum now.
        checkSum = dataSize + startingAddress.Bytes[0] + startingAddress.Bytes[1];
        InputFile.ignore(2); // Ignore the record type.
        // Loop through the data and write to the file, while adding to the checksum.
        for (int i = 0; i < dataSize; ++i) {
            InputFile.get(dataString,3);
            outputByte = strtoul(dataString,NULL,16);
            checkSum += outputByte;
            OutputFile << outputByte;
        }
        // Finally, get the included checksum for verification.
        InputFile.get(dataString,3);
        thisCheckSum = strtoul(dataString,NULL,16);
        checkSum = (~checkSum) + 1;
        // Sanity check: compare the checksum we've been keeping with the one in the file.
        if (checkSum != thisCheckSum)
            std::cerr << "Line " << lineNum << ": Checksum may not be valid. Continuing..." << std::endl;
        InputFile >> std::ws;
        ++lineNum;
    }
    return 0;
}

void PrintHelp () {
    std::cout <<
    "Usage:\n"
    "-b:              Forces binary-to-hex.\n"
    "-x:              Forces hex-to-binary.\n"
    "-o:              Sets the output filename.\n"
    "                 (Default: input with new extension)\n"
    "-s:              Sets the data size per line. (for bin->hex)\n"
    "                 (Between 1 and 255. Default: 1 byte.)\n"
    "-h or --help:    Gives you this text.\n"
    "-v or --version: Prints the version.\n" << std::flush;
}

int main (int argc, char ** argv) {
    std::string fullFileName;
    std::string filePath;
    std::string fileName;
    std::string fileExtension;
    FileTypeEnum fileType = NoFile;
    std::string outputFileName;
    int dataSize = 1;

    std::cout << "** bin2ihex **" << std::endl;

    // If no arguments specified, print help.
    if (argc < 2) {
        PrintHelp();
        return 0;
    }

    // Process arguments.
    for (int i = 1; i < argc; ++i) {
        std::string token(argv[i]);
        if (token == "-h" || token == "--help") {
            PrintHelp();
            return 0;
        }
        else if (token == "-v" || token == "--version") {
            std::cout << "Version " << PROG_VERSION << std::endl;
            return 0;
        }
        else if (token == "-x") {
            if (fileType == NoFile)
                fileType = HexFile;
            else
                std::cerr << "Multiple filetypes specified. Using the first..." << std::endl;
        }
        else if (token == "-b") {
            if (fileType == NoFile)
                fileType = BinFile;
            else
                std::cerr << "Multiple filetypes specified. Using the first..." << std::endl;
        }
        else if (token == "-o") {
            ++i;
            if (i >= argc) {
                std::cerr << "Please specify the filename when using \"-o\"." << std::endl;
                return 1;
            }
            outputFileName = argv[i];
        }
        else if (token == "-s") {
            ++i;
            if (i >= argc) {
                std::cerr << "Please specify the filename when using \"-s\"." << std::endl;
                return 1;
            }
            dataSize = atoi(argv[i]);
            if (dataSize < 1) {
                std::cerr << "Data size is too small. Must be more than 0." << std::endl;
                return 1;
            }
            else if (dataSize > 255) {
                std::cerr << "Data size is too large. Must be less than 256." << std::endl;
                return 1;
            }
        }
        else {
            if (fullFileName.empty()) {
                fullFileName = argv[i];
            }
            else {
                std::cerr << "Duplicate file specified. Using the first." << std::endl;
            }
        }
    }

    // If the file name wasn't detected in the arguments, tell the user.
    if (fullFileName.empty()) {
        std::cerr << "No file detected. Please specify a file." << std::endl;
        return 1;
    }

    // Prepare substrings
    fileName = fullFileName.substr(fullFileName.find_last_of("/\\") + 1);
    filePath = fullFileName.substr(0,fullFileName.find_last_of("/\\") + 1);
    fileExtension = fileName.substr(fileName.find_last_of(".") + 1);

    // Detect the filetype based off of the extension if none specified.
    if (fileType == NoFile) {
        if (fileExtension == "bin")
            fileType = BinFile;
        else if (fileExtension == "hex")
            fileType = HexFile;
        else {
            std::cerr << "No valid file detected." << std::endl;
            return 1;
        }
    }

    // Detect the output filename if none specified.
    if (outputFileName.empty()) {
        outputFileName = fileName.substr(0, fileName.find_last_of("."));
        outputFileName+= (fileType == BinFile) ? ".hex" : ".bin";
    }

    // Open the input file
    std::ifstream InputFile(fullFileName);
    if (!InputFile.good()) {
        std::cerr << "File " << fileName << " can't be opened." << std::endl;
        return 1;
    }
    // Open the output file
    std::ofstream OutputFile(filePath + outputFileName);
    if (!OutputFile.good()) {
        std::cerr << "Tried to open " << outputFileName << " for writing, but failed." << std::endl;
        return 1;
    }

    // Convert the file
    int returnCode;
    if (fileType == BinFile) returnCode = HandleBinFile(InputFile, OutputFile, dataSize);
    else returnCode = HandleHexFile(InputFile, OutputFile);
    if (returnCode)
        std::cout << "Successfully converted " << fileName << " to " << outputFileName << "." << std::endl;
    else 
        std::cout << "Conversion of " << fileName << " failed." << std::endl;

    return 0;
}