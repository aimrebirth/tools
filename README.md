# A.I.M. Tools
This repository contains different tools for A.I.M. games.

# List of tools
1. AIM unpaker. Unpacks any .pak archive from AIM1/AIM2/AIM:R games.
1. AIM paker. Makes archive for AIM1 game (without compression).
1. DB extractor. Converts (db|quest) databases from any AIM game into .sql file to be executed with sqlite3 DBMS.
1. MMO extractor (object extractor). Extracts all data about objects on the map from .mmo file.
1. MMP extractor. Extract texture-, alpha-, height- maps etc.
1. MMM extractor. Minimap -> BMP.
1. Models converter: AIM1/2 format -> .OBJ + .MTL. Textures are included.
1. Script to TXT convertor.
1. TXT to Script convertor.
1. Texture converter: TM -> TGA.
1. MPJ loader (dummy implementation).
1. Save loader (dummy implementation).
1. AIM1 mod maker. Makes routine actions for you.

# Build instructions
1. Download and add to PATH latest SW https://software-network.org/
2. Clone this repository
3. Open the source directory
4. Execute in console `sw generate`
5. Open generated solution file (Visual Studio solution file)
6. Build the project
