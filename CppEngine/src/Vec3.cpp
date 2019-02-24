
#include "Vec3.h"

// Constructors and Destructors

Vec3::Vec3() : x(0), y(0), z(0) {}

Vec3::Vec3(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vec3::~Vec3() {

}


// Operators

Vec3 Vec3::operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

Vec3 Vec3::operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
}

Vec3 Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
}

Vec3 Vec3::operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
}

Vec3 Vec3::operator*(const float f) const {
    return Vec3(x * f, y * f, z * f);
}
Vec3 operator*(const float f, const Vec3& v) {
    return v * f;
}

Vec3 Vec3::operator*(const int i) const {
    return Vec3(x * i, y * i, z * i);
}
Vec3 operator*(const int i, const Vec3& v) {
    return v * i;
}

Vec3 Vec3::operator*(const Vec3& v) const {
    return Vec3(x * v.x, y * v.y, z * v.z);
}

Vec3 Vec3::operator/(const float f) const {
    return Vec3(x / f, y / f, z / f);
}

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator=(const Vec3& v) {
    if (this == &v) return *this;
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
}

bool Vec3::operator==(const Vec3& v) const {
    if (this == &v) return true;

    if (this->x != x) return false;
    if (this->y != y) return false;
    if (this->z != z) return false;

    return true;
}

bool Vec3::operator!=(const Vec3& v) const {
    if (this == &v) return false;

    if (this->x == x && this->y == y && this->z == z) return false;

    return true;
}


// Vec3 Functions

float Vec3::Dot(const Vec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vec3 Vec3::Cross(const Vec3& v) const {
    return
        Vec3(y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
}

float Vec3::Length() const {
    return (float)sqrt(Dot(*this));
}

float Vec3::AngleBetween(const Vec3& v, bool degrees) const {
    float angle = (float)acos((Dot(v)) / (Length() * v.Length()));

    if (degrees)
        angle *= 180.0f / (float)M_PI;

    return angle;
}

Vec3 Vec3::Normalize() const {
    float l = Length();
    return Vec3(x / l, y / l, z / l);
}

Vec3 Vec3::Reflection(const Vec3& n) const {
    return 2 * (Dot(n)) * n - *(this);
}

double Vec3::Distance(const Vec3& v) const {
    float dx = x - v.x;
    float dy = y - v.y;
    float dz = z - v.z;
    return sqrt(dx*dx + dy * dy + dz * dz);
}
