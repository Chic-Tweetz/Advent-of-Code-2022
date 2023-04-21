// --- Day 12: Hill Climbing Algorithm ---

#include <array>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

struct Coord
{
	Coord(int _x = 0, int _y = 0) : x{ _x }, y{ _y } {}

	int x;
	int y;

	friend Coord operator+(const Coord &a, const Coord &b) { return { a.x + b.x, a.y + b.y }; }
	friend Coord operator-(const Coord &a, const Coord &b) { return { a.x - b.x, a.y - b.y }; }
};

std::ostream &operator<<(std::ostream& os, const Coord &coord)
{
	return os << "{ " << coord.x << ", " << coord.y << " }";
}

class TerrainMap
{
public:
	using container_type = std::vector<std::string>;

	TerrainMap(container_type map) : m_map{ map }
	{
		m_scores.reserve(m_map.size());
		for (size_t i{ 0 }; i < m_map.size(); ++i)
		{
			m_scores.push_back(std::vector<int>(m_map[0].length(), -1));
		}
		m_scores.shrink_to_fit();
		
		findStartAndEnd();
	}

	int startToEnd()
	{
		return (findShortestPath(start, goal));
	}

	int shortestPathFromGoalToLow()
	{
		std::deque<Coord> pending{ goal };
		
		setScore(goal, 0);

		while (pending.size() && getHeight(pending.front()) != 'a' )
		{
			addNeighboursIf(pending, [](char ch1, char ch2)
				{
					return ch1 <= ( ch2 + 1 );
				});
			pending.pop_front();
		}
		return getScore(pending.front());
	}

	int findShortestPath(const Coord &from, const Coord &to)
	{
		// Add adjacent coords to the back of a deque if they are traversable
		// give each coord a score: the distance (number of orthogonal steps) from start
		// repeat for every coord added to the deque, starting from the front
		// this way, the shortest distance to every coord will be the first one checked
		// The scores vector is initialised with sentinel values (-1) so we can check
		// whether coords have been checked yet
		std::deque<Coord> pending{ from };
		
		setScore(start, 0);

		while (pending.size() && getScore(to) == -1)
		{
			addNeighboursIf(pending, [](char ch1, char ch2)
				{
					return ch2 <= ( ch1 + 1 );
				});
			pending.pop_front();
		}
		return getScore(to);
	}

private:

	container_type m_map;

	Coord start;
	Coord goal;

	std::vector<std::vector<int>> m_scores;

	bool findStartAndEnd()
	{
		int found{ 0 };
		for (size_t y{ 0 }; y < m_map.size(); ++y)
		{
			for (size_t x{ 0 }; x < m_map[0].length(); ++x)
			{
				if (m_map[y][x] == 'S')
				{
					m_map[y][x] = 'a';
					start = { static_cast<int>(x), static_cast<int>(y) };
					if (++found == 2)
					{
						return true;
					}
				}
				else if (m_map[y][x] == 'E')
				{
					m_map[y][x] = 'z';
					goal = { static_cast<int>(x), static_cast<int>(y) };
					if (++found == 2)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	void addNeighboursIf(std::deque<Coord> &pending, bool (*condition)(char, char))
	{
		auto curr{ pending.front() };
		const std::array<Coord, 4> neighbours {{
			curr + Coord{ 0, -1 },
			curr + Coord{ 0, 1 }, 
			curr + Coord{ -1, 0 }, 
			curr + Coord{ 1, 0 } 
		}};

		auto checks {
			[this, condition](const Coord &a, const Coord &b)
			{ 
				return this->inBounds(b) &&		// Valid indices
					this->getScore(b) == -1 &&	// Only look at unscored coords
					condition(this->getHeight(a), this->getHeight(b));	// Compare current height against neighbour's
			} };

		for (auto &neighbour : neighbours)
		{
			if (checks(curr, neighbour))
			{
				setScore(neighbour, getScore(curr) + 1);
				pending.push_back(neighbour);
			}
		}
	}

	char getHeight(const Coord &coord) const
	{
		return m_map[static_cast<size_t>(coord.y)][static_cast<size_t>(coord.x)];
	}

	int getScore(const Coord &coord) const
	{
		return m_scores[static_cast<size_t>(coord.y)][static_cast<size_t>(coord.x)];
	}

	void setScore(const Coord &coord, int score)
	{
		m_scores[static_cast<size_t>(coord.y)][static_cast<size_t>(coord.x)] = score;
	}

	bool inBounds(const Coord &coord)
	{
		const size_t x{ static_cast<size_t>(coord.x) };
		const size_t y{ static_cast<size_t>(coord.y) };
		return
			y >= 0 && y < m_map.size() &&
			x >= 0 && x < m_map[y].length();
	}
};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		TerrainMap grid{ utils::bufferLines(infile) };

		auto solution{ grid.startToEnd() };

		utils::printAnswer("shortest path from start to goal: ", solution);

	}
};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		TerrainMap grid{ utils::bufferLines(infile) };

		auto solution{ grid.shortestPathFromGoalToLow() };

		utils::printAnswer("shortest path from goal to low point: ", solution);

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
