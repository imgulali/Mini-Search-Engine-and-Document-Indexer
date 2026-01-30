#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct SearchRecord {
    string query;
    long long timestamp;
};

class Analytics {
public:
    Analytics();
    void logSearch(const string& query);
    vector<SearchRecord> getHistory() const;
    vector<pair<string, int>> getTopTerms(int n) const;
    
    void save();
    void load();
    void reset();

private:
    vector<SearchRecord> history;
    unordered_map<string, int> termFrequency;
};

#endif
