#pragma once

#include <cmath>
#include <iostream>

struct Vector2d
{
    Vector2d(int xy = 0) : x{ xy }, y{ xy } {}
    Vector2d(int _x, int _y) : x{ _x }, y{ _y } {}

    int x;
    int y;

    Vector2d rotateClockwise()
    {
        return { y, -x };
    }

    Vector2d rotateAntiClockwise()
    {
        return { -y, x };
    }

    int static manhatDist(const Vector2d &lhs, const Vector2d &rhs)
    {
        return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
    }

    friend Vector2d operator+(const Vector2d &lhs, const Vector2d &rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y };
    }

    friend Vector2d operator-(const Vector2d &lhs, const Vector2d &rhs)
    {
        return {lhs.x - rhs.x, lhs.y - rhs.y };
    }

    Vector2d operator*(int s)
    {
        return { x * s, y * s };
    }

    Vector2d operator/(int s)
    {
        return { x / s, y / s };
    }

    int operator[](int i)
    {
        return i == 0 ? x : y;
    }

    Vector2d& operator+=(const Vector2d &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2d& operator-=(const Vector2d &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    friend bool operator==(const Vector2d &lhs,const Vector2d& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend bool operator!=(const Vector2d &lhs,const Vector2d& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector2d& vec)
    {
        return os << "{ " << vec.x << ", " << vec.y << " }";
    }

};

template <typename T>
struct Vec2dT
{
    Vec2dT(T xy = 0) : x{ xy }, y{ xy } {}
    Vec2dT(T _x, T _y) : x{ _x }, y{ _y } {}

    T x;
    T y;

    Vec2dT rotateClockwise()
    {
        return { y, -x };
    }

    Vec2dT rotateAntiClockwise()
    {
        return { -y, x };
    }

    T static manhatDist(const Vec2dT &lhs, const Vec2dT &rhs)
    {
        return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
    }

    friend Vec2dT operator+(const Vec2dT &lhs, const Vec2dT &rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y };
    }

    friend Vec2dT operator-(const Vec2dT &lhs, const Vec2dT &rhs)
    {
        return {lhs.x - rhs.x, lhs.y - rhs.y };
    }

    Vec2dT operator*(T s)
    {
        return { x * s, y * s };
    }

    Vec2dT operator/(T s)
    {
        return { x / s, y / s };
    }

    T operator[](T i)
    {
        return i == 0 ? x : y;
    }

    Vec2dT& operator+=(const Vec2dT &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2dT& operator-=(const Vec2dT &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    friend bool operator==(const Vec2dT &lhs,const Vec2dT& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend bool operator!=(const Vec2dT &lhs,const Vec2dT& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec2dT& vec)
    {
        return os << "{ " << vec.x << ", " << vec.y << " }";
    }

};
