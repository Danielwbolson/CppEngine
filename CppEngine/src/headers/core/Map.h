
#ifndef MAP_H_
#define MAP_H_

#include <vector>

class Map {

private:
    int width, height;

public:
    std::vector<int> layout;

    Map();
    int index(const int& i, const int& j) { return i * width + j; }
    void Setup(const int&, const int&);

};
#endif 