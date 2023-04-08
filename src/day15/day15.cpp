
// #define DEBUG
// #define TESTINPUT

#include <array>
#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "utils.h"

struct Coord // I keep making Coord structs. Why don't I just copy and paste?!
{
    using type = long; // Changing int to long lets me do the * 4000000 thing at the end hehe
    
    Coord(type _x, type _y) : x{ _x }, y{ _y } {}
    
    Coord(std::string_view str, bool bSearchBackwards = false)
    {
        if (!bSearchBackwards)
        {
            x = std::atoi( str.data() + str.find('x') + 2 );
            y = std::atoi( str.data() + str.find('y') + 2 );
        }
        else
        {
            x = std::atoi( str.data() + str.find_last_of('x') + 2 );
            y = std::atoi( str.data() + str.find_last_of('=') + 1 ); 
        }
    }

    type x;
    type y; 

    type dist(const Coord &rhs)
    {
        return dist(*this, rhs);
    }

    friend Coord operator+(const Coord &lhs, const Coord &rhs)
    {
        return Coord{ lhs.x + rhs.x, lhs.y + rhs.y };
    }

    friend Coord operator-(const Coord &lhs, const Coord &rhs)
    {
        return Coord{ lhs.x - rhs.x, lhs.y - rhs.y };
    }

    friend Coord operator*(const Coord &lhs, const int i)
    {
        return { lhs.x * i, lhs.y * i };
    }

    // Manhattan distance between two coords
    static type dist(const Coord &lhs, const Coord &rhs)
    {
        return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
    }

    friend bool operator<(const Coord &lhs, const Coord &rhs)
    {
        return lhs.x < rhs.x || ( lhs.x == rhs.x && lhs.y < rhs.y );
    }

    Coord& operator+=(const Coord &add)
    {   
        x += add.x;
        y += add.y;
        return *this;
    }

    friend bool operator==(const Coord &lhs, const Coord &rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend Coord operator/(const Coord &lhs, const Coord &rhs)
    {
        return { lhs.x / rhs.x, lhs.y / rhs.y };
    }

    Coord abs()
    {
        return { std::abs(x), std::abs(y) }; 
    }

    Coord normalised()
    {
        return *this / (*this).abs();
    }

    static Coord direction(const Coord &from, const Coord &to)
    {
        return (to - from) / (to - from).abs();
    }

    Coord direction(const Coord &to)
    {
       return direction(*this, to);
    }
};

struct MinMax
{
    using minmax_t = Coord::type;

    MinMax(const minmax_t _min, const minmax_t _max) : min{ _min }, max{ _max } {}
    minmax_t min;
    minmax_t max;

    minmax_t coverage() const
    {
        return 1 + max - min;
    }

    static bool overlaps(const MinMax &lhs, const MinMax &rhs)
    {
        return lhs.min <= rhs.max && lhs.max >= rhs.min;
    }

    bool overlaps(const MinMax &other) const
    {
        return overlaps(*this, other);
    }

    bool overlaps(const MinMax::minmax_t &other) const
    {
        return other >= this->min && other <= this->max;
    }

    friend bool operator<(const MinMax &lhs, const MinMax &rhs)
    {
        return lhs.min < rhs.min || ( lhs.min == rhs.min && lhs.max < rhs.max );
    }
};

std::ostream& operator<<(std::ostream &os, const Coord &coord)
{
    return os << "{ " << coord.x << ", " << coord.y << " }";
}

std::ostream& operator<<(std::ostream &os, const MinMax &range)
{
    return os << "{ " << range.min << ", " << range.max << " }";
}

// Keep track of ranges covered by sensors on a particular row
class MinMaxRanges
{
    using container_t = std::vector<MinMax>;
    using citer_t = container_t::const_iterator;

    container_t m_ranges;
    std::set<MinMax::minmax_t> m_beacons{ };

    // Initialising like this so we can get true mins and max later
    MinMax m_extremes{ INT_MAX, INT_MIN };

    // Trying to practice some functional functions every now and then
    MinMax::minmax_t calculateCoverage(const container_t::const_iterator it, const MinMax::minmax_t acc = 0) const
    {
        if (it == m_ranges.end())
        {
            return acc;
        }

        return calculateCoverage(it + 1, acc + (*it).coverage());
    }

public:
    MinMaxRanges() {}
    MinMaxRanges(MinMax extremes) : m_extremes{ extremes } {}
    void addBeacon(const MinMax::minmax_t xPos)
    {
        m_beacons.insert(xPos);
    }

    void insert(MinMax insertion)
    { 
        DPRINTLN("Inserting: " << insertion);
        // Expand ranges when inserting an overlapping range
        auto erase{ std::remove_if(m_ranges.begin(), m_ranges.end(),
            [&insertion](auto &&existing)
            {
                if (existing.overlaps(insertion))
                {
                    DPRINT("removing: " << existing << ", expanding range: "); 

                    // Update min / max ranges when overlaps occur 
                    insertion.min = std::min(insertion.min, existing.min);
                    insertion.max = std::max(insertion.max, existing.max);

                    DPRINTLN(insertion);

                    return true;
                }
                return false;
            }) };
        m_ranges.erase(erase, m_ranges.end());
        m_ranges.push_back(insertion);
        
        // For skipping beacon overlap checks later
        m_extremes.min = std::min(insertion.min, m_extremes.min);
        m_extremes.max = std::max(insertion.max, m_extremes.max);
    }

    MinMax::minmax_t countOverlappingBeacons() const
    {
        MinMax::minmax_t count{ 0 }; 
        for (auto beacon : m_beacons)
        {
            // We can early return when our beacons are out of range
            if (beacon > m_extremes.max) 
            {
                break;
            }

            for (auto && range : m_ranges)
            {
                if (range.overlaps(beacon))
                {
                    ++count;
                }
            }
        }
        return count;
    }

    MinMax::minmax_t calculateCoverage() const
    {
        return calculateCoverage(m_ranges.cbegin(),  0) - countOverlappingBeacons();
    }

    void printAll()
    {
        for (auto &&range : m_ranges)
        {
            std::cout << range << '\n';
        }
    }

};

// Unused
int numberFromInput(std::string_view line, const size_t index = 11)
{
    if (line[index] == '=')
    {
        return std::atoi(line.data() + index + 1);
    }

    return numberFromInput(line, index + 1);
}

void parseInput(std::string_view line, MinMaxRanges &coverage)
{
    #ifdef TESTINPUT
    const Coord::type rowToCheck{ 10 };
    #else
    const Coord::type rowToCheck{ 2000000 };
    #endif

    const Coord sensor{ line };
    const Coord beacon{ line, true };

    const auto rowDist{ std::abs(sensor.y - rowToCheck) };

    if (beacon.y == rowToCheck)
    {
        coverage.addBeacon(beacon.x);
    }

    const auto beaconDist { Coord::dist(sensor, beacon) };

    // Double whatever's left from the y difference, as it spreads left and right equally
    const auto excessCoverage{ beaconDist - rowDist };

    if (excessCoverage < 0)
    {
        return;
    }


    DPRINTLN("parsed: " << sensor << ", " << beacon);

    coverage.insert(MinMax{ sensor.x - excessCoverage, sensor.x + excessCoverage });

}

struct Sensor
{
    Sensor(Coord::type x, Coord::type y, Coord::type _radius) : location{ x, y }, radius{ _radius } {}
    Sensor(Coord _location, Coord::type _radius) : location{ _location }, radius{ _radius } {}

    // Construct straight from the input file line why not
    Sensor(std::string_view inputLine)
        : location{ std::atoi( inputLine.data() + inputLine.find('x') + 2 ),
                    std::atoi( inputLine.data() + inputLine.find('y') + 2 ) }
    {
        const Coord beacon{ std::atoi( inputLine.data() + inputLine.find_last_of('x') + 2 ),
                            std::atoi( inputLine.data() + inputLine.find_last_of('=') + 1 ) };
    
        radius = Coord::dist(location, beacon);

        DPRINTLN(location << ", radius: " << radius);
    }
    Coord location;
    Coord::type radius;

    bool overlaps(const Coord &coord)
    {
        return location.dist(coord) <= radius;
    }

};

class SensorList
{
    std::vector<Sensor> m_sensors;
    const Coord lowerBound{ 0, 0 };

#ifdef TESTINPUT
    const Coord upperBound{ 20, 20 };
#else
    const Coord upperBound{ 4000000, 4000000 };
#endif

public:
    void insert(const Sensor& sensor)
    {
        m_sensors.push_back(sensor);
    }

    Coord checkPerimeters()
    {
        // Identity x{ 1, 1 } & y{ 1, -1 } ?
        const std::array<Coord, 4> directions{{
            { 1, 1 },   // right-up
            { 1, -1 },  // right-down
            { -1, -1 }, // left-down
            { -1, 1 }   // left-up
        }};

        for (auto iter1{ m_sensors.begin() }; iter1 != m_sensors.end(); ++iter1)
        {
            auto checkingSensor{ *iter1 };

            DPRINTLN("SENSOR checking: " << checkingSensor.location << " rad: " << checkingSensor.radius);

            const Coord left{ checkingSensor.location.x - checkingSensor.radius - 1, checkingSensor.location.y };
            const Coord top{ checkingSensor.location.x, checkingSensor.location.y + checkingSensor.radius + 1 };
            const Coord right{ checkingSensor.location.x + checkingSensor.radius + 1, checkingSensor.location.y };
            const Coord bottom{ checkingSensor.location.x, checkingSensor.location.y - checkingSensor.radius - 1 };

            // I've made a mess of this
            // But we walk through each edge of the sensor's perimeter
            // By iterating from left->top, top->right, rigt->bottom, bottom->left
            // Then for each location loop through the other sensors to see if they overlap the location
            // And that's basically it despite all the lines

            // These max checks are to bring the start of the edge check within bounds
            auto check{ left + 
                        directions[0] *
                        std::max(
                            std::max((lowerBound.x - left.x), Coord::type{ 0 }),
                            std::max((lowerBound.y - left.y), Coord::type{ 0 }))};

            // Iterate from the left of the sensor's radius to the top
            while (check.y < top.y && check.y <= upperBound.y && check.x <= upperBound.x)
            {
                bool bOverlapped{ true };

                // Is this location covered by any sensors?
                for (auto iter2{ m_sensors.begin() }; iter2 != m_sensors.end(); ++iter2)
                {
                    if (iter1 == iter2) continue;

                    auto sensorComp{ *iter2 };

                    // I thought we could skip ahead upon collisions (and still think it!)
                    // But this doesn't work for the non-test input so there must be edge cases or something idk!
                    if (auto sensorDist{ sensorComp.location.dist(check) }; sensorDist <= sensorComp.radius)
                    {
                        // DPRINTLN("Collision with " << sensorComp.location << ", r: " << sensorComp.radius << " at " << check);
                        
                        // This line is the skip that breaks it (but not for the test input):
                        // check += directions[0] * sensorDist;

                        // DPRINTLN("Skipping to " << check);
                        bOverlapped = true;
                        break;
                    }

                    bOverlapped = false;
                }

                // If we get here without an overlap, we've found the beacon
                if (!bOverlapped)
                {
                    return check;
                }

                DPRINTLN("left-top: " << check);
                check += directions[0];

            }

            // And the rest of this is the same but with different starts, ends & directions
            // 4 almost identical chunks of code but I'm leaving it for now
            check = top + 
                        directions[1] *
                        std::max(
                            std::max((lowerBound.x - top.x), Coord::type{ 0 }),
                            std::max((top.y - upperBound.y), Coord::type{ 0 }));
            
            while (check.x < right.x && check.y >= lowerBound.y && check.x <= upperBound.x)
            {
                bool bOverlapped{ true };

                for (auto iter2{ m_sensors.begin() }; iter2 != m_sensors.end(); ++iter2)
                {
                    if (iter1 == iter2) continue;

                    auto sensorComp{ *iter2 };

                    if (auto sensorDist{ sensorComp.location.dist(check) }; sensorDist <= sensorComp.radius)
                    {
                        // DPRINTLN("Collision with " << sensorComp.location << ", r: " << sensorComp.radius << " at " << check);
                        // check += directions[1] * sensorDist;
                        // DPRINTLN("Skipping to " << check);
                        bOverlapped = true;
                        break;
                    }

                    bOverlapped = false;

                }

                if (!bOverlapped)
                {
                    return check;
                }

                DPRINTLN("top-right: " << check);
                check += directions[1];
            }

            check = right + 
                        directions[2] *
                        std::max(
                            std::max((right.x - upperBound.x), Coord::type{ 0 }),
                            std::max((right.y - upperBound.y), Coord::type{ 0 }));

            while (check.y > bottom.y && check.y >= lowerBound.y && check.x >= lowerBound.x)
            {
                bool bOverlapped{ true };

                for (auto iter2{ m_sensors.begin() }; iter2 != m_sensors.end(); ++iter2)
                {
                    if (iter1 == iter2) continue;

                    auto sensorComp{ *iter2 };

                    if (auto sensorDist{ sensorComp.location.dist(check) }; sensorDist <= sensorComp.radius)
                    {
                        // DPRINTLN("Collision with " << sensorComp.location << ", r: " << sensorComp.radius << " at " << check);
                        // check += directions[2] * sensorDist;
                        // DPRINTLN("Skipping to " << check);
                        bOverlapped = true;
                        break;
                    }
                    bOverlapped = false;
                }

                if (!bOverlapped)
                {
                    return check;
                }

                DPRINTLN("right-bottom: " << check);
                check += directions[2];
            }
                
            check = bottom + 
                        directions[3] *
                        std::max(
                            std::max((bottom.x - upperBound.x), Coord::type{ 0 }),
                            std::max((lowerBound.y - bottom.y), Coord::type{ 0 }));

            while (check.x > left.x && check.y <= upperBound.y && check.x >= lowerBound.x)
            {
                bool bOverlapped{ true };

                for (auto iter2{ m_sensors.begin() }; iter2 != m_sensors.end(); ++iter2)
                {
                    if (iter1 == iter2) continue;

                    auto sensorComp{ *iter2 };
                    if (auto sensorDist{ sensorComp.location.dist(check) }; sensorDist <= sensorComp.radius)
                    {
                        // DPRINTLN("Collision with " << sensorComp.location << ", r: " << sensorComp.radius << " at " << check);
                        // check += directions[3] * sensorDist;
                        // DPRINTLN("Skipping to " << check);
                        bOverlapped = true;
                        break;
                    }

                    bOverlapped = false;
                }

                if (!bOverlapped)
                {
                    return check;
                }

                DPRINTLN("bottom-left: " << check); 
                check += directions[3];
            }
        }

        return { 0, 0 }; // Means it didn't work
    }
};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

        // If distance to y check 
        // is within distance to beacon
        // calculate how many x coords we cover

        MinMaxRanges coverageMap;

        while (inf)
        {
            std::string line;
            std::getline(inf, line);
            parseInput(line, coverageMap);
        }

        coverageMap.printAll();

        #ifdef TESTINPUT
        utils::printAnswer(coverageMap.calculateCoverage(), 26, "There are ", " positions which cannot be beacons on line 10");
        #else
        utils::printAnswer(coverageMap.calculateCoverage(), Coord::type{ 5100463 }, "There are ", " positions which cannot be beacons on line 200000");
        #endif

	}
};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}
        
        // Add each sensor and its radius to a list
        SensorList sensors;

        while (inf)
        {
            std::string line;
            std::getline(inf, line);
            if (line.length())
            {
                sensors.insert(Sensor{ line });
            }
        }

        // Iterate locations around each sensors' perimeter
        // Which is the extent of its radius + 1
        // and if we find a location that's not covered by any other sensor's
        // radius, we have found the distress beacon
        auto sensor{ sensors.checkPerimeters() };
        std::cout << "Distress beacon at: " << sensor << "\n";
        auto frequency{ sensor.x * 4000000 + sensor.y };
        
        #ifdef TESTINPUT
        utils::printAnswer( frequency, 56000011 );
        #else
        utils::printAnswer( frequency, 11557863040754, "Distress beacon tuning frequency: " ); // Don't know if it's right because no internet
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

