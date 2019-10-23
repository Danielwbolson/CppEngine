
#include "Component.h"
#include "GameObject.h"

Component::~Component() {
	gameObject = nullptr;
}