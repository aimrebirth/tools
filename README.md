# A.I.M. Tools
This repository contains different tools for A.I.M. games.

# List of tools
1. AIM 1 unpaker. Unpacks any .pak archive from AIM1 game.
2. DB extractor. Converts (db|quest) databases from any AIM game into .sql file to be executed with sqlite3 DBMS.
3. MMO extractor (object extractor). Extracts all data about objects on the map from .mmo file.
4. MMP extractor. Extract texture-, alpha-, height- maps etc.
5. MMM extractor. Minimap -> BMP.
6. Models converter: AIM1/2 format -> .OBJ + .MTL. Textures are included.
7. Script to TXT convertor.
8. Texture converter: TM -> TGA.

# Build instructions
1. Download and install latest CMake http://www.cmake.org/download/
2. Clone this repository
3. Open the source directory
4. Execute in console `cmake -H. -Bwin`
5. Run win/aim_tools.sln file (Visual Studio solution file)
6. Build the project
