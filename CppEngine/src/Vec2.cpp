
#include "Vec2.h"

// Constructors and Destructors

Vec2::Vec2() : x(0), y(0) {}

Vec2::Vec2(float x, float y) {
    this->x = x;
    this->y = y;
}

Vec2::~Vec2() {

}


// Operators

Vec2 Vec2::operator+(const Vec2& v) const {
    return Vec2(x + v.x, y + v.y);
}

Vec2 Vec2::operator-(const Vec2& v) const {
    return Vec2(x - v.x, y - v.y);
}

Vec2 Vec2::operator+=(const Vec2& v) {
    x += v.x;
    y += v.y;

    return *this;
}

Vec2 Vec2::operator-=(const Vec2& v) {
    x -= v.x;
    y -= v.y;

    return *this;
}

Vec2 Vec2::operator*(const float f) const {
    return Vec2(x * f, y * f);
}
Vec2 operator*(const float f, const Vec2& v) {
    return v * f;
}

Vec2 Vec2::operator*(const int i) const {
    return Vec2(x * i, y * i);
}
Vec2 operator*(const int i, const Vec2& v) {
    return v * i;
}

Vec2 Vec2::operator*(const Vec2& v) const {
    return Vec2(x * v.x, y * v.y);
}

Vec2 Vec2::operator/(const float f) const {
    return Vec2(x / f, y / f);
}

Vec2 Vec2::operator-() const {
    return Vec2(-x, -y);
}

Vec2 Vec2::operator=(const Vec2& v) {
    if (this == &v) return *this;
    this->x = v.x;
    this->y = v.y;
    return *this;
}


// Vector Functions

float Vec2::Length() const {
    return (float)sqrt(x * x + y * y);
}

double Vec2::Distance(const Vec2& v) const {
    float dx = x - v.x;
    float dy = y - v.y;
    return sqrt(dx*dx + dy * dy);
}