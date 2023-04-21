// --- Day 10: Cathode-Ray Tube ---

#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "debug.h"
#include "utils.h"

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		int cycle{ 0 };
		int x{ 1 };
		int addnext{ 0 };
		int addqueue{ 0 };
		int signal{ 0 };

		while (++cycle < 221)
		{

			// Cycle Start
			addnext = addqueue;

			// If not already executing an instruction
			if (!addqueue)
			{
				std::string instr;
				if (inf)
				{
					std::getline(inf, instr);
				}

				if (instr.length() && instr[3] == 'x')
				{
					addqueue = std::atoi(instr.data() + 4);
				}
			}
			else
			{
				addqueue = 0;
			}

			// We want to get data every 40 cycles from 20 to 220
			if ((cycle - 20) % 40 == 0)
			{
				signal += cycle * x;
			}

			// Cycle End
			x += addnext;
			
		}

		utils::printAnswer("signal strength: ", signal);

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

		int cycle{ 0 };
		int x{ 1 };
		int addnext{ 0 };
		int addqueue{ 0 };
		
		std::string answerStr;

		while (++cycle <= 240)
		{
			int pixel{ ((cycle - 1) % 40 ) }; // 0 to 39

			if (std::abs(pixel - x) < 2)
			{
				answerStr += '#';
				DOUT << '#';
			}
			else
			{
				answerStr += ' ';
				DOUT << ' ';
			}

			if (pixel == 39)
			{
				answerStr += '\n';
				DOUT << '\n';
			}

			// Cycle Start
			addnext = addqueue;

			// If not already executing an instruction
			if (!addqueue)
			{
				std::string instr;
				if (inf)
				{
					std::getline(inf, instr);
				}

				if (instr.length() && instr[3] == 'x')
				{
					addqueue = std::atoi(instr.data() + 4);
				}
			}
			else
			{
				addqueue = 0;
			}

			// Cycle End
			x += addnext;
		}
		
		answerStr += '\n';
		DOUT << '\n';
		DOUT << answerStr;

		utils::printAnswer("\nhidden letters:\n\n", answerStr, "");
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
