
#include "Component.h"
#include "GameObject.h"

Component::~Component() {
	delete gameObject;
}