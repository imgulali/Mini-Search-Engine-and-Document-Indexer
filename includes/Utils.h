#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

namespace Utils {
    
    inline string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
     
    inline vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
     
    inline string cleanToken(const string& token) {
        string result;
        for (char c : token) {
            if (isalnum(c)) {
                result += c;
            }
        }
        return toLower(result);
    }
    
     
    string getDataDir();
}

#endif
