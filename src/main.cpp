#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <conio.h>
#include <filesystem>
#include "SearchEngine.h"
#include "Utils.h"

using namespace std;
namespace fs = filesystem;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pause() {
    cout << "\n" << YELLOW << "Press any key to continue..." << RESET;
    getch();
}

void printHeader() {
    clearScreen();
    cout << BOLD << CYAN << "========================================\n";
    cout << "       DSA MINI SEARCH ENGINE           \n";
    cout << "========================================\n" << RESET;
}

void printMenu() {
    printHeader();
    cout << "1. " << GREEN << "Document Management" << RESET << " (Add/Remove/Update)\n";
    cout << "2. " << BLUE << "Search Documents" << RESET << "\n";
    cout << "3. " << MAGENTA << "Show All Documents" << RESET << "\n";
    cout << "4. " << YELLOW << "Analytics" << RESET << "\n";
    cout << "5. " << RED << "Reset All Data" << RESET << "\n";
    cout << "6. Exit\n";
    cout << "----------------------------------------\n";
    cout << "Enter choice: ";
}


void ensureDocsDir() {
    if (!fs::exists("documents")) {
        fs::create_directory("documents");
    }
}

void handleDocumentMenu(SearchEngine& engine) {
    printHeader();
    engine.sync(); 
    cout << BOLD << "--- Document Management ---\n" << RESET;
    cout << "1. Create New Document\n";
    cout << "2. Add Existing Document or Folder\n";
    cout << "3. Remove Document\n";
    cout << "4. Update Document\n";
    cout << "5. Back\n";
    cout << "Choice: ";
    
    int subChoice;
    if (!(cin >> subChoice)) {
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }
    cin.ignore();
    
    string nameInput;
    string pathInput;

    if (subChoice == 1) {
        cout << "\nEnter document name (e.g., 'notes'): ";
        getline(cin, nameInput);
        if (nameInput.empty()) return;

        if (nameInput.find(".") == string::npos) {
            nameInput += ".txt";
        }
        
        ensureDocsDir();
        string finalPath = "documents/" + nameInput;
        
        if (fs::exists(finalPath)) {
            cout << RED << "Error: File already exists. Use 'Update' to edit.\n" << RESET;
        } else {
            ofstream outfile(finalPath);
            outfile.close();
            cout << "Created new file: " << finalPath << "\n";
            
            cout << YELLOW << "Opening " << finalPath << " in Notepad...\n" << RESET;
            string cmd = "notepad \"" + finalPath + "\"";
            system(cmd.c_str());
            
            cout << GREEN << "Indexing document...\n" << RESET;
            engine.addDocument(finalPath);
        }
        
    } else if (subChoice == 2) {
        cout << "\nEnter path to file (file.txt) or folder: ";
        getline(cin, pathInput);
        if (pathInput.empty()) return;
        
        if (fs::is_directory(pathInput)) {
             cout << "Scanning directory for .txt files...\n";
             int count = 0;
             for (const auto& entry : fs::directory_iterator(pathInput)) {
                 if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                     engine.addDocument(entry.path().string());
                     count++;
                 }
             }
             cout << "Added " << count << " files.\n";
        } else {
             if (!fs::exists(pathInput)) {
                 cout << RED << "Error: File not found.\n" << RESET;
             } else {
                 engine.addDocument(pathInput);
             }
        }

    } else if (subChoice == 3) {
        cout << "\nEnter DocID or Filename to remove: ";
        getline(cin, nameInput);
        if (!nameInput.empty()) engine.removeDocument(nameInput);
        
    } else if (subChoice == 4) {
        cout << "\nEnter DocID or Filename to update: ";
        getline(cin, nameInput);
        if (!nameInput.empty()) {
            int docId = -1;
            string path;
            
            auto docs = engine.getDocumentManager().getAllDocuments();
            for (const auto& doc : docs) {
                if (to_string(doc.id) == nameInput || doc.filename == nameInput || doc.filename == nameInput + ".txt") {
                    path = doc.path;
                    break;
                }
            }
            
            if (!path.empty()) {
                cout << YELLOW << "Opening " << path << " in Notepad...\n" << RESET;
                string cmd = "notepad \"" + path + "\"";
                system(cmd.c_str());
                
                cout << GREEN << "Re-indexing document...\n" << RESET;
                engine.updateDocument(nameInput);
            } else {
                cout << RED << "Document not found.\n" << RESET;
            }
        }
    }
    
    if (subChoice != 5) pause();
}

string highlightTerms(string text, const string& query) {
    if (query.empty()) return text;
    
    vector<string> terms = Utils::split(Utils::toLower(query), ' ');
    string lowerText = Utils::toLower(text);
    string result = "";
    size_t lastPos = 0;
    
    for (size_t i = 0; i < text.length(); ++i) {
        bool matchFound = false;
        for (const auto& term : terms) {
            string cleanTerm = Utils::cleanToken(term);
            if (cleanTerm.empty()) continue;
            
            if (i + cleanTerm.length() <= text.length()) {
                string sub = lowerText.substr(i, cleanTerm.length());
                if (sub == cleanTerm) {
                     
                    result += RED;
                    result += text.substr(i, cleanTerm.length());
                    result += CYAN;
                    i += cleanTerm.length() - 1;
                    matchFound = true;
                    break;
                }
            }
        }
        if (!matchFound) {
            result += text[i];
        }
    }
    return result;
}

void handleSearch(SearchEngine& engine) {
    printHeader();
    engine.sync();
    cout << BOLD << "--- Search ---\n" << RESET;
    cin.ignore();
    string query;
    cout << "Enter search query: ";
    getline(cin, query);
    
    auto results = engine.search(query);
    
    cout << "\nFound " << BOLD << results.size() << RESET << " results:\n";
    cout << "--------------------------------------------------------\n";
    
    int rank = 1;
    for (const auto& res : results) {
        cout << BOLD << rank++ << ". " << res.doc.filename << RESET 
                  << " (Score: " << fixed << setprecision(1) << res.score << ")\n";
        
        if (!res.snippet.empty()) {
            cout << "   Snippet: " << CYAN << "\"" << highlightTerms(res.snippet, query) << "\"" << RESET << "\n";
        }
        cout << "\n";
    }
    pause();
}

void handleAnalytics(SearchEngine& engine) {
    printHeader();
    const auto& analytics = engine.getAnalytics();
    
    cout << BOLD << "--- Search Analytics ---\n" << RESET;
    cout << YELLOW << "Top 5 Searched Terms:\n" << RESET;
    auto terms = analytics.getTopTerms(5);
    for (const auto& pair : terms) {
        cout << " - " << pair.first << ": " << BOLD << pair.second << RESET << "\n";
    }
    
    cout << "\n" << YELLOW << "Recent Search History:\n" << RESET;
    auto history = analytics.getHistory();
    auto it = (history.size() > 10) ? (history.end() - 10) : history.begin();
    for (; it != history.end(); ++it) {
         cout << " - " << it->query << "\n";
    }
    pause();
}

void handleShowAll(SearchEngine& engine) {
    printHeader();
    engine.sync();
    auto docs = engine.getDocumentManager().getAllDocuments();
    cout << BOLD << "--- Indexed Documents (" << docs.size() << ") ---\n" << RESET;
    cout << left << setw(6) << "ID" 
              << setw(20) << "Name" 
              << "Path\n";
    cout << "--------------------------------------------------------\n";
    for (const auto& doc : docs) {
        cout << left << setw(6) << doc.id 
                  << setw(20) << (doc.filename.length() > 18 ? doc.filename.substr(0,15)+"..." : doc.filename)
                  << CYAN << doc.path << RESET << "\n";
    }
    pause();
}

int main() {
    system("color"); 

    SearchEngine engine;

    int choice;
    while (true) {
        printMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 6) break;

        switch (choice) {
            case 1:
                handleDocumentMenu(engine);
                break;
            case 2:
                handleSearch(engine);
                break;
            case 3:
                handleShowAll(engine);
                break;
            case 4:
                handleAnalytics(engine);
                break;
            case 5:
                char confirm;
                cout << RED << "WARNING: This will delete all indexed data and history. (Files will remain on disk)\n" << RESET;
                cout << "Are you sure? (y/n): ";
                cin >> confirm;
                if (confirm == 'y' || confirm == 'Y') {
                    engine.reset();
                } else {
                    cout << "Reset cancelled.\n";
                }
                pause();
                break;
            default:
                cout << "Invalid choice.\n";
                pause();
        }
    }

    return 0;
}
