
#include "Key.h"

Key::Key(const std::string& k) {
    password = k;
    componentType = "key";
}

Key* Key::clone() const {
    return new Key(*this);
}