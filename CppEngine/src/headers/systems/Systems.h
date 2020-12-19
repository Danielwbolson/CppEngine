
#ifndef SYSTEMS_H_
#define SYSTEMS_H_

#include <vector>
#include <string>

class Component;

class Systems {
protected:
    std::string componentType;

public:
	virtual ~Systems() {}

    virtual void Setup() = 0;
    virtual void Register(const Component*) = 0;

    virtual void Update(const float&) = 0;
    virtual void Render() = 0;
};

#endif
