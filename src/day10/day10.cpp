// Copy and paste for each day for a quick start

#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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

		std::cout << "Signal strength: " << signal << "\n\n";

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

		while (++cycle <= 240)
		{
			int pixel{ ((cycle - 1) % 40 ) }; // 0 to 39

			// std::cout << pixel % 10;

			if (std::abs(pixel - x) < 2)
			{
				std::cout << '#';
			}
			else
			{
				std::cout << ' ';
			}

			if (pixel == 39)
			{
				std::cout << '\n';
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
		
		std::cout << '\n';
		
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__, "input") };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}