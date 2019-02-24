
#include "Map.h"

Map::Map() {

}

void Map::Setup(const int& w, const int& h) {
    width = w;
    height = h;

    layout.reserve(w * h);
}
