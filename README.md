# LuaLearning
## Compile
g++ lua.cpp -o test \`pkg-config --cflags --libs luajit\` -std=c++11