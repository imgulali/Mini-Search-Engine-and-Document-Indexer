#ifndef DOCUMENT_MANAGER_H
#define DOCUMENT_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct DocumentMetadata {
    int id;
    string filename;
    string path;
    long long lastModified;
};

class DocumentManager {
public:
    DocumentManager();
    void addDocument(const string& path);
    void removeDocument(int docId);
    void removeDocument(const string& filename);
    vector<DocumentMetadata> getAllDocuments() const;
    DocumentMetadata getDocument(int docId) const;
    bool documentExists(int docId) const;
    
    vector<int> validateDocuments();
     
    void save();
    void load();
    void reset();

private:
    vector<DocumentMetadata> documents;
    unordered_map<int, int> idToIndexMap;  
    int nextDocId;
};

#endif
