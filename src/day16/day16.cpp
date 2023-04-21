// --- Day 16: Proboscidea Volcanium ---

// Release pressure to rescue elephants
// Want to have another stab at this so that part 2 gets solved in a reasonable amount of time!

#include <array>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "debug.h"
#include "utils.h"

const std::string startValve{ "AA" };
const int startTime{ 30 };
const int startTimeWithElephantHelp{ 26 };

struct Path
{
	Path(const std::string &_from, const std::string &_to) :
		a{ _from < _to ? _from : _to },
		b{ _from < _to ? _to : _from }
	{
	}
	const std::string a;
	const std::string b;

	friend bool operator<(const Path &lhs, const Path &rhs)
	{
		return lhs.a < rhs.a || (lhs.a == rhs.a && lhs.b < rhs.b);
	} 
	friend bool operator==(const Path &lhs, const Path &rhs)
	{
		return lhs.a == rhs.a && lhs.b == rhs.b;
	}
};

struct Valve
{
	using valve_dists = std::map<Path, int>;

	std::string id;
	int flow;

	valve_dists static distances;

	int release(int time)
	{
		return time * flow;
	}

	static int dist(const std::string &from, const std::string &to)
	{
		return distances[ { from, to } ];
	}

	int dist(const std::string &to)
	{
		return distances[ { id, to } ];
	}

	friend bool operator<(const Valve &lhs, const Valve &rhs)
	{
		return lhs.id < rhs.id;
	}

	friend bool operator<(const Valve &lhs, const std::string &rhs)
	{
		return lhs.id < rhs;
	}
	
	friend bool operator==(const Valve &lhs, const std::string &rhs)
	{
		return lhs.id == rhs;
	}

};

struct ValvesState
{
	using valve_cont = std::map<std::string, Valve>;

	ValvesState(){ closedValves.erase(startValve); }

	ValvesState(int _time, std::string _location, int _pressureReleased, valve_cont _closedValves)
		: time{ _time },
		location{ _location },
		pressureReleased{ _pressureReleased },
		closedValves{ _closedValves }
	{
		if (pressureReleased > bestRelease)
		{
			bestRelease = pressureReleased;
		}
	}

	int time{ startTime };
	std::string location{ startValve };
	int pressureReleased{ 0 };
	valve_cont closedValves{ allValves };
	static int bestRelease;

	static valve_cont allValves;

	static bool allValvesContains(const std::string &matchId)
	{
		return allValves.contains(matchId);
	}
	
	ValvesState open(const std::string &id)
	{
		int timeAfter{ time - Valve::dist(location, id) - 1 };

		int release{ closedValves[id].release(timeAfter) };

		auto closedAfter{ closedValves };
		closedAfter.erase(id);

		return { timeAfter,
		         id,
				 pressureReleased + release,
				 closedAfter };
	}

	static void addToAllValves(const std::string id, int flow)
	{
		allValves.insert( { id, { id, flow }} );
	}

	int perfectConditionsRelease()
	{
		int leastDist{ INT_MAX };
		std::set<int, std::greater<>> orderedFlows;

		for (auto &&valve : closedValves)
		{
			orderedFlows.insert(valve.second.flow);

			if (auto dist{ Valve::dist(location, valve.first) }; dist < leastDist)
			{
				leastDist = dist;
			}
		}

		int theoreticalTime{ time - leastDist + 1 };
		int potentialRelease{ pressureReleased };
		for (auto flow : orderedFlows)
		{
			if ((theoreticalTime -= 2) < 1)
			{
				break;
			}

			potentialRelease += (theoreticalTime) * flow;
		}
		
		return potentialRelease;
	}

	int tryAll()
	{
		if (closedValves.empty() || time < 2)
		{
			return bestRelease;
		}

		if (perfectConditionsRelease() < bestRelease)
		{
			return bestRelease;
		}

		for (auto &&v : closedValves)
		{
			open(v.first).tryAll();
		}

		return bestRelease;
	}
};

int releaseAmount(const int dist, const int flow, const int time)
{
	return (time - dist - 1) * flow;
}

struct ValvesStateTwoOpeners : public ValvesState
{
	static int bestDuoRelease;

	ValvesStateTwoOpeners() : ValvesState() { time = startTimeWithElephantHelp; }

	std::string location2{ startValve };
	std::array<int, 2> extraDistance{ { 0, 0 } };

	ValvesStateTwoOpeners(int _time, std::string _location1, std::string _location2, int _pressureReleased, valve_cont _closedValves)
		:
		ValvesState(_time, _location1, _pressureReleased, _closedValves),
		location2{ _location2 }
	{
		if (pressureReleased > bestDuoRelease)
		{
			bestDuoRelease = pressureReleased;
			// Printing a line so I can see something's actually happening!
			// DL(DCOL << "36m" << "new best: " << bestDuoRelease);
		}
	}

	int perfectConditionsRelease()
	{
		int leastDist{ INT_MAX };
		std::set<int, std::greater<>> orderedFlows;

		for (auto &&valve : closedValves)
		{
			orderedFlows.insert(valve.second.flow);

			if (auto dist{ Valve::dist(location, valve.first) }; dist < leastDist)
			{
				leastDist = dist;
			}
			if (auto dist{ Valve::dist(location2, valve.first) }; dist < leastDist)
			{
				leastDist = dist;
			}
		}

		int theoreticalTime{ time - leastDist + 1 };
		int potentialRelease{ pressureReleased };
		
		for (auto iter{ orderedFlows.begin() }; iter != orderedFlows.end(); ++iter)
		{
			if ((theoreticalTime -= 2) < 1)
			{
				break;
			}

			// We increment twice each loop, using both values if possible
			auto first{ *iter };
			++iter;
			
			// We'll add the best two * time remaining together (if there are two)
			if (iter != orderedFlows.end())
			{
				potentialRelease += (first + *iter) * theoreticalTime;
			}
			else
			{
				potentialRelease += (first) * theoreticalTime;
				break;
			}
		}

		return potentialRelease;
	}

	// This one has an exclude in hopes that my culling can work quicker in tryAll()
	int perfectConditionsRelease(const std::string &exclude)
	{
		int leastDist{ INT_MAX };
		std::set<int, std::greater<>> orderedFlows;

		// Treat exclude as if it's the current location in a new state
		for (auto &&valve : closedValves)
		{
			if (valve.first == exclude)
			{
				continue;
			}
			
			orderedFlows.insert(valve.second.flow);

			if (auto dist{ Valve::dist(exclude, valve.first) }; dist < leastDist)
			{
				leastDist = dist;
			}
			// This checks location2, but it could well be location 
			// we don't check if it's the elephant or the person doing this search
			if (auto dist{ Valve::dist(location2, valve.first) }; dist < leastDist)
			{
				leastDist = dist;
			}
		}

		int theoreticalTime{ time - leastDist + 1 };
		int potentialRelease{ pressureReleased };
		
		for (auto iter{ orderedFlows.begin() }; iter != orderedFlows.end(); ++iter)
		{
			if ((theoreticalTime -= 2) < 1)
			{
				break;
			}

			// We increment twice each loop, using both values if possible
			auto first{ *iter };
			++iter;
			
			if (iter != orderedFlows.end())
			{
				potentialRelease += (first + *iter) * theoreticalTime;
			}
			else
			{
				potentialRelease += (first) * theoreticalTime;
				break;
			}

		}

		return potentialRelease;
	}

	// Need some way of only opening one valve
	ValvesStateTwoOpeners open(const std::string &id1, const std::string &id2)
	{
		int dist1{ time };
		int dist2{ time };

		int release1{ 0 };
		int release2{ 0 };

		auto closedAfter{ closedValves };

		// We'll call an empty string no opening at all
		if (id1 != "")
		{
			dist1 = Valve::dist(location, id1) + extraDistance[0];
			release1 = closedValves[id1].release(time - dist1 - 1);
			closedAfter.erase(id1);
		}
		if (id2 != "")
		{
			dist2 = Valve::dist(location2, id2) + extraDistance[1];
			release2 = closedValves[id2].release(time - dist2 - 1);
			closedAfter.erase(id2);
		}

		// Distance to travel + 1 for time to open the valve
		int timeDecrement{ std::min(dist1, dist2) + 1 };

		int timeAfter{ time - timeDecrement };

		ValvesStateTwoOpeners newState{ timeAfter,
		         id1,
				 id2,
				 pressureReleased + release1 + release2,
				 closedAfter };

		// If person/elephant needs more time to reach their valve,
		// add it to the dist of the next valve they choose
		// The other will be 0, so we could represent this as a +ve / -ve / 0 value int instead 
		if (dist1 > dist2)
		{
			newState.extraDistance[0] = extraDistance[0] + dist1 - dist2;
		}
		else if (dist2 > dist1)
		{
			newState.extraDistance[1] = extraDistance[1] + dist2 - dist1;
		}

		return newState;
	}

#ifdef DEBUG
	static int iterations;
#endif
	// Does find the correct answer after a few minutes
	int tryAll()
	{
#ifdef DEBUG
		++iterations;
#endif
		// Static member bestDuoRelease is reassigned every time a branch gets a higher score
		if (closedValves.empty() || time < 2)
		{
			return bestDuoRelease;
		}

		if (perfectConditionsRelease() < bestDuoRelease)
		{
			return bestDuoRelease;
		}

		for (auto &&v1 : closedValves)
		{
			if (perfectConditionsRelease(v1.first) < bestDuoRelease)
			{
				continue;
			}

			for (auto &&v2 : closedValves)
			{
				if (v1.first == v2.first)
				{
					continue;
				}
				open(v1.first, v2.first).tryAll();
			}
		}

		return bestDuoRelease;
	}

};

// Adds to lists which are static members of Valve and ValvesState
void makeValveLists(const std::string& infile)
{
	auto valveInfo{ utils::bufferLines(infile) };

	std::map<std::string, std::vector<std::string>> neighbours;

	for (auto &&valve : valveInfo)
	{
		// Offset from the start of the line to valve name = 6
		const std::string tunnelFrom{ valve.data() + 6, valve.data() + 8 };

		// Offset to flow rate = 23
		if(auto flow{ std::atoi(valve.data() + 23) }; flow > 0 || tunnelFrom == startValve)
		{
			ValvesState::addToAllValves(tunnelFrom, flow);
		} 

		// The last 'e' in the line (in the word "valve" or "valves") precedes a list of neighbours
		const auto lastE { valve.find_last_of('e') };
		const auto tunnelListBegin{ valve[lastE + 1] == 's' ? lastE + 3 : lastE + 2 };

		std::string_view tunnelList{ valve.data() + tunnelListBegin, valve.length() - tunnelListBegin };
		
		neighbours[tunnelFrom] = utils::split(tunnelList, ", ");
	}
	

	for (auto &&valveNeighboursPair : neighbours)
	{
		auto valve{ valveNeighboursPair.first };

		std::map<std::pair<std::string, std::string>, int> valveDists;

		// Dist from valve to itself = 0
		valveDists[{ valve, valve }] = 0;

		std::deque<std::string> queue;
		queue.push_back(valve);

		while (queue.size())
		{
			for (auto &&neighbour : neighbours[queue.front()])
			{
				if (!valveDists.contains({ valve, neighbour }))
				{
					// valveDists includes duplicates (flipped directions) & 0-flow valves
					auto dist{ valveDists[{ valve, queue.front() }] + 1 };

					valveDists[ { valve, neighbour } ] = dist;

					// End up copying the dists we want to keep, could probably do it with one list
					if (ValvesState::allValvesContains(valve) && ValvesState::allValvesContains(neighbour))
					{
						Valve::distances[ { valve, neighbour } ] = dist;
					}

					queue.push_back(neighbour);
				}
			}
			queue.pop_front();
		}
	}
}

// Initialise static members out of line
Valve::valve_dists Valve::distances{ Valve::valve_dists{} };
ValvesState::valve_cont ValvesState::allValves{ ValvesState::valve_cont{} };
int ValvesState::bestRelease{ 0 };
int ValvesStateTwoOpeners::bestDuoRelease{ 0 };

#ifdef DEBUG
int ValvesStateTwoOpeners::iterations{ 0 };
#endif

// Follow the largest possible releases with no concern for following moves, for a decent baseline
int chaseLargestReleases()
{
	ValvesState vs;

	while (vs.time > 1 && vs.closedValves.size() > 0)
	{
		int largestRelease{ 0 };
		std::string largestReleaseValve;
		for (auto &&cv : vs.closedValves) 
		{
			const auto release { cv.second.release(vs.time - 1 - Valve::dist(vs.location, cv.first)) };
			if (release > largestRelease)
			{
				largestRelease = release;
				largestReleaseValve = cv.first;
			}
		}
		vs = vs.open(largestReleaseValve);
	}

	return vs.pressureReleased;
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

		std::map<Path, int> distances;
		std::map<std::string, int> valves;

		makeValveLists(infile);

		chaseLargestReleases();

		// 12486 iterations to get answer
		int answer{ ValvesState{}.tryAll() };

		utils::printAnswer("highest possible pressure release in 30 minutes: ", answer);
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

		std::map<Path, int> distances;
		std::map<std::string, int> valves;

		makeValveLists(infile);

		// Well this worked after about 5 MINUTES 
		// Clearly there's room for improvement here!
		int answer{ ValvesStateTwoOpeners{}.tryAll() };

		utils::printAnswer("highest possible pressure release in 26 minutes with an elephant's help: ", answer);

		// 10349731 Iterations (part 1 took 12486!)
		// DL("This took " << ValvesStateTwoOpeners::iterations << " iterations!");
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
