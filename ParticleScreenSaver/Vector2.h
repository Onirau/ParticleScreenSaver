#ifndef VECTOR2_H
#define VECTOR2_H

class IntVector2 {
public:
    int x, y;

    // Constructors
    IntVector2() : x(0), y(0) {}
    IntVector2(int xVal, int yVal) : x(xVal), y(yVal) {}

    // Addition
    IntVector2 operator+(const IntVector2& other) const {
        return IntVector2(x + other.x, y + other.y);
    }

    // Subtraction
    IntVector2 operator-(const IntVector2& other) const {
        return IntVector2(x - other.x, y - other.y);
    }

    // Scalar multiplication
    IntVector2 operator*(int scalar) const {
        return IntVector2(x * scalar, y * scalar);
    }

    // Scalar division
    IntVector2 operator/(int scalar) const {
        if (scalar != 0) {
            return IntVector2(x / scalar, y / scalar);
        }
        else {
            // Handle division by zero
            // You may want to throw an exception or handle it in some other way
            return IntVector2();
        }
    }

    // Comparison
    bool operator==(const IntVector2& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const IntVector2& other) const {
        return !(*this == other);
    }
};


class Vector2 {
public:
    float x, y;

    // Constructors
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float xVal, float yVal) : x(xVal), y(yVal) {}

    // Addition
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    // Subtraction
    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    // Scalar multiplication
    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    // Scalar division
    Vector2 operator/(float scalar) const {
        if (scalar != 0.0f) {
            return Vector2(x / scalar, y / scalar);
        }
        else {
            // Handle division by zero
            // You may want to throw an exception or handle it in some other way
            return Vector2();
        }
    }

    // Comparison
    bool operator==(const Vector2& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }
};

#endif // !1