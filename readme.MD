## *STANDALONE* Installation / How to build
```bash
cd /App/Scripts/interpreter
git clone https://github.com/LigaInsider/interpreter.git
cd interpreter
g++ -O3 -std=c++17 -fopenmp -o interpreter main.cpp
chmod +x 777 interpreter
```