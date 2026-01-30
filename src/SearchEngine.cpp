#include "SearchEngine.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

string readFile(const string& path) {
    ifstream in(path);
    if (!in) return "";
    stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

SearchEngine::SearchEngine() {
    sync();
}

void SearchEngine::sync() {
    auto missing = docManager.validateDocuments();
    for (int id : missing) {
        index.removeDocument(id);
    }
    if (!missing.empty()) {
        cout << "Notice: Cleaned up " << missing.size() << " missing documents from index.\n";
    }
}

void SearchEngine::reset() {
    docManager.reset();
    index.reset();
    analytics.reset();
    cout << "All data has been reset successfully.\n";
}

void SearchEngine::addDocument(const string& path) {
    docManager.addDocument(path);
    
    auto docs = docManager.getAllDocuments();
    if (docs.empty()) return;
    
    DocumentMetadata newDoc = docs.back(); 

    string content = readFile(newDoc.path);
    index.addDocument(newDoc.id, content);
}

void SearchEngine::removeDocument(const string& identifier) {
    int docId = -1;
    
    try {
        docId = stoi(identifier);
        if (!docManager.documentExists(docId)) {
            docId = -1; 
        }
    } catch (...) {
        docId = -1;
    }
    
    if (docId == -1) {
        auto docs = docManager.getAllDocuments();
        for (const auto& doc : docs) {
            if (doc.filename == identifier) {
                docId = doc.id;
                break;
            }
             
            if (doc.filename == identifier + ".txt") {
                 docId = doc.id;
                 break;
            }
        }
    }
    
    if (docId != -1) {
        docManager.removeDocument(docId);
        index.removeDocument(docId); 
    } else {
        cout << "Document not found: " << identifier << "\n";
    }
}

void SearchEngine::updateDocument(const string& identifier) {
    
    int docId = -1;
    string path;
    
    auto docs = docManager.getAllDocuments();
    for (const auto& doc : docs) {
        if (to_string(doc.id) == identifier) {
            docId = doc.id;
            path = doc.path;
            break;
        }
        if (doc.filename == identifier) {
            docId = doc.id;
            path = doc.path;
            break;
        }
        if (doc.filename == identifier + ".txt") {
            docId = doc.id;
            path = doc.path;
            break;
        }
    }
    
    if (docId != -1) {
        index.removeDocument(docId);
        string content = readFile(path);
        
        index.addDocument(docId, content);
        cout << "Updated document ID: " << docId << "\n";
    } else {
        cout << "Document not found for update.\n";
    }
}


string extractSnippet(const string& content, const string& query) {
    if (content.empty()) return "";
    
    string lowerContent = Utils::toLower(content);
    vector<string> queryTokens = Utils::split(Utils::toLower(query), ' ');
    
    size_t firstPos = string::npos;
    
    if (queryTokens.size() > 1) {
         firstPos = lowerContent.find(Utils::toLower(query));
    }
    
    if (firstPos == string::npos) {
        for (const auto& token : queryTokens) {
            string cleaned = Utils::cleanToken(token);
            if (cleaned.empty()) continue;
            size_t pos = lowerContent.find(cleaned);
            if (pos != string::npos) {
                if (firstPos == string::npos || pos < firstPos) {
                    firstPos = pos;
                }
            }
        }
    }

    if (firstPos == string::npos) return content.substr(0, 100) + "..."; 
    
    size_t start = firstPos;
    while (start > 0 && content[start-1] != '.' && content[start-1] != '!' && content[start-1] != '?') {
        start--;
    }
    
    while (start < content.length() && (content[start] == '\n' || content[start] == '\r' || content[start] == ' ')) {
        start++;
    }
      
    size_t end = firstPos;
    while (end < content.length() && content[end] != '.' && content[end] != '!' && content[end] != '?') {
        end++;
    }
    if (end < content.length()) end++; 
    
    string snippet = content.substr(start, end - start);   

    replace(snippet.begin(), snippet.end(), '\n', ' ');
    replace(snippet.begin(), snippet.end(), '\r', ' ');
    
    string cleanSnippet;
    bool lastSpace = false;
    for (char c : snippet) {
        if (c == ' ') {
            if (!lastSpace) {
                cleanSnippet += ' ';
                lastSpace = true;
            }
        } else {
            cleanSnippet += c;
            lastSpace = false;
        }
    }
    
    return cleanSnippet;
}

vector<SearchResult> SearchEngine::search(const string& query) {
    analytics.logSearch(query);
    
    vector<string> keywords = Utils::split(query, ' ');
    if (keywords.empty()) return {};
    
    map<int, double> relevanceMap;
    map<int, int> matchCount;
    int validKeywords = 0;
    
    for (const auto& rawWord : keywords) {
        string word = Utils::cleanToken(rawWord);
        if (word.empty()) continue;
        validKeywords++;
        
        auto results = index.lookup(word);
        for (const auto& pair : results) {
            int docId = pair.first;
            int freq = pair.second;
            
            relevanceMap[docId] += freq; 
            matchCount[docId]++;
        }
    }
    
    vector<SearchResult> finalResults;
    
    if (validKeywords == 0) return {};
    
    for (const auto& pair : relevanceMap) {
        int docId = pair.first;
        if (matchCount[docId] == validKeywords) {
            SearchResult res;
            res.doc = docManager.getDocument(docId);
            res.score = pair.second;
            string content = readFile(res.doc.path);
            res.snippet = extractSnippet(content, query);
            
            finalResults.push_back(res);
        }
    }
    
    sort(finalResults.begin(), finalResults.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });
    
    return finalResults;
}

const DocumentManager& SearchEngine::getDocumentManager() const {
    return docManager;
}

const Analytics& SearchEngine::getAnalytics() const {
    return analytics;
}
