// Copy and paste for each day for a quick start

#include <array>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "utils.h"

struct Coord
{
	Coord() = default;
	Coord(int _x, int _y) : x{ _x }, y{ _y } {}

	int x{ 0 };
	int y{ 0 };

	// Overloading specific operators that should help with the tail function
	friend Coord operator+(const Coord& a, const Coord& b) { return { a.x + b.x, a.y + b.y }; }
	friend Coord operator-(const Coord& a, const Coord& b) { return { a.x - b.x, a.y - b.y }; }
	friend bool operator==(const Coord& a, const Coord& b) { return a.x == b.x && a.y == b.y; }
	friend bool operator<(const Coord&a, const Coord& b) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); }

	friend Coord operator+(const Coord& a, const int& b) { return { a.x + b, a.y + b }; }
	// Int comparisons not necessarily intuitive but useful for the checks we use
	friend bool operator==(const Coord& a, const int& b) { return a.x == b && a.y == b; }
	friend bool operator!=(const Coord& a, const int& b) { return !(a.x == b && a.y == b); }
	friend bool operator>(const Coord& a, const int& b) { return a.x > b || a.y > b; }

	explicit operator bool () const {return !(x == 0 && y == 0); }

	Coord& operator+=(const Coord& b)
	{
		this->x += b.x;
		this->y += b.y;
		return *this;
	}
	Coord& operator+=(const int& b)
	{
		this->x += b;
		this->y += b;
		return *this;
	}
	
	friend Coord operator/(const Coord& a, const Coord& b) { return { a.x / b.x, a.y / b.y }; }

	Coord abs() { return { std::abs(x), std::abs(y) }; }
	
};

std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    os << "{ " << coord.x << ", " << coord.y << " }";
    return os;
}

std::pair<Coord, int> parseLine(const std::string &line)
{
	int dist{ std::atoi(line.data() + 2) };

	switch (line[0])
	{
	case 'U' : return { Coord(0, 1), dist };
	case 'D' : return { Coord(0, -1), dist };
	case 'L' : return { Coord(-1, 0), dist };
	case 'R' : return { Coord(1, 0), dist };
	default  : return { Coord(0, 0), dist };
	}
}

Coord moveTail(const Coord &head, const Coord & tail)
{
	auto dist{ head - tail };
	auto distMag{ dist.abs() };

	// Keep direction, reduce any x or y movement to at most 1
	if (distMag > 1)
	{	
		return dist / distMag;
	}

	return { 0, 0 };
}

// Update knot locations until rope.end() is reached, return true if tail moves
bool propagateRopeMove(Coord* knotParent, const Coord* end)
{
	// If we make it all the way to rope.end(), tail must have moved
	if (knotParent + 1 == end)
	{
		return true;
	}

	auto dist{ *knotParent - *(knotParent + 1) };

	// Stop propogation, return false to signify no need to update list of visited coords
	if (!dist)
	{
		return false;
	}

	auto distMag{ dist.abs() };

	// Keep direction, reduce any x or y movement to at most 1
	if (distMag > 1)
	{	
		*(knotParent + 1) += dist / distMag;
	}

	return propagateRopeMove(++knotParent, end);
}

// Returns true if the tail moves
bool moveRope(Coord* head, const Coord *end, const Coord &move)
{
	*head += move;
	return propagateRopeMove(head, end);
}

void drawRope(Coord* head, Coord* end)
{
	Coord min { *head };
	Coord max { *head };

	std::for_each(head + 1, end, 
		[&min, &max](const Coord &knot)
		{
			if (knot.x > max.x)
			{
				max.x = knot.x;
			}
			else if (knot.x < min.x)
			{
				min.x = knot.x;
			}

			if (knot.y > max.y)
			{
				max.y = knot.y;
			}
			else if (knot.y < min.y)
			{
				min.y = knot.y;
			}
		});

	Coord span{ (max - min) + 1 };

	std::string blank( static_cast<size_t>(span.x), ' ' );
	std::vector<std::string> grid{ static_cast<size_t>(span.y), blank };

	for (int i{ 0 }; &(head[i]) != end; ++i)
	{
		char ch;
		if (i == 0)
		{
			ch = 'H';
		}
		else if (&(head[i]) == end - 1)
		{
			ch = 'T';
		}
		else
		{
			ch = std::to_string(i + 1)[0];
		}
		// Why haven't I struggled with this before
		auto normalised{ head[i] - min };
		auto &ingrid{ grid[static_cast<size_t>(normalised.y)][static_cast<size_t>(normalised.x)] };
		ingrid = ingrid == ' ' ? ch : ingrid;
	}

	for (auto &line : grid)
	{
		std::cout << line << '\n';
	}

	std::cout << '\n';

}

struct BmpHeader {
    char bitmapSignatureBytes[2] = {'B', 'M'};
    uint32_t sizeOfBitmapFile = 54 + 786432;
    uint32_t reservedBytes = 0;
    uint32_t pixelDataOffset = 54;
}; 

struct BmpInfoHeader {
    uint32_t sizeOfThisHeader = 40;
    int32_t width = 512; // in pixels
    int32_t height = 512; // in pixels
    uint16_t numberOfColorPlanes = 1; // must be 1
    uint16_t colorDepth = 24;
    uint32_t compressionMethod = 0;
    uint32_t rawBitmapDataSize = 0; // generally ignored
    int32_t horizontalResolution = 3780; // in pixel per meter
    int32_t verticalResolution = 3780; // in pixel per meter
    uint32_t colorTableEntries = 0;
    uint32_t importantColors = 0;
};

struct Pixel {
    uint8_t blue = 255;
    uint8_t green = 0;
    uint8_t red = 0;
};

void createBitMap()
{
	BmpHeader bmpHeader;
	BmpInfoHeader bmpInfoHeader;
	Pixel pixel;
    std::ofstream fout("output.bmp", std::ios::binary);

    fout.write((char *) &bmpHeader, 14);
    fout.write((char *) &bmpInfoHeader, 40);

    // writing pixel data
    size_t numberOfPixels = static_cast<size_t>(bmpInfoHeader.width) * static_cast<size_t>(bmpInfoHeader.height);
    for (size_t i = 0; i < numberOfPixels; i++) {
        fout.write((char *) &pixel, 3);
    }
    fout.close();
}

void drawRope(const Coord &head, const Coord &tail)
{
	int spacesCount { std::abs(head.x - tail.x) };
	int newLinesCount { std::abs(head.y - tail.y) };
	std::string spaces{};
	std::string newlines{};
	for (int i{ 0 }; i < spacesCount - 1; ++i)
	{
		spaces += ' ';
	}
	for (int i{ 0 }; i < newLinesCount - 1; ++i)
	{
		spaces += '\n';
	}

	if (tail.y > head.y)
	{
		if (tail.x < head.x)
		{
			std::cout << 'T' << newlines << spaces << 'H';
		}
		else
		{
			std::cout << spaces << 'T' << newlines << 'H';
		}
	}
	else
	{
		if (tail.x < head.x)
		{
			std::cout<< spaces << 'H' << newlines << 'T';
		}
		else
		{
			std::cout << 'H' << newlines << spaces << 'T';
		}
	}

	std::cout << '\n';

}

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}


		Coord tail{ 0, 0 };
		Coord head{ 0, 0 };
		std::set<Coord> visited{ tail };

		while(inf)
		{
			std::string instr;
			std::getline(inf, instr);
			auto result{ parseLine(instr) };

			for (int i{ 0 }; i < result.second; ++i)
			{
				head += result.first;

				auto tailmove{ moveTail(head, tail) };

				if (tailmove != 0)
				{
					tail += tailmove;
					visited.insert(tail);
				}

			}
		}

		std::cout << "Unique spaces visited by tail of 2 Planck length rope: " << visited.size() << '\n';


	}
};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::array<Coord, 10> rope;
		std::set<Coord> visited{ rope[0] };

		while(inf)
		{
			std::string instr;
			std::getline(inf, instr);
			auto moves{ parseLine(instr) };

			while (moves.second)
			{
				if (moveRope(rope.begin(), rope.end(), moves.first))
				{
					visited.insert(*rope.rbegin());
				}

				--moves.second;

				// This is drawing far too many lines...
				drawRope(rope.begin(), rope.end());
			}
		}

		// auto minx{ *(std::min_element(rope.begin(), rope.end())) };
		// auto maxx{ *(std::max_element(rope.begin(), rope.end())) };

		// auto miny{ *(std::min_element(rope.begin(), rope.end(),
		// 	[](const Coord &a, const Coord &b)
		// 	{
		// 		return a.y > b.y;
		// 	}))  };
		// auto maxy{ *(std::min_element(rope.begin(), rope.end(),
		// 	[](const Coord &a, const Coord &b)
		// 	{
		// 		return a.y < b.y;
		// 	}))  };

		// std::cout << "min: " << minx << " max: " << maxx << '\n';
		// std::cout << "miny: " << miny << "maxy: " << maxy << '\n';

		// Coord span;

		// span.x = maxx.x - minx.x;
		// span.y = maxy.y - miny.y;

		// std::cout << "Span: " << span << '\n';

		// std::cout << "Unique spaces visited by tail of 10 Planck length rope: " << visited.size() << '\n';
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	// Puzzle1::solve(input); 
	// Puzzle2::solve(input);
	
	createBitMap();

	return 0;
}