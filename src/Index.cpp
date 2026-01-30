#include "Index.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

string getIndexPath() {
    return Utils::getDataDir() + "/index.dat";
}

Index::Index() {
    load();
}

void Index::reset() {
    index.clear();
    if (fs::exists(getIndexPath())) {
        fs::remove(getIndexPath());
    }
}

void Index::addDocument(int docId, const string& content) {
    vector<string> tokens = Utils::split(content, ' ');
    unordered_map<string, int> wordCounts;

    for (const auto& rawType : tokens) {
        string term = Utils::cleanToken(rawType);
        if (!term.empty()) {
            wordCounts[term]++;
        }
    }

    for (const auto& pair : wordCounts) {
        string term = pair.first;
        int count = pair.second;
        auto& docList = index[term];
         
        bool found = false;
        for (auto& entry : docList) {
            if (entry.first == docId) {
                entry.second = count;
                found = true;
                break;
            }
        }
        if (!found) {
            docList.push_back({docId, count});
        }
    }
    save();
}

void Index::removeDocument(int docId) {
    for (auto it = index.begin(); it != index.end(); ) {
        auto& docList = it->second;
        
        docList.erase(remove_if(docList.begin(), docList.end(),
            [docId](const pair<int, int>& entry) {
                return entry.first == docId;
            }), docList.end());

        if (docList.empty()) {
            it = index.erase(it);
        } else {
            ++it;
        }
    }
    save();
}

vector<pair<int, int>> Index::lookup(const string& word) {
    string term = Utils::toLower(word);
    if (index.find(term) != index.end()) {
        return index[term];
    }
    return {};
}

void Index::save() {
    ofstream out(getIndexPath(), ios::binary);
    if (!out) return;

    size_t size = index.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(size));

    for (const auto& pair : index) {
        size_t len = pair.first.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(pair.first.c_str(), len);
         
        size_t vecSize = pair.second.size();
        out.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
         
        for (const auto& entry : pair.second) {
            out.write(reinterpret_cast<char*>(const_cast<int*>(&entry.first)), sizeof(entry.first));  
            out.write(reinterpret_cast<char*>(const_cast<int*>(&entry.second)), sizeof(entry.second));  
        }
    }
}

void Index::load() {
    ifstream in(getIndexPath(), ios::binary);
    if (!in) return;

    index.clear();
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    for (size_t i = 0; i < size; ++i) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        string word(len, ' ');
        in.read(&word[0], len);

        size_t vecSize;
        in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

        vector<pair<int, int>> docList;
        docList.reserve(vecSize);
        for (size_t j = 0; j < vecSize; ++j) {
            int docId, freq;
            in.read(reinterpret_cast<char*>(&docId), sizeof(docId));
            in.read(reinterpret_cast<char*>(&freq), sizeof(freq));
            docList.push_back({docId, freq});
        }
        index[word] = docList;
    }
}
