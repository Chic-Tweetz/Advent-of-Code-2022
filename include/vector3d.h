#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

struct Vector3d
{
    Vector3d(int xyz = 0) : x{ xyz }, y{ xyz }, z{ xyz } {}
    Vector3d(int _x, int _y, int _z) : x{ _x }, y{ _y }, z{ _z } {}
    Vector3d(const std::vector<int> &arr) : x{ arr[0] }, y{ arr[1] }, z{ arr[2] } {}
    Vector3d(const std::vector<std::string> &arr) : x{ std::stoi(arr[0]) }, y{ std::stoi(arr[1]) }, z{ std::stoi(arr[2]) } {}

    template<typename T>
    Vector3d(T _x, T _y, T _z) : x{ static_cast<int>(_x) }, y{ static_cast<int>(_y) }, z{ static_cast<int>(_z) } {}


    int x;
    int y;
    int z;

    void xyzMax(const Vector3d &rhs)
    {
        x = rhs.x > x ? rhs.x : x;
        y = rhs.y > y ? rhs.y : y;
        z = rhs.z > z ? rhs.z : z;
    }

    friend Vector3d operator+(const Vector3d &lhs, const Vector3d &rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    }

    friend Vector3d operator+(const Vector3d &lhs, int rhs)
    {
        return {lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
    }

    friend Vector3d operator-(const Vector3d &lhs, const Vector3d &rhs)
    {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
    }

    Vector3d operator*(int s)
    {
        return { x * s, y * s, z * s };
    }

    Vector3d operator/(int s)
    {
        return { x / s, y / s, z / s };
    }

    Vector3d& operator+=(const Vector3d &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vector3d& operator-=(const Vector3d &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    friend bool operator==(const Vector3d &lhs,const Vector3d& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }

    friend bool operator!=(const Vector3d &lhs,const Vector3d& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector3d& vec)
    {
        return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";
    }
};
