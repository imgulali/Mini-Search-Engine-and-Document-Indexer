#include "Analytics.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include "Utils.h"

using namespace std;
namespace fs = filesystem;

string getHistoryPath() {
    return Utils::getDataDir() + "/analytics.dat";
}

Analytics::Analytics() {
    load();
}

void Analytics::logSearch(const string& query) {
    if (query.empty()) return;
    SearchRecord record;
    record.query = query;
    record.timestamp = time(nullptr);
    history.push_back(record);
    termFrequency[query]++;
    save();
}

vector<SearchRecord> Analytics::getHistory() const {
    return history;
}

void Analytics::reset() {
    history.clear();
    termFrequency.clear();
    if (fs::exists(getHistoryPath())) {
        fs::remove(getHistoryPath());
    }
}

vector<pair<string, int>> Analytics::getTopTerms(int n) const {
    vector<pair<string, int>> terms(termFrequency.begin(), termFrequency.end());

    sort(terms.begin(), terms.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    if (terms.size() > (size_t)n) {
        terms.resize(n);
    }
    return terms;
}

void Analytics::save() {
    ofstream out(getHistoryPath(), ios::binary);
    if (!out) return;
    
    size_t histSize = history.size();
    out.write(reinterpret_cast<char*>(&histSize), sizeof(histSize));
    for (const auto& rec : history) {
        size_t len = rec.query.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(rec.query.c_str(), len);
        out.write(reinterpret_cast<char*>(const_cast<long long*>(&rec.timestamp)), sizeof(rec.timestamp));
    }
    
    size_t mapSize = termFrequency.size();
    out.write(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    for (const auto& pair : termFrequency) {
        size_t len = pair.first.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(pair.first.c_str(), len);
        out.write(reinterpret_cast<char*>(const_cast<int*>(&pair.second)), sizeof(pair.second));
    }
}

void Analytics::load() {
    ifstream in(getHistoryPath(), ios::binary);
    if (!in) return;
    
    history.clear();
    termFrequency.clear();
    
    size_t histSize;
    in.read(reinterpret_cast<char*>(&histSize), sizeof(histSize));
    for (size_t i = 0; i < histSize; ++i) {
        SearchRecord rec;
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        rec.query.resize(len);
        in.read(&rec.query[0], len);
        in.read(reinterpret_cast<char*>(&rec.timestamp), sizeof(rec.timestamp));
        history.push_back(rec);
    }
    
    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    for (size_t i = 0; i < mapSize; ++i) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        string term(len, ' ');
        in.read(&term[0], len);
        int freq;
        in.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        termFrequency[term] = freq;
    }
}
