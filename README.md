# A.I.M. Tools
This repository contains different tools for A.I.M. games.

# List of tools
1. AIM 1 unpaker. Unpacks any .pak archive from AIM1 game.
2. DB extractor. Converts (db|quest) databases from any AIM game into .sql file to be executed with sqlite3 DBMS.
3. OBJ extractor. Extracts all data about objects on the map from .mmo file. Tested only with AIM1 game.
4. MMP extractor (not working, only template).
5. Models converter.
6. Script to TXT convertor.

# Build instructions
1. Download and install latest CMake http://www.cmake.org/download/
2. Clone this repository
3. Open the source directory
4. Execute in console `cmake -H. -Bwin`
5. Run win/aim_tools.sln file (Visual Studio solution file)
6. Build the project
