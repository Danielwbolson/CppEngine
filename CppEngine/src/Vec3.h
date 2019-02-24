
#ifndef VEC_3_H_
#define VEC_3_H_

#define _USE_MATH_DEFINES
#include <math.h>

class Vec3 {

public:
    Vec3();
    Vec3(float x, float y, float z);
    ~Vec3();

    float x, y, z;

    // Operators
    Vec3 operator+(const Vec3&) const;
    Vec3 operator-(const Vec3&) const;
    Vec3 operator+=(const Vec3&);
    Vec3 operator-=(const Vec3&);
    Vec3 operator*(const float) const;
    Vec3 operator*(const int) const;
    Vec3 operator*(const Vec3&) const;
    Vec3 operator/(const float) const;
    Vec3 operator-() const;
    Vec3 operator=(const Vec3&);
    bool operator==(const Vec3&) const;
    bool operator!=(const Vec3&) const;

    // Vec3 functions
    float Dot(const Vec3&) const;
    Vec3 Cross(const Vec3&) const;
    float Length() const;
    float AngleBetween(const Vec3&, bool degrees = false) const;
    Vec3 Normalize() const;
    Vec3 Reflection(const Vec3&) const;
    double Distance(const Vec3&) const;
};

Vec3 operator*(const int, const Vec3&);
Vec3 operator*(const float, const Vec3&);

#endif