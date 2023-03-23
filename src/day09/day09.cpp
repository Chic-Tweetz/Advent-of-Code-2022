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

	friend Coord operator+(const Coord& a, const Coord& b) { return { a.x + b.x, a.y + b.y }; }
	friend Coord operator-(const Coord& a, const Coord& b) { return { a.x - b.x, a.y - b.y }; }
	friend bool operator==(const Coord& a, const Coord& b) { return a.x == b.x && a.y == b.y; }

	friend bool operator!=(const Coord& a, const int& b) { return !(a.x == b && a.y == b); }
	// friend bool operator<(const Coord&a, const Coord& b) { return a.y < b.y || ( a.y == b.y && a.x < b.x ); }
	friend bool operator<(const Coord&a, const Coord& b) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); }

	Coord& operator+=(const Coord& b)
	{
		this->x += b.x;
		this->y += b.y;
		return *this;
	}
	
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

// Long ropes can create diagonal gaps of 2, 2 so you need to make sure 
// to limit diagonal moves to max of 1, 1
Coord tailMove(const Coord &head, const Coord & tail)
{
	Coord dist{ head - tail };
	Coord move{ tail };
	
	if (dist.x > 1)
	{
		move.x = head.x - 1;
		move.y = head.y;
	}
	else if (dist.x < -1)
	{
		move.x = head.x + 1;
		move.y = head.y;
	}
	else if (dist.y > 1)
	{
		move.y = head.y - 1;
		move.x = head.x;
	}
	else if (dist.y < -1)
	{
		move.y = head.y + 1;
		move.x = head.x;
	}

	return move;
	
}

// Dirty if else chains but this finally got the right answer
Coord tailMove2(const Coord &head, const Coord & tail)
{
	// Move up to 1 in one or two directions

	auto dist{ head - tail };
	Coord move{ 0, 0 };
	if (dist.x > 1)
	{
		move.x = 1;

		if (dist.y > 0)
		{
			move.y = 1;
		}
		else if (dist.y < 0)
		{
			move.y = -1;
		}
	}
	else if (dist.x < -1)
	{
		move.x = -1;

		if (dist.y > 0)
		{
			move.y = 1;
		}
		else if (dist.y < 0)
		{
			move.y = -1;
		}
	}

	else if (dist.y > 1)
	{
		move.y = 1;

		if (dist.x > 0)
		{
			move.x = 1;
		}
		else if (dist.x < 0)
		{
			move.x = -1;
		}
	}
	else if (dist.y < -1)
	{
		move.y = -1;

		if (dist.x > 0)
		{
			move.x = 1;
		}
		else if (dist.x < 0)
		{
			move.x = -1;
		}
	}

	return move;
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

		std::cout << "H: " << head << " | T: " << tail << '\n';
		//int limit {10};
		// while(inf && limit-- > 0)
		while(inf)
		{
			std::string instr;
			std::getline(inf, instr);
			auto result{ parseLine(instr) };
			std::cout << "== " << instr << " : " << result.first << " * " << result.second << " ==" << "\n\n";

			for (int i{ 0 }; i < result.second; ++i)
			{
				head += result.first;

				auto newtail{ tailMove(head, tail) };

				if (newtail != tail)
				{
					tail = tailMove(head, tail);
					visited.insert(tail);
				}

				std::cout << "H: " << head << " | T: " << tail << "\n";

				// drawRope(head, tail);
			}
			std::cout << '\n';
		}

		for(auto t : visited)
		{
			std::cout << "visited: " << t << '\n';
		}

		std::cout << "unique spaces visited: " << visited.size() << '\n';
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
		
		// int limit { 25 };
		// while (inf && limit-- > 0)
		while(inf)
		{
			std::string instr;
			std::getline(inf, instr);
			auto result{ parseLine(instr) };
			// std::cout << "== " << instr << " : " << result.first << " * " << result.second << " ==" << "\n\n";

			for (int i{ 0 }; i < result.second; ++i)
			{
				rope[0] += result.first;

				for (size_t knot{ 1 }; knot < rope.size() - 1; ++knot)
				{
					rope[knot] += tailMove2(rope[knot - 1], rope[knot]);
				}

				auto tailmove{ tailMove2(rope[rope.size() - 2], rope[rope.size() - 1]) };

				if (tailmove != 0)
				{
					rope[rope.size() - 1] += tailmove;
					visited.insert(rope[rope.size() - 1]);
				}
			
				// for (auto &k : rope)
				// {
				// 	std::cout << k << '\n';
				// }
				// std::cout << '\n';
				// drawRope(head, tail);
			}
		}

		// Still wrong. Wronger even than before!
		std::cout << "unique spaces visited: " << visited.size() << '\n';
	}

};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	

	return 0;
}