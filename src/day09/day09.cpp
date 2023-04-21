// --- Day 9: Rope Bridge ---

#include <algorithm>
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

#include "image.h"
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
	friend bool operator>(const Coord&a, const Coord& b) { return a.x > b.x || ( a.x == b.x && a.y > b.y ); }

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
	
	friend Coord operator/(const Coord& a, const Coord& b)
	{
		int divx{ b.x == 0 ? 1 : b.x };
		int divy{ b.y == 0 ? 1 : b.y };
		return { a.x / divx, a.y / divy };
	}

	Coord abs() { return { std::abs(x), std::abs(y) }; }
	
};

using Rope = std::array<Coord, 20>;

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
	static int calls{ 0 };

	Coord min{ *head };
	Coord max{ *head };

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

	/*
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
		
		auto normalised{ head[i] - min };
		auto &ingrid{ grid[static_cast<size_t>(normalised.y)][static_cast<size_t>(normalised.x)] };
		ingrid = ingrid == ' ' ? ch : ingrid;
	}

	for (auto &line : grid)
	{
		std::cout << line << '\n';
	}

	std::cout << '\n';
	*/


	//// Well that was fun
	

	//int canvasWidth{ 640 };
	//int canvasHeight{ 480 };
	//Coord origin{ 320, 240 };

	//Image bmp{ canvasWidth, canvasHeight, Color(1.f, 1.f, 1.f)};

	//for (int i{ 0 }; &(head[i]) != end; ++i)
	//{
	//	auto pixel{ origin + head[i] };
	//	if (pixel.x < 0 || pixel.y < 0 || pixel.x >= canvasWidth || pixel.y >= canvasHeight)
	//	{
	//		continue;
	//	}
	//	bmp.setColor(Color{ 1.f, 0.f, 0.f }, pixel.x, pixel.y);
	//}


	//Image bmp{ static_cast<int>(grid[0].length()), static_cast<int> (grid.size()) };
	//for (size_t y{ 0 }; y < grid.size(); ++y)
	//{
	//	for (size_t x{ 0 }; x < grid[0].length(); ++x)
	//	{
	//		if (grid[y][x] != ' ')
	//		{
	//			bmp.setColor(Color(1.f, 0.f, 0.f), static_cast<int>(x), static_cast<int>(y));
	//		}
	//	}
	//}

	
	//min.x = min.x > 0 ? 0 : min.x;
	//min.y = min.y > 0 ? 0 : min.y;
	//max.x = max.x < 0 ? 0 : max.x;
	//max.y = max.y < 0 ? 0 : max.y;


	//min.x = min.x > 0 ? -1 * max.x : min.x;
	//min.y = min.y > 0 ? -1 * max.y : min.y;
	//max.x = max.x < 0 ? -1 * min.x : max.x;
	//max.y = max.y < 0 ? -1 * min.y : max.y;

	auto minabs = min.abs();
	auto maxabs = max.abs();

	max.x = std::max(minabs.x, maxabs.x);
	min.x = std::max(minabs.x, maxabs.x) * -1;
	max.y = std::max(minabs.y, maxabs.y);
	min.y = std::max(minabs.y, maxabs.y) * -1;

	
	//static Coord largestSpan{ 0, 0 };
	span = (max - min) + 1;

	//largestSpan.x = span.x > largestSpan.x ? span.x : largestSpan.x;
	//largestSpan.y = span.y > largestSpan.y ? span.y : largestSpan.y;

	// Want to keep the origin at the centre really
	span = { 301, 161 };
	min.x = -150;
	min.y = -80;
	max.x = 150;
	max.y = 80;

	if (++calls < 500)
	{
		std::string imagename{ "./images/rope/rope_" + std::to_string(calls) + ".bmp" };
		Image bmp{ span.x, span.y };

		std::cout << imagename << " = span: " << span << '\n';

		for (int y{ 0 }; y < span.y; ++y)
		{
			int originx{ 0 - min.x };
			bmp.setColor(Color{ 0.f, 0.f, 0.25f }, originx, y);
		}

		for (int x{ 0 }; x < span.x; ++x)
		{
			int originy{ 0 - min.y };
			bmp.setColor(Color{ 0.f, 0.f, 0.25f }, x, originy);
		}

		for (int i{ 0 }; &(head[i]) != end; ++i)
		{
			auto pixel{ head[i] - min };
			std::cout << pixel << ' ';
			if (pixel.x < 0 || pixel.y < 0 || pixel.x >= span.x || pixel.y >= span.y)
			{
				continue;
			}
			bmp.setColor(Color{ 1.f, 0.f, 0.f }, pixel.x, pixel.y);
		}
		std::cout << '\n';

		bmp.save(imagename.data());

	}

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

		utils::printAnswer("unique spaces visited by tail of 2 Planck length rope: ", visited.size());
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

		Rope rope;
		std::set<Coord> visited{ rope[0] };

		while(inf)
		{
			std::string instr;
			std::getline(inf, instr);
			auto moves{ parseLine(instr) };

			while (moves.second)
			{
				// absurd
				auto ptr{ rope.end() - 1 };
				auto &val{ *ptr };
				auto* ptr2{ &val };
				// refactor to use iterators instead
				if (moveRope(&(*rope.begin()), ++ptr2, moves.first))
				{
					visited.insert(*rope.rbegin());
				}

				--moves.second;

				// Careful uncommenting this, it creates a series of bitmaps!
				// drawRope(&(*rope.begin()), ptr2);
			}
		}

		utils::printAnswer("unique spaces visited by tail of 10 Planck length rope: ", visited.size());
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
