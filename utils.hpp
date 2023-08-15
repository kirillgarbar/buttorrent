#pragma once

#include <string>

std::string hexDecode(const std::string& value);

int bytesToInt(std::string bytes);

bool getBit(const std::string& bitField, int n);

bool setBit(std::string& bitField, int n);