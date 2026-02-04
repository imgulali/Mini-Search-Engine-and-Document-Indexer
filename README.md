# Minie

**Minie** is a lightweight, high-performance command-line search engine built in C++ specifically for the DSA Lab Project. It allows you to index local text documents, perform full-text searches with relevance scoring, and view search analytics, all from a unified terminal interface.

## Project Structure

```
Minie/
├── CMakeLists.txt       # Build configuration
├── README.md            # Project documentation
├── src/                 # Source code
│   ├── main.cpp         # Entry point
│   ├── ...
└── includes/            # Header files
```

## Build Instructions (using g++)

Since you are using `g++`, you can compile the project directly from the terminal without needing CMake.

1. Open your terminal in the project root folder.

2. Run the following command:
   ```bash
   g++ -std=c++17 -I includes src/*.cpp -o minie
   ```

3. Run the executable:
     ```powershell
     .\minie.exe
     ```
