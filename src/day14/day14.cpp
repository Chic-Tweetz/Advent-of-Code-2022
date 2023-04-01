
#define TESTINPUT
// #define DEBUG

#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

#include "utils.h"

struct Coord
{
    using Coord_t = int;

    Coord() : x{ 0 }, y{ 0 } {}
    Coord(Coord_t _x, Coord_t _y) : x{ _x }, y{ _y } {}

    Coord_t x;
    Coord_t y;

    Coord abs()
    {
        return { x < 0 ? x * - 1 : x, y < 0 ? y * - 1 : y };
    }

    Coord normalised()
    {
        return *this / abs();
    }

    friend Coord operator/(const Coord &lhs, const Coord &rhs)
    {
        return { lhs.x / rhs.x, lhs.y / rhs.y };
    }

    friend Coord operator+(const Coord &lhs, const Coord &rhs)
    {
        return { lhs.x + rhs.x, lhs.y + rhs.y };
    }
    friend Coord operator-(const Coord &lhs, const Coord &rhs)
    {
        return { lhs.x - rhs.x, lhs.y - rhs.y };
    }

    friend bool operator==(const Coord &lhs, const Coord &rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    friend bool operator!=(const Coord &lhs, const Coord &rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const Coord &lhs, const Coord &rhs)
    {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
    }
    friend bool operator>(const Coord &lhs, const Coord &rhs)
    {
        return lhs.x > rhs.x || (lhs.x == rhs.x && lhs.y > rhs.y);
    }

};

std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    return os << "{ " << coord.x << ", " << coord.y << " }";
}

// Recurse through the line while fnc == true
void forEachOnLine(const Coord &coord, const Coord &step, const Coord &end, std::function<bool(const Coord&)> fnc)
{
    // Uses a bool returning std::function so we can return early if it returns false
    // Specifically so we can stop if falling sand has a collision
    if (coord != end && fnc(coord))
    {
        forEachOnLine(coord + step, step, end, fnc);
    }
}

// Include the first, exclude the last
void forEachOnLine(const Coord &start, const Coord &end, std::function<bool(const Coord&)> fnc)
{
    const Coord step{ (end - start).normalised() };
    forEachOnLine(start, step, end, fnc);
}

// Skip the first one, include the last one (skip the first because it'll be added to the map twice elsewise)
void forEachOnLineSkipFirst(const Coord &start, const Coord &end, std::function<bool(const Coord&)> fnc)
{
    const Coord step{ (end - start).normalised() };
    forEachOnLine(start + step, step, end + step, fnc);
}

namespace Puzzle1
{
    // Return false when sand falls into the void or true if it settles
    bool sandFall(std::set<Coord> &cave, const Coord &sand)
    {
        static const Coord::Coord_t leftBound{ (*(cave.begin())).x };
        static const Coord::Coord_t rightBound{ (*(cave.rbegin())).x };
        // Just finding the largest y value from the cave stones
        static const Coord::Coord_t lowerBound{
            (*(std::max_element(cave.begin(), cave.end(),
            [](const Coord& a, const Coord& b)
                {
                    return a.y < b.y;
                }))).y };

        if (sand.x < leftBound || sand.x > rightBound || sand.y > lowerBound)
        {
            return false;
        }
            
        // Fall straight down if there's room
        if (!cave.contains(sand + Coord{ 0, 1 }))
        {
            return sandFall(cave, sand + Coord{ 0 , 1 });
        }

        // Left-down
        if (!cave.contains(sand + Coord{ -1, 1 }))
        {
            return sandFall(cave, sand + Coord{ -1, 1 });
        }

        // Right-down
        if (!cave.contains(sand + Coord{ 1, 1 }))
        {
            return sandFall(cave, sand + Coord{ 1, 1 });
        }

        // Settle
        cave.insert(sand); 
        return true;
        
    }

	void solve(const std::string& infile)
	{
        std::set<Coord> caveMap;

        // Firstly, get the coords from the input file to build a cave map
        utils::forEachLine(infile,
            [&caveMap](std::string_view line)
            {
                bool bFirst{ true };
                Coord start{}; 

                // We get each coord and fill the map with the lines between them
                utils::doOnSplit(line, " -> ",
                    [&bFirst, &start, &caveMap](std::string_view coordStr)
                    {
                        const auto comma{ coordStr.find(',') };
                        if (bFirst)
                        {
                            start = {
                                std::atoi(coordStr.data()),
                                std::atoi(coordStr.data() + comma + 1)
                            };

                            caveMap.insert(start);
                            bFirst = false;
                        }
                        else
                        {
                            Coord end = {
                                std::atoi(coordStr.data()),
                                std::atoi(coordStr.data() + comma + 1)
                            };
                            
                            // Ahhhhh I didn't mean to nest so many lambdas
                            forEachOnLineSkipFirst(start, end,
                            [&caveMap](const Coord& coord)
                            {
                                caveMap.insert(coord);
                                return true;
                            });

                            start = end;
                        }
                    });
            });

        const Coord sandStart{ 500, 0 };

        int unitsOfSand{ 0 };
        
        while(sandFall(caveMap, sandStart))
        {
            ++unitsOfSand;
        }

#ifdef TESTINPUT
        utils::printAnswer(unitsOfSand, 24, "Total of: ", " units of sand before abyssal fall");
#else
        utils::printAnswer(unitsOfSand, 728, "Total of: ", " units of sand before abyssal fall");
#endif

	}
};

namespace Puzzle2
{
    // Return false when sand comes to rest at 500, 0
    bool sandFall(std::set<Coord> &cave, const Coord &sand)
    {
        // Just finding the largest y value from the cave stones
        static const Coord::Coord_t caveFloor{
            (*(std::max_element(cave.begin(), cave.end(),
            [](const Coord& a, const Coord& b)
                {
                    return a.y < b.y;
                }))
            ).y + 2 };

        if (sand.y == caveFloor - 1)
        {
            cave.insert(sand);
            return true;
        }

        // Fall straight down if there's room
        if (!cave.contains(sand + Coord{ 0, 1 }))
        {
            return sandFall(cave, sand + Coord{ 0 , 1 });
        }

        // Left-down
        if (!cave.contains(sand + Coord{ -1, 1 }))
        {
            return sandFall(cave, sand + Coord{ -1, 1 });
        }

        // Right-down
        if (!cave.contains(sand + Coord{ 1, 1 }))
        {
            return sandFall(cave, sand + Coord{ 1, 1 });
        }

        if (sand == Coord{ 500, 0 })
        {
            return false;
        }

        // Settle
        cave.insert(sand); 
        return true;
        
    }

	void solve(const std::string& infile)
	{
        std::set<Coord> caveMap;
        utils::forEachLine(infile,
            [&caveMap](std::string_view line)
            {
                bool bFirst{ true };
                Coord start{}; 

                utils::doOnSplit(line, " -> ",
                    [&bFirst, &start, &caveMap](std::string_view coordStr)
                    {
                        const auto comma{ coordStr.find(',') };
                        if (bFirst)
                        {
                            start = {
                                std::atoi(coordStr.data()),
                                std::atoi(coordStr.data() + comma + 1)
                            };

                            caveMap.insert(start);
                            bFirst = false;
                        }
                        else
                        {
                            Coord end = {
                                std::atoi(coordStr.data()),
                                std::atoi(coordStr.data() + comma + 1)
                            };

                            forEachOnLineSkipFirst(start, end,
                            [&caveMap](const Coord& coord) // Ahhhhh I didn't mean to nest so many lambdas
                            {
                                caveMap.insert(coord);
                                return true;
                            });

                            start = end;
                        }
                    });
            });

        const Coord sandStart{ 500, 0 };

        // Starting from 1 'cause we were off by 1!
        int unitsOfSand{ 1 };
        
        while(sandFall(caveMap, sandStart))
        {
            ++unitsOfSand;
        }

#ifdef TESTINPUT
        utils::printAnswer(unitsOfSand, 93, "Total of: ", " units of sand before cave is full");
#else
        utils::printAnswer(unitsOfSand, 27623, "Total of: ", " units of sand before cave is full");
#endif

    }
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}