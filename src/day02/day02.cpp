#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

#include "utils.h"

class Rps
{
public:
enum class Choice
{
    errChoice   = 0,
    rock        = 1,
    paper       = 2,
    scissors    = 3,
    endChoice
};

using enum Choice;

static const int oppCharSub{ 'A' - 1 };
static const int youCharSub{ 'X' - 1 };

static const int win{ 6 };
static const int draw{ 3 };
static const int loss{ 0 };

static constexpr int charOutcome (char outcome)
{
    switch (outcome)
    {
    case 'Z': return win;
    case 'Y': return draw;
    default: return loss;
    }
}
static constexpr Choice charThem (char outcome)
{
    switch (outcome)
    {
    case 'A': return rock;
    case 'B': return paper;
    case 'C': return scissors;
    default: return errChoice;
    }
}

static constexpr std::string_view choicestr(Choice choice)
{
    switch (choice)
    {
    case rock:      return "rock";
    case paper:     return "paper";
    case scissors:  return "scissors";
    default:        return "???";  
    }
}

static Choice outcome(char them, char outcome)
{
    if(outcome < 'X' || outcome > 'Z' ||
    them < 'A' || them > 'C')
    {
        return errChoice;
    }

    return Rps::outcome(charThem(them), charOutcome(outcome));
}

static Choice outcome(Choice them, int outcome)
{
    if (static_cast<int>(them) <= static_cast<int>(errChoice) ||
        static_cast<int>(them) >= static_cast<int>(endChoice) ||
        outcome < loss || outcome > win)
    {
        return errChoice;
    }

    switch (outcome)
    {
    case win: switch(them)
        {
            case rock: return paper;
            case paper: return scissors;
            default: return rock;
        }
    case loss: switch(them)
        {
            case rock: return scissors;
            case paper: return rock;
            default: return paper;
        }
    default: return them;
    }
}

static int play(Choice them, Choice you)
{      
    if (static_cast<int>(you) <= static_cast<int>(errChoice) ||
        static_cast<int>(you) >= static_cast<int>(endChoice) ||
        static_cast<int>(them) <= static_cast<int>(errChoice) ||
        static_cast<int>(them) >= static_cast<int>(endChoice))
    {
        return 0;
    }

    int score{ static_cast<int>(you) };

    // std::cout << choicestr(you);
    if (them == you)
    {
        // std::cout << "you drew\n";
        // std::cout << " draws with "
        //     << choicestr(them)
        //     << ": +" << draw + score << '\n';
        return draw + score;
    }

    if ( ( (them == rock)     && (you == paper   ) ) ||
        ( (them == paper)    && (you == scissors) ) ||
        ( (them == scissors) && (you == rock    ) ) )
    {
        //std::cout << " beats "
            // << choicestr(them)
            // << ": +" << win + score << '\n';
        // std::cout << "you win\n";
        return win + score;
    }

    // std::cout << "you lose";
    // std::cout << " loses to "
    //         << choicestr(them)
    //         << ": +" << loss + score << '\n';
    return loss + score;

}

static int play(char themin, char youin)
{
    Choice them { static_cast<Choice>(themin - oppCharSub) };
    Choice you { static_cast<Choice>(youin - youCharSub) };

    return play(them, you);
}

static int play2(char them, char outcome)
{
    return Rps::play(Rps::charThem(them), Rps::outcome(them, outcome));
    
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

        const int oppIndex{ 0 };
        const int youIndex{ 2 };

        int yourScore{ 0 };
        while (inf)
        {
            std::string instr;
            std::getline(inf, instr);

            yourScore += Rps::play(instr[oppIndex], instr[youIndex]);
        }
        std::cout << "your score = " << yourScore << '\n';

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

        const int oppIndex{ 0 };
        const int youIndex{ 2 };

        int yourScore{ 0 };
        while (inf)
        {
            std::string instr;
            std::getline(inf, instr);

            yourScore += Rps::play2(instr[oppIndex], instr[youIndex]);
        }

        std::cout << "your score = " << yourScore << '\n';
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

    Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}

