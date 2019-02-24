
#ifndef VEC_2_H_
#define VEC_2_H_

#define _USE_MATH_DEFINES
#include <math.h>

class Vec2 {

public:
    Vec2();
    Vec2(float x, float y);
    ~Vec2();

    float x, y;

    // Operators
    Vec2 operator+(const Vec2&) const;
    Vec2 operator-(const Vec2&) const;
    Vec2 operator+=(const Vec2&);
    Vec2 operator-=(const Vec2&);
    Vec2 operator*(const float) const;
    Vec2 operator*(const int) const;
    Vec2 operator*(const Vec2&) const;
    Vec2 operator/(const float) const;
    Vec2 operator-() const;
    Vec2 operator=(const Vec2&);

    // Vector functions
    float Length() const;
    double Distance(const Vec2&) const;
};

Vec2 operator*(const int, const Vec2&);
Vec2 operator*(const float, const Vec2&);

#endif