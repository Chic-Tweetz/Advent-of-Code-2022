// --- Day 18: Boiling Boulders ---

#include <array>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "utils.h"
#include "vector3d.h"

struct LavaMap
{
	using cont_t = std::vector<std::vector<std::vector<int>>>;
	LavaMap(int sizeX, int sizeY, int sizeZ) :
		map{ ST(sizeX), std::vector<std::vector<int>>{ ST(sizeY), std::vector<int>(ST(sizeZ), 0) } }
	{
	}
	LavaMap(const Vector3d &v3) : LavaMap(v3.x, v3.y, v3.z) {}

	int& operator[](const Vector3d &v3)
	{
		return map[ST(v3.x)][ST(v3.y)][ST(v3.z)];
	}

	std::vector<std::vector<int>>& operator[](int i)
	{
		return map[ST(i)];
	}

	bool inBounds(const Vector3d &co)
	{
		return co.x >= 0 && co.x < TOI(map.size()) &&
			   co.y >= 0 && co.y < TOI(map[0].size()) &&
			   co.z >= 0 && co.z < TOI(map[0][0].size());
	}

	// A for each to keep these nested fors out of my sight
	// int arg == value of coord in map, Vector3d& arg is the coord itself
	void forEach(std::function<void(int, Vector3d&)> fnc, const Vector3d &startOffset = 0, const Vector3d &endOffset = 0)
	{
		for (size_t x{ ST(startOffset.x) }; x < map.size() - ST(endOffset.x); ++x)
		{
			for (size_t y{ ST(startOffset.y) }; y < map[0].size() - ST(endOffset.y); ++ y)
			{
				for (size_t z{ ST(startOffset.z) }; z < map[0][0].size() - ST(endOffset.z); ++z)
				{
					Vector3d coord{ x, y, z };
					fnc((*this)[coord], coord);	
				}
			}
		}
	}

	// For each with predicate (I used this one for part 1)
	// fnc is passed the coordinate of truthy predicates as a Vector3d
	void forEachThat(std::function<bool(int)> pred, std::function<void(Vector3d&)> fnc, const Vector3d &startOffset = 0, const Vector3d &endOffset = 0)
	{
		for (size_t x{ ST(startOffset.x) }; x < map.size() - ST(endOffset.x); ++x)
		{
			for (size_t y{ ST(startOffset.y) }; y < map[0].size() - ST(endOffset.y); ++ y)
			{
				for (size_t z{ ST(startOffset.z) }; z < map[0][0].size() - ST(endOffset.z); ++z)
				{
					Vector3d coord{ x, y, z };
					if (pred((*this)[coord]))
					{
						fnc(coord);	
					}
				}
			}
		}
	}

	cont_t map;

};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::string input{ utils::bufferInput(infile) };

		// Map out the entirety of the bounding cuboid around the lava
		// It's quick this way!

		Vector3d largest{ 0 };
		// First find the largest value in each dimension to initialise the cube list in one go
		utils::doOnSplit(input, "\n", [&largest](std::string_view line)
		{ 
			Vector3d coord{ utils::splitInts(line, ",") };
			largest.xyzMax(coord);
		});

		// Vector of vector of vectors of int for [x][y][z] random access
		LavaMap map3d{ largest + 1 };

		// Set each lava coordinate to 1 in our map3d list
		utils::doOnSplit(input, "\n", [&map3d](std::string_view line)
		{ 
			Vector3d coord{ utils::splitInts(line, ",") };
			map3d[coord] = 1;
		});

		int surfaceArea{ 0 };

		auto isLava{ [](int val){ return val == 1; } };

		auto addSurfaceArea { [&surfaceArea, &map3d](Vector3d &coord){
			int sides{ 6 };
			const std::array<Vector3d, 6> adjacent{ { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } } };
			for (auto &&dir : adjacent)
			{
				auto neighbour{ coord + dir };
				if (map3d.inBounds(neighbour) && map3d[coord + dir] == 1)
				{
					--sides;
				}
			}
			surfaceArea += sides;
		} };
		
		// Mmm that's nice syntax, that's the good stuff
		map3d.forEachThat(isLava, addSurfaceArea);

		utils::printAnswer("surface area of lava droplet: ", surfaceArea);

	}

};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		// Map a cuboid, enveloping the extents of the input coordinates such that the lava is entirely contained 
		// (Size is 24 x 23 x 24 for input data)
		// Initialise all values to 0 (0 can signify empty space initially)
		
		// Using input coordinates, make all lava-occupied spaces 1 (0 == nothing, 1 == lava)
		// Starting from the outside, give every point which is not occupied by lava some other value (2),
		// queueing neighbours with values of 0, changing those to 2 (2 == exterior space)

		// The exterior surface area will then be incremented for every neighbour of a 2 space which 
		// has a value of 1 - ie everywhere exterior coords are adjacent to lava coords

		// This full map (vector of vectors of vectors) is a lot quicker than what I did for part 1 btw
		// Do it there too?

		std::string input{ utils::bufferInput(infile) };

		Vector3d largest{ 0 };
		// First find the largest value in each dimension to initialise the cube list in one go
		utils::doOnSplit(input, "\n", [&largest](std::string_view line)
		{ 
			Vector3d coord{ utils::splitInts(line, ",") };
			largest.xyzMax(coord);
		});

		// Vector of vector of vectors of int for [x][y][z] random access
		LavaMap map3d{ largest + 3 };

		// Set each lava coordinate to 1 in our map3d list
		utils::doOnSplit(input, "\n", [&map3d](std::string_view line)
		{ 
			Vector3d coord{ utils::splitInts(line, ",") };
			// We add 1 to input coords x, y & z because we reserve [0] for exterior spaces
			map3d[coord + 1] = 1;
		});

		// Starting from origin (which we have ensured is an exterior (non-lava) coordinate)
		// Breadth-first search neighbouring spaces with value 0,
		// set them to 2 to signify exterior coordinates (& avoid re-adding them to the queue)
		std::deque<Vector3d> queue{ { 0, 0, 0 } };
		map3d[Vector3d{0, 0, 0}] = 2;

		int outerSurface{ 0 };

		while (!queue.empty())
		{
			auto cur{ queue.front() };
			queue.pop_front();

			const std::vector<Vector3d> adjacent{ { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } } };
			
			for (auto &&dir : adjacent)
			{
				auto neighbour{ cur + dir };

				if (map3d.inBounds(neighbour))
				{
					// Queue neighbouring exterior spaces
					if (!map3d[neighbour])
					{
						map3d[neighbour] = 2;
						queue.push_back(neighbour);
					}
					else if (map3d[neighbour] == 1)
					{
						// Exterior coordinates adjacent to lava increment surface area
						++outerSurface;
					}
				}
			}
		}

		utils::printAnswer("exterior surface area of lava droplet: ", outerSurface);

	}
};

int main(int argc, char* argv[])
{
	flags::set(argc, argv);

	const std::string input{ utils::inputFile(__FILE__) };

	try
	{
		if (utils::doP1()) Puzzle1::solve(input); 
		if (utils::doP2()) Puzzle2::solve(input);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;

}
