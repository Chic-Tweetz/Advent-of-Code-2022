// --- Day 16: Proboscidea Volcanium ---

#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "utils.h"
#include "timer.h"

struct UsefulValves
{
	using bits_t = std::uint16_t;

	using map_t = std::map<bits_t, int>;
	static map_t flows; // id = key
	static map_t distances; // bitwise or | 2 ids for key

	static std::vector<map_t*> distsPerTime;
	// Might as well store the flow + its valve - exactly the same as the map but ordered by flow
	static std::vector<std::pair<bits_t, int>> orderedFlows;
	static std::map<bits_t, std::vector<std::pair<int, bits_t>>> orderedDistancesPerValve;
	// you know what you can mark valves as open if their dist is greater than time left
	// that should prevent some checks?
};

struct Lut16 // Unused?
{
	using bits_t = UsefulValves::bits_t;
	static const std::vector<bits_t> twoToThe;
	static const std::map<bits_t, int> logBase2;
	
	static std::vector<bits_t> makeTwoPowerList()
	{
		std::vector<bits_t> list;
		bits_t j{ 1 };
		for (int i{ 0 }; j != 0; ++i)
		{
			list.push_back(j);
			j <<= 1;
		}
		return list;
	}

	static std::map<bits_t, int> makeLogBase2Map()
	{
		static std::map<bits_t, int> map;
		bits_t j{ 1 };
		for (int i{ 0 }; j != 0; ++i)
		{
			map[j] = i;
			j <<= 1;
		}
		return map;
	}
};

const std::vector<Lut16::bits_t> Lut16::twoToThe{ Lut16::makeTwoPowerList() };
const std::map<Lut16::bits_t, int> Lut16::logBase2{ Lut16::makeLogBase2Map() };

struct AllValves
{
	using bits_t = UsefulValves::bits_t;
	struct ValveInfo
	{	
		ValveInfo(std::string_view fromInput) :
			flow{ std::atoi(fromInput.data() + 23) }
		{
			auto neighboursStart{ fromInput.find_last_of('e') };
			neighboursStart += fromInput[neighboursStart + 1] == 's' ? 3 : 2;
			neighbourIds = utils::split(std::string{ fromInput.data() + neighboursStart }, ", ");
		}

		int flow;
		std::vector<std::string> neighbourIds;
	};

	static std::map<std::string, ValveInfo*> list;
	static std::map<std::pair<std::string, std::string>, int> distances;
	static std::map<std::string, bits_t> bitIds;

	static const int totalTime;

	static void push(std::string_view fromInput)
	{
		// ValveInfo newValve{ fromInput };
		list[std::string{ fromInput[6] } + fromInput[7]] = new ValveInfo(fromInput);
	}

	// Build UsefulValves lists from all valves, all valves can be forgotten about after that
	static void compress()
	{
		bitIds["AA"] = 0;
		bits_t bitId{ 1 };

		for (auto &&valve : list)
		{
			bool bUseful{ valve.second->flow > 0 };
			if (bUseful)
			{
				UsefulValves::flows[bitId] = valve.second->flow;
				bitIds[valve.first] = bitId;
				bitId <<= 1;
				//++bitId;
			}

			std::deque<std::string> queue{ valve.first };

			// We need 1 minute to open valves so we start at 1 instead of 0 here
			// Meaning (time - distance) * flow = total pressure released
			distances[{ queue.front(), queue.front() }] = 1;

			while (queue.size())
			{
				for (auto &&neighbour : list[queue.front()]->neighbourIds)
				{
					if (!distances.contains({ valve.first, neighbour }))
					{
						distances[{ valve.first, neighbour }] = distances[{ valve.first, queue.front() }] + 1;
						queue.push_back(neighbour);
					}
				}
				queue.pop_front();
			}
		}
		
		for (auto &&distpair : distances)
		{
			if (distpair.first.first == distpair.first.second) continue;
			if (bitIds.contains(distpair.first.first) && bitIds.contains(distpair.first.second))
			{
				bits_t combined{ static_cast<uint16_t>(bitIds[distpair.first.first] | bitIds[distpair.first.second]) };
				UsefulValves::distances[combined] = distpair.second;
			}
		}

		UsefulValves::distsPerTime = { static_cast<size_t>(totalTime + 1), nullptr };

		auto i{ totalTime };
		while(i > 0)
		{
			int nextChange{ -1 };
			auto *newMap{ new UsefulValves::map_t };
			for (auto &&d : UsefulValves::distances)
			{
				if (d.second <= i)
				{
					(*newMap)[d.first] = d.second;
				}
				if (d.second > nextChange && d.second < i)
				{
					nextChange = d.second;
				}
			}
			while (i > nextChange)
			{
				UsefulValves::distsPerTime[static_cast<size_t>(i)] = newMap;
				--i;
			}
		}

		for (auto &&valve : UsefulValves::flows)
		{
			UsefulValves::orderedDistancesPerValve[valve.first] = std::vector<std::pair<int, bits_t>>{};
			for (auto &&distpair : UsefulValves::distances)
			{
				// Is valve part of this dist pair, and not from AA (if they're equal it's dist from AA which has bit ID 0)
				if (distpair.first & valve.first && distpair.first != valve.first)
				{
					UsefulValves::orderedDistancesPerValve[valve.first].push_back( { distpair.second, distpair.first & ~valve.first } );
				}
			}
			std::sort(UsefulValves::orderedDistancesPerValve[valve.first].begin(), UsefulValves::orderedDistancesPerValve[valve.first].end());

		}

		UsefulValves::orderedFlows.reserve(UsefulValves::flows.size());
		for (auto &&flow : UsefulValves::flows)
		{
			UsefulValves::orderedFlows.push_back({ flow.first, flow.second });
		}
		std::sort(UsefulValves::orderedFlows.begin(), UsefulValves::orderedFlows.end(),
			[](auto &l, auto &r)
			{
				return l.second > r.second;
			});
	}
};

// Initialise static members...
std::map<std::string, AllValves::ValveInfo*> AllValves::list;
std::map<std::pair<std::string, std::string>, int> AllValves::distances;
std::map<std::string, std::uint16_t> AllValves::bitIds;
const int AllValves::totalTime{ 30 };

UsefulValves::map_t UsefulValves::flows;
UsefulValves::map_t UsefulValves::distances;
std::vector<std::pair<UsefulValves::bits_t, int>> UsefulValves::orderedFlows;
std::map<UsefulValves::bits_t, std::vector<std::pair<int, UsefulValves::bits_t>>> UsefulValves::orderedDistancesPerValve;
std::vector<UsefulValves::map_t*> UsefulValves::distsPerTime;

// Puzzle 1, single worker
struct State
{
	using bits_t = UsefulValves::bits_t;

	static const int totalTime;

	// State() : potential{ idealRelease(time, 0) } {}
	State() : opened{ initialOpened() }, potential{ initialPotential() } {}

	State(const State &prev, bits_t open) :
		time{ prev.time - UsefulValves::distances[open | prev.curr] },
		opened{ static_cast<bits_t>(prev.opened | open | unreachable(open, time))},
		curr{ open },
		released{ prev.released + time * UsefulValves::flows[open] },
		potential{ idealReleaseFromLastOpened() + released }
		// potential{ idealRelease(time, opened) + released }
	{
		if (released > bestRelease)
		{
			bestRelease = released;
		}
	}

	static int bestRelease;

	static int initialPotential()
	{
		int potential{ 0 };
		int penalty{ 0 };
		for (auto &&flowpair : UsefulValves::orderedFlows)
		{
			// Will be at least 1 extra distance for subsequent releases so we add to penalty
			potential += flowpair.first * (totalTime - UsefulValves::distances[flowpair.second] - penalty++);
		}
		
		return potential;
	}

	// Could just use 0 but I might have the 16th bit be unused and I don't want problems
	bits_t initialOpened()
	{
		bits_t ignoreValveIds{ 0 };
		for (bits_t i{ 1 }; i != 0; i <<= 1)
		{
			if (!UsefulValves::flows.contains(i))
			{
				ignoreValveIds |= i;
			}
		}
		return ignoreValveIds;
	}

	int idealReleaseFromLastOpened()
	{
		static std::map<std::pair<int, bits_t>, int> cache;

		if (cache.contains( {time, opened} ) )
		{
			return cache[{time,opened}];
		}

		int potential{ 0 };
		int penalty{ 0 };
		for (auto &&flowpair : UsefulValves::orderedFlows)
		{
			if (flowpair.second & ~opened)
			{
				// Will be at least 1 extra distance for subsequent releases so we add to penalty
				potential += flowpair.first * std::max((time - UsefulValves::distances[flowpair.second | curr] - penalty++), 0);
			}
		}
		
		cache[{time, opened}] = potential;
		return potential;
	}

	static int idealRelease(int time, bits_t openValves)
	{
		static std::map<std::pair<int, bits_t>, int> cache;

		if (cache.contains({ time, openValves }))
		{
			return cache[ { time, openValves } ];
		}

		auto potentialTime{ time };
		auto perfectRelease{ 0 };
		// auto potentialFlow{ 0 };
		// auto releaseWhileOpening{ 0 };

		for (auto &&valve : UsefulValves::orderedFlows)
		{
			if (openValves & valve.first) continue;
			// if (--potentialTime == 1) releaseWhileOpening += potentialFlow;
			if (--potentialTime == 0) break;

			// Theoretical minimal time to open a valve = 1 dist + 1 min to open = 2
			// releaseWhileOpening += 2 * potentialFlow;
			// potentialFlow += UsefulValves::flows[valve.first];
			perfectRelease += UsefulValves::flows[valve.first] * potentialTime;
		}

		cache[{ time, openValves }] = perfectRelease;

		return perfectRelease;
	}

	static bits_t unreachable(bits_t from, int time)
	{
		//  static std::map<std::pair<bits_t, int>, bits_t> cache;
		static std::map<bits_t, std::map<int, bits_t>> cache;

		if (cache.contains(from))
		{
			if (cache[from].contains(time))
			{
				return cache[from][time];
			}
		}

		bits_t unreachableValves{ 0 };
		for (bits_t mask{ 1 }; mask != 0; mask <<= 1)
		{
			if (UsefulValves::distances[from | mask] >= time)
			{
				unreachableValves |= mask;
			}
		}

		cache[from][time] = unreachableValves;

		return unreachableValves;
	}

	const int time{ totalTime };
	const bits_t opened{ 0 };
	const bits_t curr{ 0 };
	const int released{ 0 };
	int potential{ 0 };

	State open(bits_t id) const
	{
		return { *this, id };
	}

	static int openAll(const State &prevState)
	{
		if (prevState.potential < bestRelease) return bestRelease;

		for (bits_t i{ 1 }; i != 0; i <<= 1)
		{
			if (i & ~prevState.opened)
			{
				auto newState{ prevState.open(i) };
				openAll(newState);
			}
		}

		return bestRelease;
	}
};

int State::bestRelease{ 0 };
const int State::totalTime{ 30 };

struct StateTwoWorkers
{
	using v = UsefulValves;
	using bt = v::bits_t;
	using bt2 = std::uint32_t;
	using stack_t = std::stack<StateTwoWorkers>;

	static const int totalTime;

	// Default constructor will give initial state
	StateTwoWorkers() :
		time{ totalTime },
		totalReleased{ 0 },
		openedValves{ initialUnused() },
		queue{ 0 },
		availableWorkers{ 0 },
		potentialRelease{ initialPotential() }
	{
	}

	StateTwoWorkers(int _time, int _totalReleased, bt _openedValves, bt2 _queue, bt2 _workers) :
		time{ _time },
		totalReleased{ _totalReleased },
		openedValves{ _openedValves },
		queue{ _queue },
		availableWorkers{ _workers },
		potentialRelease{ perfectRelease() }
	{
	}

	int time; // Starts high, counts down
	int totalReleased; 
	// Not necessarily opened, but ~opened will flip to 1 valves yet to be opened
	bt openedValves;
	// Right 16 bits represent the queued valve id (bit mask), left bits the time it will be opened (int)
	bt2 queue; 
	// We can represent 2 workers with a 32 bit word, 16 bits each and shifting to get each one
	bt2 availableWorkers; // There will always be a right worker + either a worker in the queue or in the left
	int potentialRelease; // High estimate for remaining pressure release

	static int initialPotential()
	{
		// Sum of (Time - Distance from start valve ("AA") to each useful valve) * valve's flow
		int potential{ 0 };
		for (auto &&flowpair : v::orderedFlows)
		{
			potential += flowpair.second * (totalTime - v::distances[flowpair.first]);
		}
		
		return potential;
	}

	// Flip bits which don't represent any valve to 1 (only 16th bit for puzzle input, 11 bits for test)
	static bt initialUnused()
	{
		static bool bSet{ false };
		static bt ignoreValveIds{ 0 };

		if (!bSet)
		{
			for (bt i{ 1 }; i != 0; i <<= 1)
			{
				if (!v::flows.contains(i))
				{
					ignoreValveIds |= i;
				}
			}
			bSet = true;
		}

		return ignoreValveIds;
	}

	// Common methods for using left / right 16 bits of 32 bit words
	static bt leftbits(bt2 twobts) { return twobts >> 16; }
	static bt rightbits(bt2 twobts)	{ return twobts & 0b0000'0000'0000'0000'1111'1111'1111'1111; }
	static bt combineLeftRight(bt2 twobts) { return twobts | (twobts >> 16); }

	// Making intentions clearer, I was repeating a lot of these and it was getting hard to follow
	inline int queueTime() { return static_cast<int>(leftbits(queue)); }
	inline bt queueValve() { return rightbits(queue); }
	inline int queueFlow() { return v::flows[queueValve()]; }
	inline int queueRelease() { return queueFlow() * queueTime(); }
	inline bt2 queueValveLeftShift() { return static_cast<bt2>(queueValve()) << 16; }
	inline bt rightWorker() { return rightbits(availableWorkers); }
	inline bt leftWorker() { return leftbits(availableWorkers); }
	inline int rightWorkerDist(bt valve) { return v::distances[rightWorker() | valve]; }
	inline int leftWorkerDist(bt valve) { return v::distances[leftWorker() | valve]; }
	inline int leftDist(bt2 twoWorkers, bt valve) { return v::distances[leftbits(twoWorkers) | valve]; }
	inline int rightDist(bt2 twoWorkers, bt valve) { return v::distances[rightbits(twoWorkers) | valve]; }

	// Perfect = the nearest worker opening each remaining valve from their current locations
	int perfectRelease()
	{	
		int acc{ totalReleased + queueRelease() };

		bt2 workerLocations{ queueValveLeftShift() | availableWorkers };

		// If a worker is queued, we use the time they finish opening their queued valve
		auto leftWorkerTime{ queue ? queueTime() : time };

		for (auto &&valveFlowPair : v::flows)
		{
			if (!(valveFlowPair.first & openedValves))
			{
				acc += valveFlowPair.second * std::max(leftWorkerTime - leftDist(workerLocations, valveFlowPair.first),
					                                             time - rightDist(workerLocations, valveFlowPair.first));
			}
		}

		return acc;
	}

	// Push new states to the stateStack (we don't return new states because there are cases where none is made), it's a bit if-elsey isn't it
	void openValve(bt valve, stack_t &stateStack)
	{
		// This can be < 0 if leftWorker can reach valve but not rightWorker hence std::max
		auto rightWorkerTime{ std::max(0, time - rightWorkerDist(valve)) };

		if (!queue) // If two workers are available, queue the closest to open valve
		{
			auto leftWorkerTime{ std::max(0, time - leftWorkerDist(valve)) };

 			// If both have managed to be 0, return - it does happen a couple of hundred times but I don't think it should be able to!
			if (!(rightWorkerTime || leftWorkerTime))
			{
				return;
			}

			if (leftWorkerTime >= rightWorkerTime)
			{
				// queue left worker to open valve
				auto newState{ StateTwoWorkers{ StateTwoWorkers{ time, totalReleased, static_cast<bt>(openedValves | valve), static_cast<bt2>(valve) | (static_cast<bt2>(leftWorkerTime) << 16), static_cast<bt2>(rightbits(availableWorkers)) } } };
				stateStack.push(newState);
			}
			else
			{
				// queue right worker to open valve
				auto newState{ StateTwoWorkers{ StateTwoWorkers{ time, totalReleased, static_cast<bt>(openedValves | valve), static_cast<bt2>(valve) | (static_cast<bt2>(rightWorkerTime) << 16), static_cast<bt2>(leftbits(availableWorkers)) } } };
				stateStack.push(newState);
			}
		}
		else // A single worker is available
		{
			// Check if it'd be quicker for the worker in the queue to open both the queued valve and this new valve
			auto interValveDist{ v::distances[ queueValve() | valve ] };
			auto queuerOpensBothTime{ queueTime() - interValveDist };
			if (queuerOpensBothTime > rightWorkerTime)
			{
				return;
			}

			if (rightWorkerTime > queueTime())
			{
				// New state at rightWorkerTime with valve opened, queue copied to new state
				auto newState{ StateTwoWorkers{ rightWorkerTime, totalReleased + (v::flows[valve] * rightWorkerTime), static_cast<bt>(openedValves | valve), queue, static_cast<bt2>(valve) } };
				stateStack.push(newState);
			}
			else if (rightWorkerTime < queueTime())
			{
				// New state at queueTime(), valve & valve open time becomes new queue
				auto newState{ StateTwoWorkers{ queueTime(), totalReleased + queueRelease(), static_cast<bt>(openedValves | valve), (static_cast<bt2>(rightWorkerTime) << 16) | valve, queueValve() } };
				stateStack.push(newState);
			}
			else if (rightWorkerTime)
			{
				// rightWorkerTime == queue time, open both valves with none in queue
				auto newState{ StateTwoWorkers{ rightWorkerTime, totalReleased + (v::flows[valve] * rightWorkerTime) + queueRelease(), static_cast<bt>(openedValves | valve), 0, (static_cast<bt2>(valve) << 16) | static_cast<bt2>(queueValve()) } };
				stateStack.push(newState);
			}
		}
	}

	int depthFirst()
	{
		int best{ 0 };
		stack_t stack;
		stack.push(*this);

		long long unsigned int count{ 0 }; // For checking how many times we go through this while loop

		while (!stack.empty())
		{
			++count;
			auto top{ stack.top() };
			if (top.totalReleased + top.queueRelease() > best) best = top.totalReleased + top.queueRelease();
			
			stack.pop();

			if (top.potentialRelease < best)
			{
				continue;
			}

			for (bt i{ 1 }; i !=0; i <<= 1)
			{
				if (i & ~(top.openedValves))
				{
					top.openValve(i, stack);
				}
			}
		}

		DL("iterations: " << count);
		return best;
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

		while (inf)
		{
			std::string line;
			std::getline(inf, line);
			if (line.length())
			{
				AllValves::push(line);
			}
		}

		AllValves::compress();

		auto s{ State{} };
		utils::printAnswer("total possible pressure release in 30 seconds: ", State::openAll(s));
	}

};

const int StateTwoWorkers::totalTime{ 26 };

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		auto st2{ StateTwoWorkers{} };

		utils::printAnswer("total possible pressure release in 26 seconds with an elephant's help: ", st2.depthFirst());
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
