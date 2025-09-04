#pragma once
#include <string>
#include <vector>
#include <cctype>

inline std::vector<unsigned char> DecodeBase64(const std::string& input)
{
    std::vector<unsigned char> out;
    int val = 0, valb = -8;
    for (unsigned char c : input)
    {
        if (std::isspace(c) || c == '=') continue;
        int d = 0;
        if (c >= 'A' && c <= 'Z') d = c - 'A';
        else if (c >= 'a' && c <= 'z') d = c - 'a' + 26;
        else if (c >= '0' && c <= '9') d = c - '0' + 52;
        else if (c == '+') d = 62;
        else if (c == '/') d = 63;
        else break;
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
