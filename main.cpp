#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <random>
#include <algorithm>
#include <unordered_set>

using namespace std;

bool CompareAllBitInfos(const std::vector<std::vector<BitInfo>>& allBitInfos) {
    // Compare all bit information across multiple datasets
    for (size_t i = 0; i < allBitInfos.size(); ++i) {
        for (size_t j = i + 1; j < allBitInfos.size(); ++j) {
            if (allBitInfos[i].size() != allBitInfos[j].size()) return false;
            for (size_t k = 0; k < allBitInfos[i].size(); ++k) {
                if (allBitInfos[i][k].flipRate != allBitInfos[j][k].flipRate || 
                    allBitInfos[i][k].type != allBitInfos[j][k].type) {
                    return false;
                }
            }
        }
    }
    return true;
}

std::vector<size_t> FindInconsistentBits(const std::vector<std::vector<BitInfo>>& allBitInfos) {
    // Identify inconsistent bits across different bit information datasets
    std::vector<size_t> inconsistentBits;
    if (allBitInfos.empty()) return inconsistentBits;

    size_t numBits = allBitInfos[0].size();
    for (size_t i = 0; i < numBits; ++i) {
        bool isConsistent = true;
        for (size_t j = 1; j < allBitInfos.size() && isConsistent; ++j) {
            if (allBitInfos[0][i].type != allBitInfos[j][i].type) {
                isConsistent = false;
            }
        }
        if (!isConsistent) {
            inconsistentBits.push_back(i);
        }
    }
    return inconsistentBits;
}

int Generate(const std::string& inputId, std::unordered_map<std::string, std::vector<std::string>>& resultMap, 
             std::vector<BitInfo>& bitinfo, int fileIndex, int numSamples = 1) {
    // Generate synthetic data samples based on bit information
    // This function takes an input ID, retrieves corresponding data,
    // and generates new samples based on bit flipping probabilities.
    // 
    // Parameters:
    // - inputId: The identifier of the data being processed.
    // - resultMap: A map containing stored bit sequences for various IDs.
    // - bitinfo: A vector containing bit flipping rates and types.
    // - fileIndex: An index used for naming the generated files.
    // - numSamples: The number of samples to generate.
    //
    // Returns:
    // - 0 on success, 1 if the ID is not found.
    
    int len = (inputId.length() >= 5 ? inputId[4] - '0' : -1);
    auto it = resultMap.find(inputId);
    if (it == resultMap.end()) {
        std::cout << "File " << fileIndex << " does not contain this ID." << std::endl;
        return 1;
    }

    const std::vector<std::string>& dataVector = it->second;
    size_t numBits = dataVector[0].length() * 4;
    size_t numSamplesInDataset = dataVector.size();

    static std::unordered_map<int, int> callCounters;
    static std::string fileName;
    int& callCounter = callCounters[fileIndex];

    std::string filepath = "";
    if (callCounter % 10 == 0) {
        // Create a new file every 10 calls
        fileName = filepath + inputId + '_' + std::to_string(fileIndex) + ".can";
        std::ofstream outputFile(fileName);
        if (!outputFile) {
            std::cout << "Failed to create file: " << fileName << std::endl;
            return 0;
        }
        
        // Randomly select a data entry from the dataset
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        srand(time(NULL));
        int randomIndex = rand() % dataVector.size();
        const std::string& selectedData = dataVector[randomIndex];

        int bitSize = 8 * len;
        std::bitset<64> binaryData(stoull(selectedData, nullptr, 16));
        binaryData <<= (64 - bitSize);
        binaryData >>= (64 - bitSize);
        std::string binaryString = binaryData.to_string().substr(64 - bitSize);

        // Convert binary data to hexadecimal format and write to file
        std::string hexString = binaryToHex(binaryString);
        std::string id = inputId.substr(0, 3);
        outputFile << "10,0,0," << id << ',' << hexString << std::endl;


            std::string nextData = "";
            std::string currData = binaryString;
            int equalCount = 0;
            
            for (int j = 0; j < SamplesNum - 1;) {
                double randnum = 1;
                int couple = 0;
                
                for (int i = numBits - 1; i >= 0; i--) {
                    // Handle different bit types (fixed, increasing, or random flip)
                    if (bitinfo[i].type == FIXED && (!couple)) {
                        nextData += currData[numBits - i - 1];
                    } else if (bitinfo[i].type == INCREASING) {
                        // will be updated later
                    } else {
                        if (couple) {
                            // will be updated later
                        }
                        double flipRate = bitinfo[i].flipRate;
                        randnum = dis(gen);
                        nextData += (randnum < flipRate) ? ((currData[numBits - i - 1] == '0') ? '1' : '0') : currData[numBits - i - 1];
                    }
                }

                // Convert to hex and check repetition
                if (nextData == currData) {
                    equalCount++;
                    if (equalCount % 20 == 0 || equalCount > 2000) {
                        outputFile << "10,0,0," << id << ',' << binaryToHex(nextData) << std::endl;
                        currData = nextData;
                        j += (equalCount > 2000) ? SamplesNum : (1 << (equalCount / 20));
                        if (equalCount > 2000) break;
                    }
                } else {
                    outputFile << "10,0,0," << id << ',' << binaryToHex(nextData) << std::endl;
                    currData = nextData;
                    equalCount = 0;
                    j++;
                }
                nextData = "";
            }
            
            std::cout << "Sample generated successfully. File name: " << fileName << std::endl;
            outputFile.close();
            callCounter++;
            return 0;
        }
        else {
            // Append data to the existing file
            fileName.replace(fileName.length() - 5, 1, std::to_string(a));
            std::ofstream outputFile(fileName, std::ios::app);
            if (!outputFile) {
                std::cout << "Failed to open file for appending: " << fileName << std::endl;
                return 0;
            }
            
            // Repeat similar processing for appending data
            // (The rest of this block is similar to the previous one and follows the same logic)
            // Select a random data entry, convert it, and append new samples

            std::cout << "Sample appended successfully. File name: " << fileName << std::endl;
            callCounter++;
            outputFile.close();
            return 0;
        }
    }
    else {
        // Handle case where the input ID is not found in the dataset
        std::cout << "File " << a << " does not contain this ID" << std::endl;
        return 1;
    }
}

