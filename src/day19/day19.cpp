// --- Day 19: Not Enough Minerals ---

#include <array>
#include <cmath>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "utils.h"
#include "timer.h"

const std::array<int, 35> triangularNumbers { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120,136, 153, 171, 190, 210, 231, 253, 276, 300, 325, 351, 378, 406, 435, 465, 496, 528, 561, 595 };

// Indices for each resource - used like an enum but I don't have to cast anything this way
namespace Resources
{
	const inline size_t begin{ 0 };
	const inline size_t ore{ 0 };
	const inline size_t clay{ 1 };
	const inline size_t obsidian{ 2 };
	const inline size_t geode{ 3 };
	const inline size_t end{ 4 };
};

struct Blueprint
{
	// Char searches and offsets to important numbers from input file
	Blueprint(std::string_view s) :
		id{ std::atoi(s.data() + 10) },
		oreCosts{ {
			std::atoi(s.data() + s.find('E', 13) + 21),
			std::atoi(s.data() + s.find('c', 46) + 17),
			std::atoi(s.data() + s.find('d', 79) + 17),
			std::atoi(s.data() + s.find('g', 119) + 18)
		 } },
		obsidianClay { std::atoi(s.data() + s.find('n', 103) + 3) },
		geodeObsidian { std::atoi(s.data() + s.find_last_of('r', s.length() - 10) + 7) },

		maxBots{ std::max(std::max(oreCosts[1], oreCosts[2]), oreCosts[3]), obsidianClay, geodeObsidian, INT_MAX }, 
		lastBuildTimes{ { oreCosts[0] + 4, 5, 3, 1 } } 
	{
		for (size_t i{ 0 }; i < triangularGeodeObsCosts.size(); ++i)
		{
			triangularGeodeObsCosts[i] = TOI(i) * geodeObsidian;
			triangularObsidianClayCosts[i] = TOI(i) * obsidianClay;
		}
	}

	const int id;
	const std::array<int, 4> oreCosts; // Array of ore costs lining up with Resources::resource index

	const int obsidianClay;
	const int geodeObsidian;

	const std::array<int, 4> maxBots; // high cost of each resource, eg if 3 ore is the highest cost, you never need more than 3 orebots
	const std::array<int, 4> lastBuildTimes; // The last minute building each robot could still help get a geode

	std::array<int, 35> triangularGeodeObsCosts;
	std::array<int, 35> triangularObsidianClayCosts;
};

class Choosey
{
public:
	Choosey(const Blueprint &blueprint, bool bAuto = false, int startTime = 24) : time{ startTime }, bp{ blueprint } { fastFwd(); if (!bAuto) { prompt(); } }
	
	// New states constructed from the previous state + the robot to build next
	Choosey(const Choosey &prev, size_t newBot) : time{ prev.time }, bp{ prev.bp }
	{
		for (auto i{ Resources::begin }; i < Resources::end; ++i)
		{
			res[i] = prev.res[i];
			bots[i] = prev.bots[i];
			whenBuild[i] = prev.whenBuild[i];
			canBuild[i] = prev.canBuild[i];
		}
		buildInFuture(newBot);
	}
private:
	int time;
	std::array<int, 4> bots{ 1, 0, 0, 0 };
	std::array<int, 4> res{ 0, 0, 0, 0 };
	std::array<bool, 4> canBuild{ false, false, false, false };
	std::array<int, 4> whenBuild{ 0, 0, 0, 0 };
	const Blueprint &bp;

	std::deque<char> multicmds{}; // For cmd line input, not used for puzzle

	// This is how we cull - if you build 1 of each bot each turn, how many geodes would that get you
	// With some clay / obsidian / geode checking to bring the number down
	// You can be more aggressive here - you can only build 1 bot per minute after all
	int maxGeodes()
	{
		// If you built a claybot every minute, how much clay could you gather?
		auto actualClay{ res[Resources::clay] + bots[Resources::clay] * time };

		// The last useful claybot you can build is at minute 5
		// Its clay becomes available to build an obsidianbot at minute 3
		// Giving just enough time for one extra geodebot to produce 1 extra geode

		// Therefore we can reduce the total clay output by only builing claybots till minute 5

		int clayBuildMinutes{ std::max(0, 1 + time - bp.lastBuildTimes[Resources::clay]) };
		auto theoreticalClay{ actualClay + triangularNumbers[ST(clayBuildMinutes)] 
	         + clayBuildMinutes * bp.lastBuildTimes[Resources::clay] };

		auto actualObs{ res[Resources::obsidian] + bots[Resources::obsidian] * time };

		// We can do the same for that minute 3 obsidibot

		int obsiBuildMinutes{ std::max(0, time - bp.lastBuildTimes[Resources::obsidian]) };
		size_t triangularIndex{ ST(obsiBuildMinutes) };

		while (bp.triangularObsidianClayCosts[triangularIndex] > theoreticalClay)
		{	
			--triangularIndex;
		}

		auto maxObs = actualObs + triangularNumbers[triangularIndex]
		     + obsiBuildMinutes * bp.lastBuildTimes[Resources::obsidian];

		auto actualGeodes{ res[Resources::geode] + bots[Resources::geode] * time };

		triangularIndex = ST(time);

		while (bp.triangularGeodeObsCosts[triangularIndex] > maxObs)
		{
			--triangularIndex;
		}
		auto theoreticalGeodes{ triangularNumbers[triangularIndex] };

		return actualGeodes + theoreticalGeodes;
	}

	// Number of geodes you'd gather if building a geodebot every minute until the end
	int maxGeodesSimple()
	{
		auto actual{ res[Resources::geode] + bots[Resources::geode] * time };
		return actual + triangularNumbers[ST(time)];
	}

	// Gives actual number of geodes that would be gathered by minute 0
	int projectedGeodes()
	{
		return res[Resources::geode] + bots[Resources::geode] * time;
	}

	// Print current resource / bot counts in a neatly spaced grid
	void printResAndBots() const
	{
		//     ore  clay obsi geodes
		//     1234 1234 1234 1234
		//     1234 1234 1234 1234
		size_t separation{ 6 };
		std::array<std::string, 4> resStrs{ { "     ", "     ", "     ", "     " } };
		std::array<std::string, 4> botStrs{ { "     ", "     ", "     ", "     " } };
		std::array<std::string, 4> whenStrs{ { "     ", "     ", "     ", "     " } };
		for (size_t i{ Resources::begin }; i < Resources::geode; ++i)
		{
			resStrs[i] = std::to_string(res[i]);
			while (resStrs[i].length() < separation)
			{
				resStrs[i] += " ";
			}

			botStrs[i] = std::to_string(bots[i]);
			while (botStrs[i].length() < separation)
			{
				botStrs[i] += " ";
			}

			whenStrs[i] = whenBuild[i] ? std::to_string(25 - whenBuild[i]) : "-";
			while (whenStrs[i].length() < separation)
			{
				whenStrs[i] += " ";
			}
		}

		resStrs[Resources::geode] = std::to_string(res[Resources::geode]);
		botStrs[Resources::geode] = std::to_string(bots[Resources::geode]);
		whenStrs[Resources::geode] = whenBuild[Resources::geode] ? std::to_string(25 - whenBuild[Resources::geode]) : "-";

		std::cout << "ore   clay  obsi  geodes\n";
		
		for (size_t i{ Resources::begin }; i < Resources::end; ++i)
		{
			std::cout << resStrs[i];
		}
		std::cout << '\n';
		for (size_t i{ Resources::begin }; i < Resources::end; ++i)
		{
			std::cout << botStrs[i];
		}
		std::cout << '\n';
		for (size_t i{ Resources::begin }; i < Resources::end; ++i)
		{
			std::cout << whenStrs[i];
		}
		std::cout << '\n';
	}

	// Cmd line input to choose which bots to build
	// For fun mainly, but also to make sure everything works as it should
	void prompt()
	{
		std::cout << "== minute " << 25 - time << " ==\n";
		printResAndBots();
		if(canBuild[0])
		{
			std::cout << "build orebot: " << 1 << '\n';
		} 
		if(canBuild[1])
		{
			std::cout << "build claybot:" << 2 << '\n';
		} 
		if(canBuild[2])
		{
			std::cout << "build obsidibot: " << 3 << '\n';
		}
		if(canBuild[3])
		{
			std::cout << "build geodebot: " << 4 << '\n'; 
		}
		std::cout << "build nothing: " << 0 << '\n';

		while (!multicmds.size())
		{
			std::string in;
			std::cin >> in;
			std::for_each(in.begin(), in.end(),
			[this](auto ch){
				this->multicmds.push_back(ch);
			});
			in = "";
		}
		bool bMoved{ false };
		do 
		{
			bMoved = choice();
			updateCanBuilds();
			updateWhenBuilds();
		} while(!bMoved && multicmds.size());

		if (time > 0)
		{
			prompt();
		}
	}

	// Cmd prompt choices for manually checking everything's working right
	bool choice()
	{
		char cmd{ multicmds.front() };
		multicmds.pop_front();

		switch(cmd)
		{
			case '0': forward(); fastFwd(); return true;
			case '1': if (canBuild[Resources::ore]) { buildOrebot(); return true; } else { return false; }
			case '2': if (canBuild[Resources::clay]) { buildClaybot(); return true; } else { return false; }
			case '3': if (canBuild[Resources::obsidian]) { buildObsibot(); return true; } else { return false; }
			case '4': if (canBuild[Resources::geode]) { buildGeodebot(); return true; } else { return false; }

			case 'q': if (whenBuild[Resources::ore]) { buildInFuture(Resources::ore); return true; } else { return false; }
			case 'w': if (whenBuild[Resources::clay]) { buildInFuture(Resources::clay); return true; } else { return false; }
			case 'e': if (whenBuild[Resources::obsidian]) { buildInFuture(Resources::obsidian); return true; } else { return false; }
			case 'r': if (whenBuild[Resources::geode]) { buildInFuture(Resources::geode); return true; } else { return false; }

			default: return false;
		}
	}
	
	// Move one minute forwards, adding 1 resource for every robot
	void forward()
	{
		for (size_t i{ Resources::begin }; i < Resources::end; ++i)
		{
			res[i] += bots[i];
		}
		if (--time <= 0)
		{
			return;
		}
	}

	// Move time to where resources for building bot have been gathered, build it
	void buildInFuture(size_t resource)
	{
		// Add all resources gathered in the interim
		for (size_t i{ Resources::begin }; i < Resources::end; ++i)
		{
			res[i] += (1 + (time - whenBuild[resource])) * bots[i];
		}
		
		// Subtract an extra minute to account for build time
		time = whenBuild[resource] - 1;

		// Subtract ore cost
		res[Resources::ore] -= bp.oreCosts[resource];

		// Subtract extra resource cost for obsidibots or geodebots
		if (resource == Resources::obsidian)
		{
			res[Resources::clay] -= bp.obsidianClay;
		}
		else if (resource == Resources::geode)
		{
			res[Resources::obsidian] -= bp.geodeObsidian;
		}

		++bots[resource];

		updateCanBuilds();
		updateWhenBuilds();
		fastFwd();
	}

	// CanBuilds aren't used for the acutal puzzle
	void updateCanBuilds()
	{
		canBuild[0] = canBuildOrebot();
		canBuild[1] = canBuildClaybot();
		canBuild[2] = canBuildObsibot();
		canBuild[3] = canBuildGeodebot();
	}

	// WhenBuilds give the minute each bot could be constructed with current resources / bots
	// 0 = either never or too late to make a difference
	void updateWhenBuilds()
	{
		whenBuild[0] = willBeAbleToBuildOrebot();
		whenBuild[1] = willBeAbleToBuildClaybot();
		whenBuild[2] = willBeAbleToBuildObsidibot();
		whenBuild[3] = willBeAbleToBuildGeodebot();
	}

	void fastFwd()
	{
		while (!canBuild[0] && !canBuild[1] && !canBuild[2] && !canBuild[3] && time > 0)
		{
			// DOUT << "== minute " << 25 - time << " ==\n";
			forward();
			updateCanBuilds();
			updateWhenBuilds();
		}
	}

	// There are a lot of similar methods because I went one by one making sure everything was working
	// Could simplifiy now
	void buildOrebot()
	{
		forward();
		++bots[Resources::ore];
		res[Resources::ore] -= bp.oreCosts[Resources::ore];
		updateCanBuilds();
		updateWhenBuilds();
		fastFwd();
	}

	void buildClaybot()
	{
		forward();
		++bots[Resources::clay];
		res[Resources::ore] -= bp.oreCosts[Resources::clay];
		updateCanBuilds();
		updateWhenBuilds();
		fastFwd();
	}

	void buildObsibot()
	{
		forward();
		++bots[Resources::obsidian];
		res[Resources::ore] -= bp.oreCosts[Resources::obsidian];
		res[Resources::clay] -= bp.obsidianClay;
		updateCanBuilds();
		updateWhenBuilds();
		fastFwd();
	}
	
	void buildGeodebot()
	{
		forward();
		++bots[Resources::geode];
		res[Resources::ore] -= bp.oreCosts[Resources::geode];
		res[Resources::obsidian] -= bp.geodeObsidian;
		updateCanBuilds();
		updateWhenBuilds();
		fastFwd();
	}

	bool canBuildOrebot()
	{
		return ((bots[Resources::ore] < bp.maxBots[Resources::ore])
		        && (bp.oreCosts[Resources::ore] <= res[Resources::ore])
				&& (time >= bp.lastBuildTimes[Resources::ore]));
	}

	bool canBuildClaybot()
	{
		return ((bots[Resources::clay] < bp.maxBots[Resources::clay])
		        && (bp.oreCosts[Resources::clay] <= res[Resources::ore])
				&& (time >= bp.lastBuildTimes[Resources::clay]));
	}

	bool canBuildObsibot()
	{
		return ((bots[Resources::obsidian] < bp.maxBots[Resources::obsidian]) && (bp.oreCosts[Resources::obsidian] <= res[Resources::ore])
		        && (bp.obsidianClay <= res[Resources::clay])
				&& (time >= bp.lastBuildTimes[Resources::obsidian]));
	}
	
	bool canBuildGeodebot()
	{
		return ((bp.oreCosts[Resources::obsidian] <= res[Resources::ore])
		        && (bp.geodeObsidian <= res[Resources::obsidian]));
	}

	int willBeAbleToBuildOrebot()
	{
		auto oreDefecit{ bp.oreCosts[Resources::ore] - res[Resources::ore] };

		if (oreDefecit <= 0) return time;

		auto timeToGatherOre{ oreDefecit / bots[Resources::ore] + ((oreDefecit % bots[Resources::ore]) ? 1 : 0) };

		auto buildTime{ time - timeToGatherOre };

		if (buildTime >= bp.lastBuildTimes[Resources::ore])
		{
			return buildTime;
		}
		else
		{
			return 0;
		}
	}

	int willBeAbleToBuildClaybot()
	{
		auto oreDefecit{ bp.oreCosts[Resources::clay] - res[Resources::ore] };

		if (oreDefecit <= 0) return time;

		auto timeToGatherOre{ oreDefecit / bots[Resources::ore] + ((oreDefecit % bots[Resources::ore]) ? 1 : 0) };

		auto buildTime{ time - timeToGatherOre };

		if (buildTime >= bp.lastBuildTimes[Resources::clay])
		{
			return buildTime;
		}
		else
		{
			return 0;
		}
	}

	int willBeAbleToBuildObsidibot()
	{
		if (!bots[Resources::clay]) return 0;

		auto oreDefecit{ bp.oreCosts[Resources::obsidian] - res[Resources::ore] };
		auto clayDefecit{ bp.obsidianClay - res[Resources::clay] };

		if (clayDefecit <= 0 && oreDefecit <= 0) return time;

		auto timeToGatherClay{ clayDefecit ?
			(clayDefecit / bots[Resources::clay] + ((clayDefecit % bots[Resources::clay]) ? 1 : 0))
			: 0};
		auto timeToGatherOre{ oreDefecit ?
			(oreDefecit / bots[Resources::ore] + ((oreDefecit % bots[Resources::ore]) ? 1 : 0))
			: 0};

		auto buildTime{ time - std::max(timeToGatherClay, timeToGatherOre) };

		if (buildTime >= bp.lastBuildTimes[Resources::obsidian])
		{
			return buildTime;
		}
		else
		{
			return 0;
		}
	}

	int willBeAbleToBuildGeodebot()
	{
		if (!bots[Resources::obsidian]) return 0;

		auto oreDefecit{ bp.oreCosts[Resources::geode] - res[Resources::ore] };
		auto obsiDefecit{ bp.geodeObsidian - res[Resources::obsidian] };

		if (obsiDefecit <= 0 && oreDefecit <= 0) return time;

		auto timeToGatherObsi{ obsiDefecit ?
			(obsiDefecit / bots[Resources::obsidian] + ((obsiDefecit % bots[Resources::obsidian]) ? 1 : 0))
			: 0};
		auto timeToGatherOre{ oreDefecit ?
			(oreDefecit / bots[Resources::ore] + ((oreDefecit % bots[Resources::ore]) ? 1 : 0))
			: 0};

		auto buildTime{ time - std::max(timeToGatherObsi, timeToGatherOre) };

		if (buildTime >= bp.lastBuildTimes[Resources::geode])
		{
			return buildTime;
		}
		else
		{
			return 0;
		}
	}

public:
	int breadthFirst()
	{
		updateWhenBuilds();
		std::deque<Choosey> queue;

		queue.push_back(*this);

		int iter{ 0 };   // For checking how well our culling is going
		int culled{ 0 };

		int mostGeodes{ 0 };
		while (!queue.empty())
		{
			++iter;
			auto top{ queue.front() };
			queue.pop_front();

			for (auto i{ Resources::begin }; i < Resources::end; ++i)
			{
				if (top.whenBuild[i] > 0)
				{
					Choosey nextUp{ top, i };
					if (nextUp.projectedGeodes() > mostGeodes)
					{
						mostGeodes = nextUp.projectedGeodes();
						queue.push_back(nextUp);
					}
					else if (nextUp.maxGeodes() > mostGeodes)
					{
						queue.push_back(nextUp);
					}
					else
					{
						++culled;
					}
				}
			}
		}

		DOUT << style::blue << "== BP" << bp.id << " ==\n" << style::reset;
		DOUT << "most geodes: " << mostGeodes << '\n';
		DOUT << "iters: " << iter << '\n';
		DOUT << "culled: " << culled << '\n';
		DOUT << "score: " << bp.id * mostGeodes << '\n';

		return mostGeodes;
	}
};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		Timer t;

		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string blueprint;
		std::getline(inf, blueprint);

		int scoreSum{ 0 };
		
		t.reset();

		while (blueprint.length())
		{
			Blueprint bp{ blueprint };
			Choosey cmds{ bp, true };

			scoreSum += cmds.breadthFirst() * bp.id; // breadth by 0.1 seconds

			std::getline(inf, blueprint);
		}		

		DOUT << "elapsed: " << t.elapsed() << '\n';

		utils::printAnswer("sum of blueprint scores: ", scoreSum);

	}

};

// 3 minutes 23 seconds to beat
// 1.75 seconds! Consider it beaten
// Curiously, test input is much slower (14s last tested)
namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		Timer t;

		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string blueprint;
		std::getline(inf, blueprint);

		int count{ flags::t() ? 2 : 3 };
		int scoreSum{ 1 };

		t.reset();

		while (--count >= 0)
		{
			Blueprint bp{ blueprint };
			Choosey cmds{ bp, true, 32 };

			scoreSum *= cmds.breadthFirst(); // 1.75117, 1.75187, 1.7477 
			std::getline(inf, blueprint);
		}		

		DOUT << "elapsed: " << t.elapsed() << '\n';

		utils::printAnswer("product of first three blueprints' highest geode yield: ", scoreSum);

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
