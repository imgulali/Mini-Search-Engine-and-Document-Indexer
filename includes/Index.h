#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include "DocumentManager.h"

using namespace std;

using InvertedIndex = unordered_map<string, vector<pair<int, int>>>;

class Index {
public:
    Index();
    void addDocument(int docId, const string& content);
    void removeDocument(int docId);
    vector<pair<int, int>> lookup(const string& word);
    
    void save();
    void load();
    void reset();

private:
    InvertedIndex index;
};

#endif
