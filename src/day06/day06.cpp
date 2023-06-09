// --- Day 6: Tuning Trouble ---

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "utils.h"

// Really kinda silly but I had fun with it
class CharsCircular
{
public:
	CharsCircular(size_t num = 4) : size{ num }
	{
		Node** currptr{ &curr };
		for(size_t i{ 0 }; i < size; ++i)
		{
			*currptr = new Node;
			currptr = &((*currptr)->next);
		}
		*currptr = curr; // Final element should be pointing back at the first
	}

	CharsCircular(std::string_view chars) : size{ chars.length() }
	{
		Node** currptr{ &curr };
		for(size_t i{ 0 }; i < size; ++i)
		{
			*currptr = new Node(chars[i]);
			currptr = &((*currptr)->next);
		}
		*currptr = curr; // Final element should be pointing back at the first 
	}

	~CharsCircular()
	{
		Node* start{ curr };
		do 
		{
			Node* temp{ curr };
			curr = curr->next;
			delete temp;
		} while(curr != start);
	}

	void replace(char ch)
	{
		curr->ch = ch;
		curr = curr->next;
	}

	void insert(char ch)
	{
		curr->next = new Node(ch, curr->next);
	}

	void print()
	{
		Node* start{ curr };
		
		do
		{
			std::cout << curr->ch;
			curr = curr->next;
		} while (curr != start);

		std::cout << '\n';
	}

	bool unique()
	{
		// compare each char with the other chars
		Node* trav{ curr->next };
		Node* comp{ curr };

		do
		{
			while (trav != curr)
			{
				// std::cout << "comp: " << comp->ch << " with: " << trav->ch << '\n';
				if (comp->ch == trav->ch)
				{
					// std::cout << "duplicates\n";
					return false;
				}

				trav = trav->next;
			}

			comp = comp->next;
			trav = comp->next;

		} while (comp != curr);
		
		// std::cout << "unique\n";
		return true;
	}   

private:
	struct Node
	{
		Node() {}
		Node(char _ch, Node* _next = nullptr) : ch{ _ch }, next{ _next } {}
		char ch{ '0' };
		Node* next;
	};

	Node* curr;
	size_t size{ 4 };
};

size_t unique(const std::string &instr, size_t &start, const size_t count)
{
	const auto end{ start + count - 1 };

	if(end >= instr.length())
	{
	// We've searched the whole input without finding a unique group
	// (this shouldn't actually happen)
		return 0;
	}

	// First comparison char less than, second less than equal to end
	// ie, the last comparison is the 2nd to last and last char
	for (auto left{ start }; left < end; ++left)
	{
		// Second comparison char starts at the first + 1
		// ie char [0] and [1] is the first comparison
		for (auto right{ left + 1 }; right <= end; ++right)
		{
			// std::cout << "comparing: " << instr[i] << " and: " << instr[j] << "\n";
			if (instr[left] == instr[right])
			{
				// we can skip ahead when detecting duplicates
				// which is why we've passed start by ref
				start = left + 1; 
				return 0;
			}
		}
	}

	// Off by one...
	// std::cout << "unique string found at " << end + 1 << '\n';

	return end + 1;
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

		std::string instr;
		std::getline(inf, instr);

		const size_t sequenceLength{ 4 };
		
		size_t iter{ 0 };
		while (iter < instr.length())
		{
			if (auto answer{ unique(instr, iter, sequenceLength) }; answer)
			{
				utils::printAnswer("unique string found at: ", answer);
				return;
			}
		}

		std::cout << "not found\n";
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

		std::string instr;
		std::getline(inf, instr);

		const size_t sequenceLength{ 14 };
		
		size_t iter{ 0 };
		while (iter < instr.length())
		{
			if (auto answer{ unique(instr, iter, sequenceLength) }; answer)
			{
				utils::printAnswer("unique string found at: ", answer);
				return;
			}
		}

		std::cout << "not found\n";
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
