// Copy and paste for each day for a quick start

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
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

		utils::printAnswer();

	}

};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		utils::printAnswer();

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
