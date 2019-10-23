
#ifndef KEY_H_
#define KEY_H_

#include "Component.h"

class Key : public Component {

private:
    std::string password;

public:
    Key() {}
    Key(const std::string&);
    Key* clone() const;

    std::string Password() const { return password; }
};

#endif
