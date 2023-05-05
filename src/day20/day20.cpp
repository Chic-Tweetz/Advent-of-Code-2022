// --- Day 20: Grove Positioning System ---

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "timer.h"
#include "utils.h"

// Doubly-linked - you could go single and convert negative moves to +ve though
template<typename T>
struct Node
{
	Node(T _data, Node* _prev = nullptr, Node* _next = nullptr) :
		data{ _data }, 
		prev{ _prev },
		next{ _next }
	{	
	}

	T data;
	Node *prev;
	Node *next;

	void remove()
	{
		prev->next = next;
		next->prev = prev;
	}

	// Remove node from list, travel to its new position and insert
	void move(T amount)
	{		
		if (amount > 0)
		{
			Node *trav{ this };
			remove();

			while (--amount >= 0)
			{
				trav = trav->next;
			}

			trav->next->prev = this;
			next = trav->next;
			trav->next = this;
			prev = trav;
		} 
		// Reversing is exactly the same but prevs are flipped to nexts and nexts to prevs
		else if (amount < 0)
		{
			Node *trav{ this };
			remove();

			while (++amount <= 0)
			{
				trav = trav->prev;
			}

			trav->prev->next = this;
			prev = trav->prev;
			trav->prev = this;
			next = trav;
		}
	}

	inline static Node** getNext(Node* node)
	{
		return &(node->next);
	}

	inline static Node** getPrev(Node* node)
	{
		return &(node->prev);
	}

	// What's a linked-list without pointers to pointers anyway
	// Same as move except next / prev are replaced by fwd / rev
	// Which get &next / &prev depending on positive / negative move

	// Although more fun, this takes about twice as long as move()
	void move2(T amount)
	{		
		if (amount == 0) return;

		Node** (*fwd)(Node*){ amount > 0 ? getNext : getPrev };
		Node** (*rev)(Node*){ amount > 0 ? getPrev : getNext };

		amount = std::abs(amount);

		Node *trav{ this };

		remove();

		while (--amount >= 0)
		{
			trav = *fwd(trav);
		}

		// Equivalents for *fwd(this) == this->next in comments because it's hard to understand otherwise:
		//                 *rev(this) == this->prev

		// trav->next->prev = this;
		*rev(*fwd(trav)) = this;
		// next = trav->next;
		*fwd(this) = *fwd(trav);
		// trav->next = this;
		*fwd(trav) = this;
		// prev = trav;
		*rev(this) = trav;

	}
};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		auto inputLines{ utils::bufferLines(infile) };

		std::vector<Node<int>*> nodeOrder;
		nodeOrder.reserve(inputLines.size());

		nodeOrder.push_back(new Node<int>{ std::stoi(inputLines[0]), 0 });
		
		Node<int> *zero{ nodeOrder[0]->data == 0 ? nodeOrder[0] : nullptr };

		for (size_t i{ 1 }; i < inputLines.size(); ++i)
		{
			// auto newNode{ new Node{ std::stoi(inputLines[i]) } };
			auto newNode{ new Node{ std::stoi(inputLines[i]), nodeOrder[i - 1] } };
			nodeOrder[i - 1]->next = newNode;
			nodeOrder.push_back(newNode);

			if (newNode->data == 0)
			{
				zero = newNode;
				DOUT << "got zero\n";
			}

		}

		nodeOrder[0]->prev = nodeOrder[nodeOrder.size() - 1];
		nodeOrder[nodeOrder.size() - 1]->next = nodeOrder[0];

		for (auto *node : nodeOrder)
		{
			// Modulo the move by the size ( - 1 ) due to massive numbers
			// Subtract 1 because the moving node itself is taken out of the list for the move
			node->move2(node->data % TOI(nodeOrder.size() - 1));
		}

		int i{ 0 };
		auto curr{ zero };
		int sum{ 0 };
		while(i++ < 3000)
		{
			curr = curr->next;
			if (i == 1000 || i == 2000 || i == 3000)
			{
				sum += curr->data;
			}
		}

		utils::printAnswer("sum of grove coordinates: ", sum);

	}

};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		// Timer timer{};

		auto inputLines{ utils::bufferLines(infile) };

		std::vector<Node<long long>*> nodeOrder;
		nodeOrder.reserve(inputLines.size());

		const long key{ 811589153 };

		nodeOrder.push_back(new Node<long long>{ std::stoll(inputLines[0]) * key, 0 });
		
		;
		Node<long long> *zero{ nodeOrder[0]->data == 0 ? nodeOrder[0] : nullptr };

		for (size_t i{ 1 }; i < inputLines.size(); ++i)
		{
			// auto newNode{ new Node{ std::stoi(inputLines[i]) } };
			auto newNode{ new Node{ std::stoll(inputLines[i]) * key, nodeOrder[i - 1] } };
			nodeOrder[i - 1]->next = newNode;
			nodeOrder.push_back(newNode);

			if (newNode->data == 0)
			{
				zero = newNode;
			}

		}

		// Make start & end circular
		nodeOrder[0]->prev = nodeOrder[nodeOrder.size() - 1];
		nodeOrder[nodeOrder.size() - 1]->next = nodeOrder[0];

		int count{ 10 };

		// timer.reset();

		while (--count >= 0)
		{
			for (auto *node : nodeOrder)
			{
				node->move((node->data) % TOI(nodeOrder.size() - 1));
			}
		}

		// DL(timer.elapsed() << " elapsed");

		int i{ 0 };
		auto trav{ zero };
		long long sum{ 0 };
		while(i++ < 3000)
		{
			trav = trav->next;
			if (i == 1000 || i == 2000 || i == 3000)
			{
				sum += trav->data;
			}
		}

		utils::printAnswer("sum of 10x mixed grove coordinates * key: ", sum);
	}
};

int main(int argc, char* argv[])
{
	flags::set(argc, argv);

	const std::string input{ utils::inputFile(__FILE__) };

	DL("input: " << input); 

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
