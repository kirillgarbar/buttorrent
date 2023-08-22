#include "utils.hpp"
#include <bitset>

//Converts consecutive bytes to integer by merging them together
int bytesToInt(const std::string bytes) {
    std::string binStr;
    long byteCount = bytes.size();
    for (int i = 0; i < byteCount; i++)
        binStr += std::bitset<8>(bytes[i]).to_string();
    return stoi(binStr, 0, 2);
}
    
//Decode string from hex to ordinary format
std::string hexDecode(const std::string& value) {
    int hashLength = value.length();
    std::string decodedHexString;
    for (int i = 0; i < hashLength; i += 2)
    {
        std::string byte = value.substr(i, 2);
        char c = (char) (int) strtol(byte.c_str(), nullptr, 16);
        decodedHexString.push_back(c);
    }
    return decodedHexString;
}

//Get bit n'th from bitfield
bool getBit(const std::string& bitField, int n) {
    char byte = bitField.at(n / 8);
    int offset = 7 - n % 8;
    return (byte >> offset) & 1 != 0;
}

//Set bit n'th from bitfield
bool setBit(std::string& bitField, int n) {
    int offset = 7 - n % 8;
    return bitField.at(n / 8) |= (1 << offset);
}

std::string formatTime(long seconds) {
    if (seconds < 0)
        return "inf";

    std::string result;
    // compute h, m, s
    std::string h = std::to_string(seconds / 3600);
    std::string m = std::to_string((seconds % 3600) / 60);
    std::string s = std::to_string(seconds % 60);
    // add leading zero if needed
    std::string hh = std::string(2 - h.length(), '0') + h;
    std::string mm = std::string(2 - m.length(), '0') + m;
    std::string ss = std::string(2 - s.length(), '0') + s;
    // return mm:ss if hh is 00
    if (hh.compare("00") != 0) {
        result = hh + ':' + mm + ":" + ss;
    }
    else {
        result =  mm + ":" + ss;
    }
    return result;
}