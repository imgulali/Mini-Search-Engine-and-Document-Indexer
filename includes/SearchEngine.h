#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>
#include "DocumentManager.h"
#include "Index.h"
#include "Analytics.h"

using namespace std;

struct SearchResult {
    DocumentMetadata doc;
    double score;
    string snippet;  
};

class SearchEngine {
public:
    SearchEngine();
     
    void addDocument(const string& path);
    void removeDocument(const string& identifier);
    void updateDocument(const string& identifier);
    void sync();  
    void reset();
     
    vector<SearchResult> search(const string& query);
    
    const DocumentManager& getDocumentManager() const;
    const Analytics& getAnalytics() const;

private:
    DocumentManager docManager;
    Index index;
    Analytics analytics;
};

#endif
