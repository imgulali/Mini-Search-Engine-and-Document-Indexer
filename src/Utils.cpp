#include <windows.h>
#include "Utils.h"
#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = filesystem;

namespace Utils {
    string getDataDir() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        string fullPath(buffer);

        size_t pos = fullPath.find_last_of("\\/");
        string exeDir = (pos == string::npos) ? "" : fullPath.substr(0, pos);
        
        string dataDir = exeDir + "/.data";
        
        if (!fs::exists(dataDir)) {
            try {
                fs::create_directory(dataDir);
            } catch (const fs::filesystem_error& e) {
                cerr << "Error creating data directory: " << e.what() << endl;
            }
        }
        return dataDir;
    }
}
