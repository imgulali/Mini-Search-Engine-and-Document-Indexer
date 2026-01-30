#include "DocumentManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "Utils.h"

using namespace std;
namespace fs = filesystem;

string getDocsPath() {
    return Utils::getDataDir() + "/documents.dat";
}

DocumentManager::DocumentManager() : nextDocId(1) {
    load();
}

void DocumentManager::addDocument(const string& path) {
    if (!fs::exists(path)) {
        cerr << "Error: File not found: " << path << endl;
        return;
    }

    if (fs::is_directory(path)) {
        cerr << "Error: Path is a directory: " << path << endl;
        return;
    }

    fs::path p(path);
    if (p.extension() != ".txt") {
        cerr << "Error: Only .txt files are supported: " << path << endl;
        return;
    }

    string filename = p.filename().string();
    error_code ec;
    string absPath = fs::canonical(p, ec).string();
    if (ec) {
        absPath = fs::absolute(p).string();
    }

    for (const auto& doc : documents) {
        if (doc.path == absPath) {
            cout << "Document already indexed: " << filename << "\n";
            return;
        }
    }

    DocumentMetadata doc;
    doc.id = nextDocId++;
    doc.filename = filename;
    doc.path = absPath;
    doc.lastModified = fs::last_write_time(p).time_since_epoch().count();

    documents.push_back(doc);
    idToIndexMap[doc.id] = documents.size() - 1;
    
    save();
    cout << "Added document: " << filename << " (ID: " << doc.id << ")\n";
}

void DocumentManager::removeDocument(int docId) {
    if (idToIndexMap.find(docId) == idToIndexMap.end()) {
        cerr << "Error: Document ID " << docId << " not found.\n";
        return;
    }

    int indexToRemove = idToIndexMap[docId];
    DocumentMetadata& lastDoc = documents.back();
    DocumentMetadata& targetDoc = documents[indexToRemove];
    
    idToIndexMap[lastDoc.id] = indexToRemove;
    
    swap(targetDoc, lastDoc);
    documents.pop_back();
    idToIndexMap.erase(docId);

    save();
    cout << "Removed document ID: " << docId << "\n";
}

void DocumentManager::removeDocument(const string& filename) {
    int idToRemove = -1;
    for (const auto& doc : documents) {
        if (doc.filename == filename) {
            idToRemove = doc.id;
            break;
        }
    }
    
    if (idToRemove != -1) {
        removeDocument(idToRemove);
    } else {
        cerr << "Error: Document '" << filename << "' not found.\n";
    }
}

vector<DocumentMetadata> DocumentManager::getAllDocuments() const {
    return documents;
}

DocumentMetadata DocumentManager::getDocument(int docId) const {
    if (idToIndexMap.find(docId) != idToIndexMap.end()) {
        return documents[idToIndexMap.at(docId)];
    }
    return {};
}

bool DocumentManager::documentExists(int docId) const {
    return idToIndexMap.find(docId) != idToIndexMap.end();
}

vector<int> DocumentManager::validateDocuments() {
    vector<int> missingDocs;
    
    for (const auto& doc : documents) {
        if (!fs::exists(doc.path)) {
            missingDocs.push_back(doc.id);
        }
    }
    
    for (int docId : missingDocs) {
        removeDocument(docId);
    }
    
    return missingDocs;
}

void DocumentManager::save() {
    ofstream out(getDocsPath(), ios::binary);
    if (!out) return;

    size_t count = documents.size();
    out.write(reinterpret_cast<char*>(&count), sizeof(count));
    out.write(reinterpret_cast<char*>(&nextDocId), sizeof(nextDocId));

    for (const auto& doc : documents) {
        out.write(reinterpret_cast<char*>(const_cast<int*>(&doc.id)), sizeof(doc.id));
        
        size_t len = doc.filename.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(doc.filename.c_str(), len);
        
        len = doc.path.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(doc.path.c_str(), len);
        
        out.write(reinterpret_cast<char*>(const_cast<long long*>(&doc.lastModified)), sizeof(doc.lastModified));
    }
}

void DocumentManager::load() {
    ifstream in(getDocsPath(), ios::binary);
    if (!in) return;

    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    in.read(reinterpret_cast<char*>(&nextDocId), sizeof(nextDocId));

    documents.clear();
    idToIndexMap.clear();

    for (size_t i = 0; i < count; ++i) {
        DocumentMetadata doc;
        in.read(reinterpret_cast<char*>(&doc.id), sizeof(doc.id));
        
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        doc.filename.resize(len);
        in.read(&doc.filename[0], len);
        
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        doc.path.resize(len);
        in.read(&doc.path[0], len);
        
        in.read(reinterpret_cast<char*>(&doc.lastModified), sizeof(doc.lastModified));
        
        documents.push_back(doc);
        idToIndexMap[doc.id] = i;
    }
}

void DocumentManager::reset() {
    documents.clear();
    idToIndexMap.clear();
    nextDocId = 1;
    
    if (fs::exists(getDocsPath())) {
        fs::remove(getDocsPath());
    }
}
