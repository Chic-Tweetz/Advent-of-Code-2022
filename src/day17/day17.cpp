// --- Day 17: Pyroclastic Flow ---

#include <algorithm>
#include <array>
#include <bitset>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "debug.h"
#include "log.h"
#include "utils.h"
#include "vector2d.h"

struct Rect
{
	Rect(int left, int lower, int right, int upper) : lowerLeft{ left, lower }, upperRight{ right, upper } {}
	Vector2d lowerLeft;
	Vector2d upperRight;

	int top() const
	{
		return upperRight.y;
	}
	int bottom() const
	{
		return lowerLeft.y;
	}
	int right() const
	{
		return upperRight.x;
	}
	int left() const
	{
		return lowerLeft.x;
	}

	bool overlap(const Rect &other)
	{
		return overlap(*this, other);
	}

	static bool overlap(const Rect &a, const Rect &b)
	{
		return overlap(a.lowerLeft.x, a.lowerLeft.y, a.upperRight.x, a.upperRight.y,
		               b.lowerLeft.x, b.lowerLeft.y, b.upperRight.x, b.upperRight.y);
	}

	bool overlap(int x, int y)
	{
		return ((y <= upperRight.y && y >= lowerLeft.y) &&
		        (x >= lowerLeft.x && x <= upperRight.x));
	}

	static bool overlap(int aLeft, int aLow, int aRight, int aTop, int bLeft, int bLow, int bRight, int bTop)
	{
		return !(( aTop   < bLow ) ||			
			     ( aRight < bLeft ) ||			
		         ( aLeft  > bRight ) || 
			     ( aLow   > bTop ));
	}

	Rect translate(int x, int y) const
	{
		return { lowerLeft.x + x, lowerLeft.y + y, upperRight.x + x, upperRight.y + y };
	}
};

class Rock
{
protected:
	// Origin = lower-left corner for ease of spawning in correct location
	Vector2d prevLocation;
	
	// Reference a list of primitives instead of copying for each
	std::vector<Rect> &primitives;

	// Width, Height
	Vector2d bounds;

public:	
	Vector2d location;
	int pointersTo{ 0 }; // For clean-up because I'm not using smart pointers atm

	// Width, Height
	Rock(int x, int y, std::vector<Rect> &prims) :
		primitives{ prims },
		bounds{ 0, 0 },
		location{ x, y }
	{
		// Origin is bottom left so primitives will always be at least 0
		for (auto &&prim : primitives)
		{
			if (bounds.x < prim.upperRight.x)
			{
				bounds.x = prim.upperRight.x;
			}
			if (bounds.y < prim.upperRight.y)
			{
				bounds.y = prim.upperRight.y;
			}
		}
	}

	Rect boundingRect() const
	{
		return Rect{ location.x, location.y, location.x + bounds.x, location.y + bounds.y };
	}

	int top() const
	{
		return location.y + bounds.y;
	}
 
	void translate(int x, int y)
	{
		prevLocation = location;
		location.x += x;
		location.y += y;
	}

	void translate(const Vector2d &vec)
	{
		prevLocation = location;
		location += vec;
	}

	void undoTranslate()
	{
		location = prevLocation;
	}

	void setLocation(int x, int y)
	{
		location.x = x;
		location.y = y;
	}

	void fall()
	{
		translate(0, -1);
	}

	bool collides(const Rect &other)
	{
		for (auto &&collider : primitives)
		{
			if (collider.translate(location.x, location.y).overlap(other))
			{
				return true;
			}
		}
		return false;
	};

	bool collides(int x, int y)
	{
		for (auto &&collider : primitives)
		{
			if (collider.translate(location.x, location.y).overlap(x, y))
			{
				return true;
			}
		}
		return false;
	}

	bool collides(const Rock &other)
	{
		for (auto &&collider : primitives)
		{
			for (auto &&otherCollider : other.primitives)
			{
				if (collider.translate(location.x, location.y).overlap(
					otherCollider.translate(other.location.x, other.location.y)))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool collidesWithWall()
	{
		return (location.x < 0 || location.x + bounds.x > 6);
	}

};

class CollisionCollection
{
public: 
	~CollisionCollection()
	{
		for (auto &&bucket : m_rocks)
		{
			for (auto *rock : bucket)
			{
				if (--(rock->pointersTo) == 0)
				{
					delete rock;
				}
			}
		}
	}

	using bucket_t = std::vector<Rock*>;
	using container_t = std::vector<bucket_t>;

	int highestPoint{ 0 };
	int lowestPoint{ 0 };
	long long cummulativeHeight{ 0 };

	bool collides(Rock *rock) const
	{
		auto lowerBucket{ rock->location.y };
		auto upperBucket{ rock->boundingRect().top() };

		for (auto i{ heightToIndex(lowerBucket) }; i <= heightToIndex(upperBucket); ++i)
		{
			for (auto *otherRock : m_rocks[i])
			{
				if (rock->collides(*otherRock))
				{
					return true;
				}
			}
		}
		return false;
	}

	Rock* collides(const Vector2d &point) const
	{
		for (auto *rock : m_rocks[heightToIndex(point.y)])
		{
			if (rock->collides(point.x, point.y))
			{
				return rock;
			}
		}
		return nullptr;
	}

	void purgeBelow(long height)
	{
		for (size_t i{ 0 }; i <= heightToIndex(height); ++i)
		{
			for (auto *rock : m_rocks[i])
			{
				if (--(rock->pointersTo) == 0)
				{
					delete rock;
				}
			}
		}
		m_rocks.erase(m_rocks.begin(), m_rocks.begin() + static_cast<long>(heightToIndex(height)));
	}

	void purgeNotInList(const std::set<Rock*> &list)
	{
		// std::vector<Rock*> deleteRocks;

		// for (auto &&bucket : m_rocks)
		// {
		// 	auto eraseFrom{ std::remove_if(bucket.begin(), bucket.end(),
		// 		[&list](Rock *rock)
		// 		{
		// 			if (!list.contains(rock))
		// 			{
		// 				if (--(rock->pointersTo) == 0)
		// 				{
		// 					delete rock;
		// 				}
		// 				return true;
		// 			}
		// 			return false;
		// 		})};
		// 	bucket.erase(eraseFrom, bucket.end());
		// }

		// Still gets the wrong answer when this method is called, so this is presumably equivalent
		for (auto &&bucket : m_rocks)
		{
			for (auto *rock : bucket)
			{
				if (!list.contains(rock) && --(rock->pointersTo) <= 0)
				{
					delete rock;
				}
			}

			bucket.clear();
		}

		for (auto *rock : list)
		{
			rock->pointersTo = 0;
			push(rock);
		}
	}

	void resetHeight()
	{
		// Have to move all rocks, then that means all their containers are probably wrong!

		// We only want to keep the rocks that can be collided with by new ones
		// so we might as well combine this method with purgeNotInList
		highestPoint -= lowestPoint;

		std::set<Rock*> allRocks;

		for (auto &&bucket : m_rocks)
		{
			for (auto *rock : bucket)
			{
				allRocks.insert(rock);
			}
		}

		for (auto &&bucket : m_rocks)
		{
			bucket.clear();
		}

		for (auto *rock : allRocks)
		{
			rock->translate(0, -lowestPoint);
			rock->pointersTo = 0;
			push(rock);
		}

		lowestPoint = 0;
	}

	void newWallSpan(const std::set<Rock*> &list)
	{
		for (auto &&bucket : m_rocks)
		{
			for (auto *rock : bucket)
			{
				if(!list.contains(rock) && --(rock->pointersTo) <= 0)
				{
					delete rock;
				}
			}

			bucket.clear();
		}

		highestPoint -= lowestPoint;
		
		for (auto *rock : list)
		{
			rock->pointersTo = 0;
			rock->translate(0, -lowestPoint);
			push(rock);
		}

		lowestPoint = 0;
	}

	void deleteRocksBelow(long lowest)
	{
		auto lowestSurvivors{ heightToIndex(lowest) };

		for (size_t i{ 0 }; i < lowestSurvivors; ++i)
		{
			for (auto *rock : m_rocks[i])
			{
				if (--(rock->pointersTo) == 0)
				{
					delete rock;
				}
			}
			m_rocks[i].clear();
		}
	}

	// Don't think I use this anywhere
	struct RockLinkedList
	{
		RockLinkedList(Rock* rockPtr, RockLinkedList* nextNode = nullptr) : rock{ rockPtr }, next{ nextNode } {}
		Rock* rock;
		RockLinkedList *next = nullptr;

		RockLinkedList* push(Rock *newRock)
		{
			if (!rock)
			{
				rock = newRock;
				return this;
			}
			else
			{
				return new RockLinkedList(newRock, this);
			}
		}
	};

	void push(Rock* newRock)
	{
		auto lowBucket{ heightToIndex(newRock->location.y) };
		auto highBucket{ heightToIndex(newRock->top()) };

		while(m_rocks.size() <= highBucket + 1)
		{
			m_rocks.push_back(std::vector<Rock*>{});
		}
		
		for (size_t i{ lowBucket }; i <= highBucket; ++i)
		{
			if (m_rocks[i].size() == 0)
			{
				m_rocks[i] = std::vector<Rock*>{ newRock };
			}
			else
			{
				m_rocks[i].push_back(newRock);

			}

			++(newRock->pointersTo);
		}
	}

private:
	container_t m_rocks;

	int m_ySeparation{ 100 };

	size_t heightToIndex(long height) const
	{
		return static_cast<size_t>(height / m_ySeparation);
	}

};

int jetDirection(const char lr)
{
	return lr == '<' ? -1 : 1 ; 
}

Vector2d jetDirection(std::string::iterator jet)
{
	return *jet == '<' ? Vector2d{ -1, 0 } : Vector2d{ 1, 0 };
}

// Print state of rocks in a range
void drawRange(Vector2d lowerLeft, Vector2d upperRight, const std::vector<Rock*> &rocks, Rock* faller)
{
	for (int y{ upperRight.y }; y > lowerLeft.y; --y)
	{
		for(int x{ lowerLeft.x }; x < upperRight.x; ++x)
		{
			bool bOccupied{ false };

			if (faller->collides(x, y))
			{
				bOccupied = true;
				DP('@' << style::reset);
			}
			else
			{
				for (auto &&rock : rocks)
				{
					if (rock->collides(x, y))
					{
						bOccupied = true;
						DP('#' << style::reset);
						break;
					}
				}
			}

			if (!bOccupied)
			{
				DP(' ' << style::reset);
			}

		}
		DP('\n' << style::reset);
	}
}

// Print out the state of the rocks in a range, this time using our CollisionCollection
void drawRange(Vector2d lowerLeft, Vector2d upperRight, const CollisionCollection &rocks, Rock* faller)
{
	for (int y{ upperRight.y }; y > lowerLeft.y; --y)
	{
		for(int x{ lowerLeft.x }; x < upperRight.x; ++x)
		{
			bool bOccupied{ false };

			if (x == -1 || x == 7)
			{
				bOccupied = true;
				DP('|');
			}
			else if (faller->collides(x, y))
			{
				bOccupied = true;
				DP('@' << style::reset);
			}
			else
			{
				if (rocks.collides( { x, y } ))
				{
					bOccupied = true;
					DP('#' << style::reset);
				}	
			}

			if (!bOccupied)
			{
				DP(' ' << style::reset);
			}

		}
		DP('\n' << style::reset);
	}
	DL("");
}

// State of floor and indices of cycling rocks & jets, + height
struct RocksState
{
	using floor_t = std::vector<std::bitset<8>>;

	RocksState(){}
	RocksState(size_t rockI, size_t jetI, floor_t floor, long long _highPoint, long long _count) : rockIndex{ rockI }, jetIndex{ jetI }, highFloor{ floor }, highPoint{ _highPoint }, count{ _count } {}

	size_t rockIndex;
	size_t jetIndex;
	floor_t highFloor;
	long long highPoint;
	long long count;

	friend bool operator==(const RocksState &lhs, const RocksState &rhs)
	{
		if (lhs.highFloor.size() == rhs.highFloor.size() && lhs.jetIndex == rhs.jetIndex && lhs.rockIndex == rhs.rockIndex)
		{
			for (size_t i{ 0 }; i < lhs.highFloor.size(); ++i)
			{
				if (lhs.highFloor[i] != rhs.highFloor[i])
				{
					break;
				}
			}
			return true;
		}
		return false;
	}

	// Less than required for storing in an ordered list
	friend bool operator<(const RocksState &lhs, const RocksState &rhs)
	{
		if ((lhs.jetIndex < rhs.jetIndex) ||
		   ((lhs.jetIndex == rhs.jetIndex) && (lhs.rockIndex < rhs.rockIndex)) ||
		   ((lhs.jetIndex == rhs.jetIndex) && (lhs.rockIndex == rhs.rockIndex) && (lhs.highFloor.size() < rhs.highFloor.size())))
		{
			return true;
		}

		if (lhs.jetIndex == rhs.jetIndex && lhs.rockIndex == rhs.rockIndex && lhs.highFloor.size() == rhs.highFloor.size())
		{
			for (size_t i{ 0 }; i < lhs.highFloor.size(); ++i)
			{
				if (lhs.highFloor[i] != rhs.highFloor[i])
				{
					return lhs.highFloor[i].to_ulong() < rhs.highFloor[i].to_ulong();
				}
			}
		}

		return false;

	}
};

// Now that you have a floor checker, you can use it to purge every rock not touching it
RocksState::floor_t wallToWall(Vector2d start, CollisionCollection &rocks)
{
	// Find first settled (non-wall) rock
	while (!(rocks.collides(start)))
	{
		start.y -= 1;
	}

	// From a given starting point, we can call 0 a clockwise turn or 1 a move forwards
	// Thus we can save some memory by storing bits rather than locations
	size_t moveCount{ 0 };

	// bitset doesn't actually use any less space and isn't fast either
	// but it's fun to use bits sometimes
	// probably better off using bitwise operators on unsigned longs
	std::vector<std::bitset<8>> moves{ 0b0000'0000 };

	int lowPoint{ start.y };

	// "Left hand" on empty space, facing right
	Vector2d curr{ start };
	Vector2d dir{ 1, 0 };
	Vector2d left{ dir.rotateAntiClockwise() };

	// Can use this to purge rocks which don't make up the floor
	// But I'm just going to remove rocks below the lowest height
	// std::set<Rock*> floorRocks{ rocks.collides(start) };

	while (curr.x < 6)
	{
		if (auto *leftCollider{ rocks.collides(curr + left) }; !leftCollider)
		{
			if (auto *frontCollider{ rocks.collides(curr + dir) }; frontCollider)
			{
				// floorRocks.insert(frontCollider);

				moves[moveCount / 8].set(moveCount % 8);

				if (++moveCount / 8 >= moves.size())
				{
					moves.push_back(0b0000'0000);
				}

				curr += dir;

				if (curr.y < lowPoint)
				{
					lowPoint = curr.y;
				}
			}
			else
			{
				// 1 anti-clockwise turn = 3 clockwise turns
				if ((moveCount += 3) / 8 >= moves.size())
				{
					moves.push_back(0b0000'0000);
				}

				left = dir;
				dir = dir.rotateClockwise();
			}
		}
		else
		{
			// floorRocks.insert(leftCollider);
			
			if (++moveCount / 8 >= moves.size())
			{
				moves.push_back(0b0000'0000);
			}

			moves[moveCount / 8].set(moveCount % 8);
			
			if (++moveCount / 8 >= moves.size())
			{
				moves.push_back(0b0000'0000);
			}
			
			dir = left;
			left = left.rotateAntiClockwise();
			curr += dir;

			if (curr.y < lowPoint)
			{
				lowPoint = curr.y;
			}
		}
	}

	if (lowPoint > rocks.lowestPoint)
	{
		// We can delete rocks that can now longer be collided with:
		rocks.deleteRocksBelow(lowPoint);

		// But trying to adjust the height of the rocks (such that the "low point" remains at 0)
		// So far has caused issues, so I might let it go for now!

		// Behold my failed methods!

		// rocks.shiftRocksDown(lowPoint - rocks.lowestPoint);
		// rocks.newWallSpan(floorRocks);
		// rocks.purgeBelow(lowPoint);
		// rocks.purgeNotInList(floorRocks);
		// rocks.resetHeight();
	}

	return moves;

}

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		Logger logger{ utils::getFilePath(__FILE__, "log-pt1"), false, false };
		
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string jets;
		std::getline(inf, jets);

		std::vector<Rect> Horizontal4{ { 0, 0, 3, 0 } }; // ####

		std::vector<Rect> Vertical4{ { 0, 0, 0, 3 } }; // #
		                                               // #
									                   // #
		                                               // #

		std::vector<Rect> Cross3{ { 0, 1, 2, 1 },   // ### plus
		
		                          { 1, 0, 1, 2 } }; // #        #
		                                            // # makes ###
				                                    // #        #

		std::vector<Rect> Square2x2{ { 0, 0, 1, 1 } }; // ##
		                                               // ##

		std::vector<Rect> BackwardsL3{ { 0, 0, 2, 0 },   // ### plus

		                               { 2, 0, 2, 2 } }; // #         #
									                     // # makes   #
														 // #       ###

		std::array<std::vector<Rect>, 5> collisionShapes{ Horizontal4, Cross3, BackwardsL3, Vertical4,  Square2x2  };

		std::vector<Rect> floorCollider{ { 0, 0, 6, 0 } }; 
		std::vector<Rect> wallsCollider{ { 0, 0, 0, INT_MAX } }; 

		// By separating rock collections by some y value, we can reduce the number of checks 
		const int ySeparation{ 100 };

		std::vector<std::vector<Rock*>> settledRocksPartitioned;

		settledRocksPartitioned.push_back(std::vector<Rock*>{});

		std::vector<Rock*> walls{ new Rock{ -1, 0, wallsCollider } , new Rock{ 7, 0, wallsCollider } };

		settledRocksPartitioned[0].push_back( new Rock{ 0, 0, floorCollider });
		
		int highestPoint{ 0 };

		auto rockShape{ collisionShapes.begin() };
		auto jet{ jets.begin() };

		const int totalRocks{ 2022 };
		int rockCount{ 0 };

		while (++rockCount <= totalRocks)
		{
			Rock* testrock{ new Rock{ 2, highestPoint + 4, *rockShape } };

			logger << 'r' << rockCount << ":s" << rockShape - collisionShapes.begin() << ':';

			// Loop through rock shapes
			if (++rockShape == collisionShapes.end())
			{
				rockShape = collisionShapes.begin();
			}

			bool bSettled{ false };

			while (!bSettled)
			{
				// Jet pushes rock horizontally
				testrock->translate(jetDirection(jet));
				
				logger << *jet;

				// Loop through jet string
				if (++jet == jets.end())
				{
					jet = jets.begin();
				}
				
				// Horizontal jet movement collision check
				{
					const int rockTop{ testrock->boundingRect().upperRight.y };
					const int rockBottom{ testrock->location.y };

					const size_t rockUpperContainer{ static_cast<size_t>(rockTop / ySeparation) };		
					const size_t rockLowerContainer{ static_cast<size_t>(rockBottom / ySeparation) };

					if (settledRocksPartitioned.size() < rockUpperContainer + 1)
					{
						settledRocksPartitioned.push_back(std::vector<Rock*>{});
					}

					bool bCollides{ false };

					for (auto *rock : walls)
					{
						if (testrock->collides(*rock))
						{
							bCollides = true;
							testrock->undoTranslate();
							break;
						}
					}

					for (auto *rock : walls)
					{
						if (testrock->collides(*rock))
						{
							bCollides = true;
							testrock->undoTranslate();
							break;
						}
					}

					for (auto i{ rockLowerContainer }; i <= rockUpperContainer; ++i)
					{
						for (auto *rock : settledRocksPartitioned[i])
						{
							if (testrock->collides(*rock))
							{
								bCollides = true;
								testrock->undoTranslate();
								break;
							}
						}
						if (bCollides)
						{
							break;
						}
					}

					if (bCollides)
					{
						logger << '*';
					}
				}

				// drawRange({ -3, -2 }, { 9, 15 }, settledRocksPartitioned[0], testrock);

				// Rock falls down 1 space
				testrock->translate(0, -1);

				{
					const int rockTop{ testrock->boundingRect().upperRight.y };
					const int rockBottom{ testrock->location.y };

					const size_t rockUpperContainer{ static_cast<size_t>(rockTop / ySeparation) };		
					const size_t rockLowerContainer{ static_cast<size_t>(rockBottom / ySeparation) };

					for (auto i{ rockLowerContainer }; i <= rockUpperContainer; ++i)
					{
						for (auto *rock : settledRocksPartitioned[i])
						{
							if (testrock->collides(*rock))
							{
								bSettled = true;
								testrock->undoTranslate();
								break;
							}
						}
						if (bSettled)
						{
							break;
						}
					}
				}

				const int rockTop{ testrock->boundingRect().upperRight.y };
				const int rockBottom{ testrock->location.y };

				const size_t rockUpperContainer{ static_cast<size_t>(rockTop / ySeparation) };		
				const size_t rockLowerContainer{ static_cast<size_t>(rockBottom / ySeparation) };

				if (bSettled)
				{
					logger << '\n';

					if (testrock->boundingRect().upperRight.y > highestPoint)
					{
						highestPoint = testrock->boundingRect().upperRight.y;
					}

					for (auto i{ rockLowerContainer }; i <= rockUpperContainer; ++i)
					{
						++testrock->pointersTo;
						settledRocksPartitioned[i].push_back(testrock);
					}
				}
			}
		}

		for (auto &&rockCont : settledRocksPartitioned)
		{
			for (auto *rock : rockCont)
			{
				if (--(rock->pointersTo) <= 0)
				{
					delete rock;
				}
			}
		}

		utils::printAnswer("height of rock tower after 2022 rocks have fallen: ", highestPoint);
	}
};

// Simulate as previous, but:
// Check what the floor looks like and store that state in a list
// Compare new states against old ones until a repeating pattern is found
// Sum as many repeats' height differences as fit in the rock count
// Then simulate the remainder as before
namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		Logger logger{ utils::getFilePath(__FILE__, "log-pt2"), false, false };

		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string jets;
		std::getline(inf, jets);

		std::vector<Rect> Horizontal4{ { 0, 0, 3, 0 } }; // ####

		std::vector<Rect> Vertical4{ { 0, 0, 0, 3 } }; // #
		                                               // #
									                   // #
		                                               // #

		std::vector<Rect> Cross3{ { 0, 1, 2, 1 },   // ### plus
		
		                          { 1, 0, 1, 2 } }; // #        #
		                                            // # makes ###
				                                    // #        #

		std::vector<Rect> Square2x2{ { 0, 0, 1, 1 } }; // ##
		                                               // ##

		std::vector<Rect> BackwardsL3{ { 0, 0, 2, 0 },   // ### plus

		                               { 2, 0, 2, 2 } }; // #         #
									                     // # makes   #
														 // #       ###

		std::array<std::vector<Rect>, 5> collisionShapes{ Horizontal4, Cross3, BackwardsL3, Vertical4,  Square2x2  };

		std::vector<Rect> floorCollider{ { 0, 0, 6, 0 } }; 

		CollisionCollection rocks;
		rocks.push(new Rock{ 0, 0, floorCollider });

		size_t rockShape{ 0 };
		size_t jet{ 0 };

		const long long totalRocks{ 1000000000000 };
		long long rockCount{ 0 };

		std::set<RocksState> previousStates;

		long long skipRepeatChecksFor{ static_cast<long long>(jets.length()) };

		bool bRepetitionFound{ false };
		while (++rockCount <= totalRocks)
		{
			// Foolishly I left the purging stuff in the floor checker
			// highest point needs to be adjusted when purging floors
			// haven't been able to get the right answer with purging and shifting
			Rock* testrock{ new Rock{ 2, rocks.highestPoint + 4, collisionShapes[rockShape] } };

			logger << 'r' << rockCount << ":s" << rockShape << ':';

			// Loop through rock shapes
			if (++rockShape == collisionShapes.size())
			{
				rockShape = 0;
			}

			bool bSettled{ false };

			// drawRange({ -1, -1 }, { 8, rocks.highestPoint }, rocks, testrock);

			while (!bSettled)
			{
				// Jet pushes rock horizontally
				testrock->translate(jetDirection(jets[jet]), 0);

				logger << jets[jet];

				// Loop through jet string
				if (++jet == jets.size())
				{
					jet = 0;
				}
				
				// Horizontal jet movement collision check
				if (testrock->collidesWithWall() || rocks.collides(testrock))
				{
					logger << '*';

					testrock->undoTranslate();
				}

				// Rock falls down 1 space
				testrock->translate(0, -1);

				if (rocks.collides(testrock))
				{
					testrock->undoTranslate();
					rocks.push(testrock);
					bSettled = true;

					logger << '\n';
				}
			}

			if (testrock->top() > rocks.highestPoint)
			{
				rocks.cummulativeHeight += testrock->top() - rocks.highestPoint;
				rocks.highestPoint = testrock->top();
			}

			// drawRange({ -1, -1 }, { 8, rocks.highestPoint }, rocks, testrock);

			RocksState newState{ rockShape, jet, wallToWall({ 0, rocks.highestPoint }, rocks), rocks.cummulativeHeight, rockCount };
			if (!bRepetitionFound && rockCount > skipRepeatChecksFor)
			{
				if (previousStates.contains(newState))
				{
					bRepetitionFound = true;

					DL("states matched:\n");
					auto patternStart{ *(previousStates.find(newState)) };
					DL("jet index:   " << patternStart.jetIndex << " vs " << newState.jetIndex);
					DL("shape index: " << patternStart.rockIndex << " vs " << newState.rockIndex);
					DL("rocks wall-wall span: ");
					for (size_t i{ 0 }; i < patternStart.highFloor.size(); ++i)
					{
						DP(patternStart.highFloor[i] << " ");
					}
					DL("vs")
					for (size_t i{ 0 }; i < newState.highFloor.size(); ++i)
					{
						DP(newState.highFloor[i] << " ");
					}
					DL("\n");

					auto heightDiff{ newState.highPoint - patternStart.highPoint };
					auto countDiff{ newState.count - patternStart.count };

					auto loopsRemaining{ totalRocks - rockCount };
					auto multiplesOfLoop{ loopsRemaining / countDiff };
					auto remainderAfterSkip{ loopsRemaining % countDiff };

					DL("state matched at rock:      " << patternStart.count);
					DL("pattern loops at rock:      " << rockCount);
					DL("pattern length:             " << countDiff);

					DL("height at start of pattern: " << patternStart.highPoint);
					DL("height at end of pattern:   " << newState.highPoint);
					DL("height difference:          " << heightDiff);

					DL("rocks remaining:            " << loopsRemaining);
					DL("full pattern loops:         " << multiplesOfLoop);
					DL("height from pattern:        " << multiplesOfLoop * heightDiff);
					
					rocks.cummulativeHeight += (multiplesOfLoop * heightDiff);

					DL("height after pattern aded:  " << rocks.cummulativeHeight);
					DL("rocks skipped:              " << multiplesOfLoop * countDiff);
					DL("remaining rocks:            " << remainderAfterSkip);
					
					rockCount = totalRocks - remainderAfterSkip; 

				}

				previousStates.insert(newState);

			}
		}

		utils::printAnswer("highest point after 1000000000000 rocks: ", rocks.cummulativeHeight);

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
