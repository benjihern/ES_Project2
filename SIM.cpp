#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <algorithm>

class CodeCompression {
private:
    std::vector<std::string> binary_code;
    std::vector<std::string> original_binary;
    std::string uncompressed_code;
    std::string dictionary[16];

public:
    CodeCompression() {}
    
    std::vector<std::string> getBinaryCode() { return binary_code; }
    void readBinaryFile(const std::string& filename);
    void writeCompressedFile(const std::string& filename);
    void writeDecompressedFile(const std::string& filename);
    
    void compress();
    void decompress(const std::string& filename);
    
    void bitmask();
    void bitMismatch1();
    void bitMismatch2Consecutive();  
    void bitMismatch4();
    void bitSeparated2();
    void directMatching();
    void uncompressed();
    void RLE();
    
    void setDictionary();
    void printDictionary();
    std::string decompressPattern(const std::string& pattern);
};

void CodeCompression::readBinaryFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            binary_code.push_back(line);
            original_binary.push_back(line);
        }
    }

    file.close();
}

void CodeCompression::writeCompressedFile(const std::string& filename) {
    std::ofstream file(filename);

    std::string code_string;
    for (const auto& code : binary_code) {
        code_string += code;
    }

    int num_zeros = 32 - (code_string.length() % 32);

    for (int i = 0; i < code_string.length(); i += 32) {
        if (code_string.substr(i, 32).length() < 32) {
            file << code_string.substr(i, 32); 
            for (int i = 0; i < num_zeros; i++) file << "0";
            file << "\n";
        }
        else file << code_string.substr(i, 32) << "\n";
    }
    
    file << "xxxx" << std::endl;
    
    for (int i = 0; i < 16; i++) {
        file << dictionary[i] << std::endl;
    }
    
    file.close();
}

void CodeCompression::writeDecompressedFile(const std::string& filename) {
    std::ofstream file(filename);
    
    file << uncompressed_code << std::endl;
    
    file.close();
}

void CodeCompression::compress() {
    setDictionary();
    directMatching();
    bitmask();
    bitMismatch1();
    bitMismatch2Consecutive();
    bitMismatch4();
    bitSeparated2();
    uncompressed();
    RLE();
}

void CodeCompression::decompress(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::string compressed_code;

    while (std::getline(file, line) && line != "xxxx") {
        compressed_code += line;
    }
    
    //std::cout << compressed_code << "\n" << std::endl;

    for (int i = 0; i < 16; i++) {
        std::getline(file, line);
        dictionary[i] = line;
        //std::cout << dictionary[i] << std::endl;
    }

    file.close();
    int count = 0;
    while (compressed_code.length() > 0){
        if (compressed_code.length() == 0) {
            //std::cout << "Completed Full " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
            break;
        }
        std::string format = compressed_code.substr(0,3);
        if (format == "000") {
            if (compressed_code.length() < 32) {
                //std::cout << "Completed Full " << count << std::endl;
                //std::cout << uncompressed_code << std::endl;
                break;
            }
            int len = 32;
            std::string code = compressed_code.substr(3, len);
            uncompressed_code += code + '\n';
            compressed_code = compressed_code.substr(len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "001") {
            int num_len = 3;
            std::string num_string = compressed_code.substr(3, num_len);
            std::string prev = uncompressed_code.substr(uncompressed_code.length() - 33);

            int num = std::stoi(num_string, nullptr, 2);

            for (int i = 0; i < num + 1; i++) {
                uncompressed_code += prev;
                count++;
            }

            compressed_code = compressed_code.substr(num_len + 3);
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "010") {
            int location_len = 5;
            int bitmask_len = 4;
            int index_len = 4;

            std::string location_string = compressed_code.substr(3, location_len);
            std::string bitmask_string = compressed_code.substr(3 + location_len, bitmask_len);
            std::string index_string = compressed_code.substr(3 + location_len + bitmask_len, index_len);

            int location = std::stoi(location_string, nullptr, 2);
            int index = std::stoi(index_string, nullptr, 2);

            std::string entry = dictionary[index];

            for (int i = 0; i < bitmask_len; i++) {
                if (bitmask_string[i] == '1') {
                    if (entry[location + i] == '1') entry[location + i] = '0';
                    else entry[location + i] = '1';
                }
            }

            uncompressed_code += entry + '\n';
            compressed_code = compressed_code.substr(bitmask_len + location_len + index_len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "011") {
            int location_len = 5;
            int index_len = 4;

            std::string location_string = compressed_code.substr(3, location_len);
            std::string index_string = compressed_code.substr(3 + location_len, index_len);

            int location = std::stoi(location_string, nullptr, 2);
            int index = std::stoi(index_string, nullptr, 2);

            std::string entry = dictionary[index];
            if (entry[location] == '1') entry[location] = '0';
            else entry[location]  = '1';
            uncompressed_code += entry + '\n';
            compressed_code = compressed_code.substr(location_len + index_len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "100") {
            int location_len = 5;
            int index_len = 4;

            std::string location_string = compressed_code.substr(3, location_len);
            std::string index_string = compressed_code.substr(3 + location_len, index_len);

            int location = std::stoi(location_string, nullptr, 2);
            int index = std::stoi(index_string, nullptr, 2);

            std::string entry = dictionary[index];
            if (entry.substr(location, 2) == "00") {
                entry[location] = '1';
                entry[location + 1] = '1';
            }
            else if (entry.substr(location, 2) == "01") {
                entry[location] = '1';
                entry[location + 1] = '0';
            }
            else if (entry.substr(location, 2) == "10") {
                entry[location] = '0';
                entry[location + 1] = '1';
            }
            else {
                entry[location]  = '0';
                entry[location + 1]  = '0';
            }
            uncompressed_code += entry + '\n';
            compressed_code = compressed_code.substr(location_len + index_len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "101") {
            int location_len = 5;
            int index_len = 4;

            std::string location_string = compressed_code.substr(3, location_len);
            std::string index_string = compressed_code.substr(3 + location_len, index_len);

            int location = std::stoi(location_string, nullptr, 2);
            int index = std::stoi(index_string, nullptr, 2);

            std::string entry = dictionary[index];
            std::string mismatch = entry.substr(location, 4);
            
            for (int i = 0; i < mismatch.length(); i++) {
                if (mismatch[i] == '1') entry[location + i] = '0';
                else entry[location + i] = '1';
            } 
            uncompressed_code += entry + '\n';
            compressed_code = compressed_code.substr(location_len + index_len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "110") {
            int location1_len = 5;
            int location2_len = 5;
            int index_len = 4;

            std::string location1_string = compressed_code.substr(3, location1_len);
            std::string location2_string = compressed_code.substr(3 + location1_len, location2_len);
            std::string index_string = compressed_code.substr(3 + location1_len + location2_len, index_len);

            int location1 = std::stoi(location1_string, nullptr, 2);
            int location2 = std::stoi(location2_string, nullptr, 2);
            int index = std::stoi(index_string, nullptr, 2);

            std::string entry = dictionary[index];

            if (entry[location1] == '1') entry[location1] = '0';
            else entry[location1]  = '1';

            if (entry[location2] == '1') entry[location2] = '0';
            else entry[location2]  = '1';

            uncompressed_code += entry + '\n';
            compressed_code = compressed_code.substr(location1_len + location2_len + index_len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
        else if (format == "111") {
            int len = 4;
            std::string code = compressed_code.substr(3, len);
            uncompressed_code += dictionary[std::stoi(code, nullptr, 2)] + '\n';
            compressed_code = compressed_code.substr(len + 3);
            count++;
            //std::cout << "Completed " << count << std::endl;
            //std::cout << uncompressed_code << std::endl;
        }
    }
}



void CodeCompression::RLE() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i] == binary_code[i + 1]) {
            int j = 0;
            while (binary_code[i] == binary_code[i + j + 1] && j < 8) {
                j++;
            }
            binary_code[i + 1] =  "001" + std::bitset<3>(j - 1).to_string();
            if (j > 1) {
                binary_code.erase(binary_code.begin() + i + 2, binary_code.begin() + i + j + 1);
            }
        }
    }
}

void CodeCompression::uncompressed() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i] == original_binary[i]) {
            binary_code[i] = "000" + binary_code[i];
        }
    }
}

void CodeCompression::setDictionary() {
    std::map<std::string, int> frequencies_map;
    std::vector<std::pair<std::string, int>> first;
    
    for (int i = 0; i < original_binary.size(); i++) {
        const std::string& pattern = original_binary[i];
        frequencies_map[pattern]++;
        bool found = false;
        for (const auto& entry : first) {
            if (entry.first == pattern) {
                found = true;
                break;
            }
        }
        if (!found) {
            first.push_back({pattern, i});
        }
    }
    
    std::vector<std::pair<std::string, int>> frequencies;
    for (const auto& entry : frequencies_map) {
        frequencies.push_back(entry);
    }
    
    std::sort(frequencies.begin(), frequencies.end(), 
        [&first](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            } 
            else {
                int pos_a = -1, pos_b = -1;
                for (const auto& entry : first) {
                    if (entry.first == a.first) pos_a = entry.second;
                    if (entry.first == b.first) pos_b = entry.second;
                }
                return pos_a < pos_b;
            }
        });
    
    for (int i = 0; i < 16 && i < frequencies.size(); i++) {
        dictionary[i] = frequencies[i].first;
    }
}

void CodeCompression::printDictionary() {
    std::cout << "Dictionary:" << std::endl;
    for (int i = 0; i < 16; i++) {
        std::cout << std::bitset<4>(i).to_string() << ": " << dictionary[i] << std::endl;
    }
}

void CodeCompression::directMatching() {
    for (int i = 0; i < binary_code.size(); i++) {
        for (int j = 0; j < 16; j++) {
            if (!dictionary[j].empty() && binary_code[i] == dictionary[j]) {
                binary_code[i] = "111" + std::bitset<4>(j).to_string();
                break;
            }
        }
    }
}

void CodeCompression::bitmask() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i].substr(0, 3) == "111") {
            continue;
        }
        
        const std::string& current = original_binary[i];
        std::string best_compressed;
        int best_index = -1;

        for (int index = 0; index < 16; index++) {
            if (dictionary[index].empty()) continue;
            
            const std::string& entry = dictionary[index];
            
            for (int start = 0; start <= 27; start++) {
                std::string bitmask = "0000";
                bool validMask = false;
                
                for (int j = 0; j < 4; j++) {
                    if (start + j < 32 && entry[start + j] != current[start + j]) {
                        bitmask[j] = '1';
                        validMask = true;
                    }
                }
                
                if (validMask && bitmask[0] == '1') {
                    std::string modified = entry;
                    for (int j = 0; j < 4; j++) {
                        if (bitmask[j] == '1' && start + j < 32) {
                            modified[start + j] = modified[start + j] == '0' ? '1' : '0';
                        }
                    }
                    
                    if (modified == current) {
                        std::string compressed = "010" 
                            + std::bitset<5>(start).to_string() 
                            + bitmask + std::bitset<4>(index).to_string();
                        
                        if (best_compressed.empty() || index < best_index) {
                            best_compressed = compressed;
                            best_index = index;
                        }
                    }
                }
            }
        }
        
        if (!best_compressed.empty()) {
            binary_code[i] = best_compressed;
        }
    }
}

void CodeCompression::bitMismatch1() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i].substr(0, 3) == "111" || binary_code[i].substr(0, 3) == "010") {
            continue;
        }
        
        const std::string& current = original_binary[i];
        
        for (int index = 0; index < 16; index++) {
            if (dictionary[index].empty()) continue;
            
            const std::string& entry = dictionary[index];
            int count = 0;
            int location = -1;
            
            for (int j = 0; j < 32 && j < current.length() && j < entry.length(); j++) {
                if (current[j] != entry[j]) {
                    count++;
                    location = j;
                }
            }
            
            if (count == 1) {
                std::string compressed = "011" + std::bitset<5>(location).to_string()
                    + std::bitset<4>(index).to_string();
                binary_code[i] = compressed;
                break;
            }
        }
    }
}

void CodeCompression::bitMismatch2Consecutive() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i].substr(0, 3) == "111" || 
            binary_code[i].substr(0, 3) == "010" || 
            binary_code[i].substr(0, 3) == "011") {
            continue;
        }
        
        const std::string& current = original_binary[i];
        
        for (int index = 0; index < 16; index++) {
            if (dictionary[index].empty()) continue;
            const std::string& entry = dictionary[index];

            for (int start = 0; start < 31; start++) {
                if (start + 1 < 32 && 
                    current[start] != entry[start] && 
                    current[start + 1] != entry[start + 1]) {

                    bool match = true;
                    for (int j = 0; j < 32; j++) {
                        if (j != start && j != start + 1) {
                            if (j < current.length() && j < entry.length() && current[j] != entry[j]) {
                                match = false;
                                break;
                            }
                        }
                    }
                    
                    if (match) {
                        std::string compressed = "100" + std::bitset<5>(start).to_string()
                            + std::bitset<4>(index).to_string();
                        binary_code[i] = compressed;
                        break;
                    }
                }
            }
        }
    }
}

void CodeCompression::bitMismatch4() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i].substr(0, 3) == "111" || 
            binary_code[i].substr(0, 3) == "010" || 
            binary_code[i].substr(0, 3) == "011" ||
            binary_code[i].substr(0, 3) == "100") {
            continue;
        }
        
        const std::string& current = original_binary[i];
        
        for (int index = 0; index < 16; index++) {
            if (dictionary[index].empty()) continue;
            const std::string& entry = dictionary[index];
            for (int start = 0; start <= 28; start++) {
                int count = 0;
                for (int j = 0; j < 4; j++) {
                    if (start + j < 32 && current[start + j] != entry[start + j]) {
                        count++;
                    }
                }     
                if (count == 4) {
                    bool match = true;
                    for (int j = 0; j < 32; j++) {
                        if (j < start || j >= start + 4) {
                            if (j < current.length() && j < entry.length() && current[j] != entry[j]) {
                                match = false;
                                break;
                            }
                        }
                    }
                    if (match) {
                        std::string compressed = "101" + std::bitset<5>(start).to_string()
                            + std::bitset<4>(index).to_string();
                        binary_code[i] = compressed;
                        break;
                    }
                }
            }
        }
    }
}

void CodeCompression::bitSeparated2() {
    for (int i = 0; i < binary_code.size(); i++) {
        if (binary_code[i].substr(0, 3) == "111" || 
            binary_code[i].substr(0, 3) == "010" || 
            binary_code[i].substr(0, 3) == "011" || 
            binary_code[i].substr(0, 3) == "100" ||
            binary_code[i].substr(0, 3) == "101") {
            continue;
        }
        
        const std::string& current = original_binary[i];
        
        for (int index = 0; index < 16; index++) {
            if (dictionary[index].empty()) continue;  
            const std::string& entry = dictionary[index];
            std::vector<int> locations;
            for (int j = 0; j < 32 && j < current.length() && j < entry.length(); j++) {
                if (current[j] != entry[j]) {
                    locations.push_back(j);
                }
            }
            if (locations.size() == 2) {
                if (locations[1] - locations[0] != 1) {
                    std::string compressed = "110" + std::bitset<5>(locations[0]).to_string() 
                        + std::bitset<5>(locations[1]).to_string() 
                        + std::bitset<4>(index).to_string();
                    binary_code[i] = compressed;
                    break;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    CodeCompression compressor;

    std::string mode = argv[1];
        
    if (mode == "1") {
        compressor.readBinaryFile("original.txt");
        compressor.compress();
        compressor.writeCompressedFile("cout.txt");
    }
    else if (mode == "2") {
        compressor.decompress("compressed.txt");
        compressor.writeDecompressedFile("dout.txt");
    }
    
    return 0;
}