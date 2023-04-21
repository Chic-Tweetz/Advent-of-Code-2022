#pragma once

#include <iostream>

#include "utils.h" // Allows enabling with -d flag

// Debug macros - DOUT, DLN can be toggled on or off, DERR prints in RED text!
# define DERR(x) std::cerr << "\033[31m" << x << "\033[0m";
	
namespace debug
{
	bool bPrintEnabled{ true }; // Toggle DOUT when needed
}

# define DOUT if(flags::isSet(flags::Flag::debug)) std::cout
# define DP(x) if (flags::isSet(flags::Flag::debug) && debug::bPrintEnabled) { std::cout << x; }
# define DL(x) if (flags::isSet(flags::Flag::debug) && debug::bPrintEnabled) { std::cout << x << '\n'; }

# define DTOGGLE debug::bPrintEnabled = !debug::bPrintEnabled;
# define DENABLE debug::bPrintEnabled = true;
# define DDISABLE debug::bPrintEnabled = false;

// Moved these a set of consts in utils, namespace style
// # define DSTYLE "\033["
// # define DRESET "\033[0m"
// # define DBLACK "\033[30m"
// # define DRED "\033[31m"
// # define DGREEN "\033[32m"
// # define DYELLOW "\033[33m"
// # define DBLUE "\033[34m"
// # define DMAGENTA "\033[35m"         
// # define DCYAN "\033[36m"         
// # define DWHITE "\033[37m" 
// # define DBGBLACK "\033[40m"
// # define DBGRED "\033[41m"
// # define DBGGREEN "\033[42m"
// # define DBGYELLOW "\033[43m"
// # define DBGBLUE "\033[44m"
// # define DBGMAGENTA "\033[45m"
// # define DBGCYAN "\033[46m"
// # define DBGWHITE "\033[47m"
// # define DBOLD "\033[1m"
// # define DUNDERLINE "\033[4m"
// # define DINVERSE "\033[7m"
// # define DBOLDOFF "\033[21m"
// # define DUNDERLINEOFF "\033[24m"
// # define DINVERSEOFF "\033[27m"
