
#ifndef DOOR_H_
#define DOOR_H_

#include "Component.h"
#include "Collider.h"
#include "Key.h"

class Door : public Component {

private:
    std::string password;
    bool open;

public:
    Door() {}
    Door(const std::string&);
    Door* clone() const;

    void Update(const float&);
    std::string Password() const { return password; }
    bool IsOpen() const { return open; }

};

#endif