// --- Day 22: Monkey Map ---

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "utils.h"
#include "vector2d.h"

int intlogarithm(int x, int base = 10, int logx = 0)
{
	if (x < base) return logx;
	return intlogarithm(x / base, base, logx + 1);
}

// Converts the input file to one that matches the layout we use for puzzle1
// Pass ofstream or just std::cout if you just want to print to terminal instead
template<typename T>
void outputJungleLayout(const std::string &mapFile, T &os)
{
	std::ifstream inf{ mapFile };
	if (!inf) return;
	auto buff{ utils::bufferLinesWhile(inf, [](const std::string &line) -> bool	{ return line.length();	})};
	size_t xMax{ 0 };
	for (auto &&line : buff)
	{
		xMax = std::max(xMax, line.size() + 2);
	}
	const auto largest{ std::max(xMax, buff.size() + 2) };
	const int chars{ 1 + (1 + intlogarithm(largest)) * 2}; // +2 for an extra space for a comma between x & y

	for (int i{ 0 }; i <= chars; ++i)
	{
		os << ' ';
	}
	for (size_t i{ 0 }; i < xMax; ++i)
	{
		std::string xstr{  "|" + std::to_string(i) };
		os << xstr;
		for (auto i{ xstr.size() }; i <= static_cast<size_t>(chars); ++i)
		{
			os << ' ';
		}
	}
	os << '\n';
	for (size_t y{ 0 }; y < buff.size() + 2; ++y)
	{
		std::string ystr{  std::to_string(y) };
		for (auto i{ ystr.size() }; i <= static_cast<size_t>(chars); ++i)
		{
			os << ' ';
		}
		os << ystr;
		os << '|';

		if (y < 1 || y > buff.size()) { os << '\n'; continue; }

		for (auto i{ 0 }; i < chars; ++i)
		{
			os << ' ';
		}
		
		for (size_t x{ 1 }; x <= buff[y - 1].size(); ++x)
		{
			if (buff[y - 1][x - 1] == '.')
			{
				auto coordStr{ "|" + std::to_string(x) + "," + ystr };
				os << coordStr;
				for (auto i{ coordStr.size() }; i <= static_cast<size_t>(chars); ++i)
				{
					os << ' ';
				}
			}
			else
			{
				if (buff[y - 1][x - 1] == '#')
				{
					os << '|';
				}
				else
				{
					os << ' ';
				}
				for (auto i{ 0 }; i < chars; ++i)
				{
					os << ' ';
				}
			}
		}
		os << "|\n";
	}
}

struct Location
{
	Location(size_t x, size_t y) : loc{ static_cast<int>(x), static_cast<int>(y) }, facing{ 1, 0 } {}
	Vector2d loc;
	Vector2d facing;
	bool vertical()
	{
		return facing.x == 0;
	}
};

// Although this got the right answer in the end, looking at the logs it seems at least the vertical wrapping isn't correct at the boundries between two different values

// What is going on here? Why didn't I leave helpful comments?
namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		// Could just use ints or size_t or something really
		using data_t = std::uint16_t;
		using row_t = std::vector<data_t>; // let's try 16 bits
		using jungle_t = std::vector<row_t>;
		const int yshift{ 8 };
		const int xmask{ 0b1111'1111 };

		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		// Our map contains the index of each coord within themselves
		// 8 bits represent the x index, the other 8 the y index (x right, y left bits)
		// x of y and 0 are reserved for wrapping, so 0 can be used for its falsiness, if x = 0, don't move there, get it?
		jungle_t jungleMap2;
		
		// Push extra line for top->bottom wrapping values - row[0] will contain y indices for jumps to wrap back to the bottom
		{
			std::string tempStr;
			std::getline(inf, tempStr);
			// Initialise to the correct size, and as 0s to start
			jungleMap2.push_back( row_t(tempStr.size() + 2, 0) );
			inf.seekg(0, std::ios::beg); // back to start of file
		}
		
		jungleMap2.reserve(201); // rows
		size_t strReserveSize{ 153 }; // columns
		size_t longestRow{ 0 }; // Only for saving the layout to a file 
		size_t prevRowLength{ 0 }; // For wrapping bottom->top when current line is shorter than previous line

		// Seg-faults with uint_16 which is interesting!
		// It might even only seg fault on destruction...
		std::map<unsigned int, unsigned int> tops; // y row per x col for wrapping top-bottom & bottom-top
		std::map<unsigned int, unsigned int> bottoms;

		data_t row{ 0 };
		while (inf)
		{
			++row;
			char ch;
			data_t col{ 1 };
			inf.get(ch);

			// First char is a newline after the map and before the instructions so we break there
			if (ch == '\n')
			{
				break;
			}

			// Start each row with a 0 - this will be replaced by the index of the right-wrap
			jungleMap2.push_back(row_t(1, 0)); 
			jungleMap2.back().reserve(strReserveSize);

			// Leading spaces - 0s in the map so we can just check for falsiness later
			while (ch == ' ')
			{
				jungleMap2.back().push_back(0);
				inf.get(ch);
				++col;
			}

			// Value for right->left wrap
			auto rightToLeftWrap{ ch == '#' ? 0 : col }; 
			// Ref to first left->right wrap for when we have that value
			size_t leftJumpI{ static_cast<size_t>(col - 1) };

			// Go through line char by char
			while (ch != '\n')
			{
				// tops map has the x (col) location as the key, y (row) as the value
				// Is it not possible that a cube net could stick out twice in a column though? At least this works for my input.
				if (!tops.contains(col))
				{
					tops.insert({ col, row });
				}
				bottoms[col] = row; // Unlike tops, bottoms is updated every row as we go further into the file

				// right bits = col = x coord, left bits = row = y coord, bitwise or to fit them into one number
				jungleMap2.back().push_back(ch == '#' ? 0 : (col | (row << yshift)));
				inf.get(ch);
				++col;
			}

			// End lines with the x coord of the left wrap index (which is 0 if it's blocked by a #)
			jungleMap2.back().push_back(rightToLeftWrap);

			// And the left-right wrap index can now be assigned as we've finished the row and found the right index
			jungleMap2.back()[leftJumpI] = (static_cast<int>((jungleMap2.back().size()) - 2));

			// Want the length of the row before potentially fiddling with it
			auto prevRowLengthTmp{ prevRowLength }; 
			prevRowLength = jungleMap2.back().size();

			// Where current line is shorter than previous, we need to add wrapping from bottom to top
			if (jungleMap2.back().size() < prevRowLengthTmp)
			{
				jungleMap2.back().insert(jungleMap2.back().end(), prevRowLengthTmp - jungleMap2.back().size(), 0);
			}

			// Extend previous lines for edges - doesn't actually happen for my input file
			// By edge, I mean rows which stick out further than previous ones, meaning we need to add top-bottom wrapping
			if (auto &prevLine{ jungleMap2[jungleMap2.size() - 2] }; prevLine.size() < jungleMap2.back().size())
			{
				prevLine.insert(prevLine.end(), jungleMap2.back().size() - prevLine.size(), 0);	
			}

			// It's a good bet that the next row will be the same size as the current one
			strReserveSize = jungleMap2.back().size();

			longestRow = std::max(longestRow, jungleMap2.back().size());

			jungleMap2.back().shrink_to_fit(); // As if it matters here hehe
		}

		jungleMap2.push_back(row_t( prevRowLength, 0 ) );

		// Add vertical wrapping in using bottoms and tops maps
		for (auto &&pair : tops)
		{
			// pair.first = x coord, pair.second = y coord for bottom to wrap back up to
			// bottoms[pair.first] = y coord for top to wrap down to
			bool bInaccessible{ jungleMap2[pair.second][pair.first] == 0 || jungleMap2[bottoms[pair.first]][pair.first] == 0 };

			auto bottom{ bInaccessible ? 0 : bottoms[pair.first] << yshift };
			auto top{ bInaccessible ? 0 : pair.second << yshift };

			jungleMap2[pair.second - 1][pair.first] |= bottom;
			jungleMap2[bottoms[pair.first] + 1][pair.first] |= top;
		}

		// I only made this block to save the map layouts to a file to make sure it was working properly
		if (flags::isSet('l'))
		{
			Logger saveParsedMap{ Logger::defaultDirectoryFile(utils::inputFileName() + "_pt1_parsed"), true, true };
			// auto &saveParsedMap{ std::cout };
			auto yMax{ jungleMap2.size() };
			auto xMax{ longestRow - 1 };
			auto maxInd{ std::max(xMax, yMax) };
			auto charsPerCoord{ (1 + intlogarithm(maxInd)) * 2 + 1 };

			for (int i{ 0 }; i <= charsPerCoord; ++ i)
			{
				saveParsedMap << ' ';
			}
			for (int i{ 0 }; i <= static_cast<int>(xMax); ++i)
			{
				saveParsedMap << '|' << i << ' ';
				auto numChars{ intlogarithm(i) + 1 };
				while (++numChars < charsPerCoord)
				{
					saveParsedMap << ' ';
				}
			}
			saveParsedMap << '\n';
			for (size_t y{ 0 }; y < jungleMap2.size(); ++y)
			{
				auto numChars{ intlogarithm(y) + 1 };
				while (numChars++ <= charsPerCoord)
				{
					saveParsedMap << ' ';
				}
				saveParsedMap << y << '|';
				for (size_t x{ 0 }; x < jungleMap2[y].size(); ++x)
				{
					if (!jungleMap2[y][x])
					{
						for (int i{ 0 }; i <= charsPerCoord; ++i)
						{
							saveParsedMap << ' ';
						}
						continue;
					}
					auto xDecode{ jungleMap2[y][x] & xmask };
					auto yDecode{ jungleMap2[y][x] >> 8 };

					saveParsedMap << xDecode << ',' << yDecode;

					auto numCoordChars{ intlogarithm(xDecode) + intlogarithm(yDecode) + 3 };
					while (numCoordChars++ < charsPerCoord)
					{
						saveParsedMap << ' ';
					}

					saveParsedMap << '|';
				}
				saveParsedMap << '\n';
			}
		}
		
		// Offsets of 1 to account for extra col and row we use for jumps
		size_t xStart{ 1 };
		while (jungleMap2[1][xStart - 1] == 0)
		{
			++xStart;
		}

		std::string instructions;
		std::getline(inf, instructions);

		std::string_view instruction{ instructions };
		Vector2d dir{ 1, 0 }; // Right facing to start
		Vector2d loc{ static_cast<int>(xStart), 1 };
		size_t instructionIndex{ 0 };

		while (instructionIndex < instructions.size())
		{
			// Number of moves in current direction
			auto count{ std::atoi(instruction.data() + instructionIndex) };

			while (--count >= 0)
			{
				// Indices of each coord are encoded within themselves (x = right 8 bits, y = left 8 bits)
				// Due to corners needing to contain wrapping for vertical and horizontal jumps,
				// we want to make sure we only take the x or y part depending on which direction we're moving
				// That is to say, if we're moving horizontally, keep the current y value and vice versa
				auto coord{ dir.x ?
					jungleMap2[static_cast<size_t>(loc.y)][static_cast<size_t>(loc.x + dir.x)] :
					jungleMap2[static_cast<size_t>(loc.y + dir.y)][static_cast<size_t>(loc.x)] };

				if (dir.x)
				{
					if (auto coordx{ coord & xmask })
					{
						loc.x = coordx;
					}
					else
					{
						break;
					}
				}
				else if (auto coordy{ coord >> 8 })
				{
					loc.y = coordy;
				}
				else
				{
					break;
				}
			}
		
			auto turnIndex{ instruction.find_first_of("LR", instructionIndex + 1) };
			
			if (turnIndex == std::string::npos) break;

			if (instructions[turnIndex] == 'L')
			{
				dir = dir.rotateClockwise();
			}
			else
			{
				dir = dir.rotateAntiClockwise();
			}

			instructionIndex = turnIndex + 1;
		}

		// DL("col: " << loc.x << " row: " << loc.y << " facing: " << dir);

		int facingNumber{ dir.x == 1 ? 0 : 2 };
		if (dir.x == 0)
		{
			facingNumber = dir.y == 1 ? 1 : 3;
		}

		// DL("facing number: " << facingNumber);

		int sum{ 1000 * loc.y };
		sum += 4 * loc.x;
		sum += facingNumber;

		utils::printAnswer("password in 2d: ", sum);

	}

};

// I think probably don't bother with all that layout stuff you did for part 1!
namespace Puzzle2
{
	Vector2d dirFromChar(char ch)
	{
		switch (ch)
		{
			case 't' : return {  0, -1 };
			case 'b' : return {  0,  1 };
			case 'l' : return { -1,  0 };
			case 'r' : return {  1,  0 };
			default : return { 0, 0 };
		}
	}

	struct Face
	{
		Face(size_t l, size_t r, size_t t, size_t b) :  left{ l }, right{ r }, top{ t }, bottom{ b }
		{
			static int numconstructed{ 0 };
			id = numconstructed++;
		}

		int id{ 0 };

		static int edgeLength;
		static int maxInd;
		static int edgesLeftToFind;

		// I thought it might be easier to work through if I had everything separated
		// (left / right / up /down)
		// but I definitely should have used arrays and stuff because this became quite the pain

		// Bounds
		size_t left{ 0 };
		size_t right{ 0 };
		size_t top{ 0 };
		size_t bottom{ 0 };

		// Neighbours
		Face* leftFace{ nullptr };
		Face* rightFace{ nullptr };
		Face* topFace{ nullptr };
		Face* bottomFace{ nullptr };

		// Neighbour edges (l r u d for now I guess, initialising to \0 in case I want to check truthiness)
		char leftFaceEdge{ '\0' };
		char rightFaceEdge{ '\0' };
		char topFaceEdge{ '\0' };
		char bottomFaceEdge{ '\0' };

		Vector2d faceLoc(Vector2d netLoc)
		{
			return { netLoc.x - static_cast<int>(left), netLoc.y - static_cast<int>(top) };
		}

		// More complex than I meant it to be, gosh
		// Sets both this neighbour & the neighbours edge, plus sets the neighbour's neighbour to this & its edge
		bool setNeighbour(char edge, Face *neighbour, char neighbourEdge)
		{
			char *edgeCharPtr{ nullptr };
			switch (edge)
			{
				case 'l' : if (leftFace)   return false; else leftFace   = neighbour; edgeCharPtr = &leftFaceEdge;   break;
				case 'r' : if (rightFace)  return false; else rightFace  = neighbour; edgeCharPtr = &rightFaceEdge;  break;
				case 't' : if (topFace)    return false; else topFace    = neighbour; edgeCharPtr = &topFaceEdge;    break;
				case 'b' : if (bottomFace) return false; else bottomFace = neighbour; edgeCharPtr = &bottomFaceEdge; break;
				default : return false;
			}
			char *neighbourEdgePtr{ nullptr };
			switch (neighbourEdge)
			{
				case 'l' : if (neighbour->leftFace) throw std::runtime_error("face edge mismatch!"); neighbour->leftFace   = this; *edgeCharPtr = neighbourEdge; neighbourEdgePtr = &(neighbour->leftFaceEdge); break;
				case 'r' : if (neighbour->rightFace) throw std::runtime_error("face edge mismatch!"); neighbour->rightFace  = this; *edgeCharPtr = neighbourEdge; neighbourEdgePtr = &(neighbour->rightFaceEdge); break;
				case 't' : if (neighbour->topFace) throw std::runtime_error("face edge mismatch!"); neighbour->topFace    = this; *edgeCharPtr = neighbourEdge; neighbourEdgePtr = &(neighbour->topFaceEdge); break;
				case 'b' : if (neighbour->bottomFace) throw std::runtime_error("face edge mismatch!"); neighbour->bottomFace = this; *edgeCharPtr = neighbourEdge; neighbourEdgePtr = &(neighbour->bottomFaceEdge); break;
				default : return false;
			}
			*neighbourEdgePtr = edge;

			DOUT << id << edge << " = " << neighbour->id << neighbourEdge << '\n';

			edgesLeftToFind -= 2;
			return true;
		}

		// Unfolded 2d net, x and y directions are aligned - start with this one
		// It's annoying that this is four practically identical blocks
		bool checkEdgesAligned(Face &other)
		{
			if (top == other.top)
			{
				if (!leftFace && !other.rightFace && left - 1 == other.right)
				{
					leftFace = &other;
					leftFaceEdge = 'r';
					other.rightFace = this;
					other.rightFaceEdge = 'l';
					edgesLeftToFind -= 2;
					return true;
				}
				else if (!rightFace && !other.leftFace && right + 1 == other.left)
				{
					rightFace = &other;
					rightFaceEdge = 'l';
					other.leftFace = this;
					other.leftFaceEdge = 'r';
					edgesLeftToFind -= 2;
					return true;
				}
			}
			else if (left == other.left)
		 	{
				if (!topFace && !other.bottomFace && top - 1 == other.bottom)
				{
					topFace = &other;
					topFaceEdge = 'b';
					other.bottomFace = this;
					other.bottomFaceEdge = 't';
					edgesLeftToFind -= 2;
					return true;
				}
				else if (!bottomFace && !other.topFace && bottom + 1 == other.top) // Didn't appear to work?
				{
					bottomFace = &other;
					bottomFaceEdge = 't';
					other.topFace = this;
					other.topFaceEdge = 'b';
					edgesLeftToFind -= 2;
					return true;
				}
			} 
			return false;
		}

		// Slightly confusing because I'm using the same letters for location and direction
		// Consider the return 't' and 'b' as up and down and it makes more sense
		// If we move to an edge, and know which edge of that face it is, this returns the direction we're moving
		// relative to the neighbour face
		char topNeighbourDir()
		{
			switch (topFaceEdge)
			{
				case 'b' : return 't';
				case 't' : return 'b';
				case 'l' : return 'l';
				case 'r' : return 'r';
				default : return '\0';
			}
		}

		// And of course the three practically identical copies for the other neighbours
		char bottomNeighbourDir()
		{
			switch (bottomFaceEdge)
			{
				case 'b' : return 'b';
				case 't' : return 't';
				case 'l' : return 'r';
				case 'r' : return 'l';
				default : return '\0';
			}
		}

		char leftNeighbourDir()
		{
			switch (leftFaceEdge)
			{
				case 'b' : return 'l';
				case 't' : return 'r';
				case 'l' : return 'b';
				case 'r' : return 't';
				default : return '\0';
			}
		}

		char rightNeighbourDir()
		{
			switch (rightFaceEdge)
			{
				case 'b' : return 'r';
				case 't' : return 'l';
				case 'l' : return 't';
				case 'r' : return 'b';
				default : return '\0';
			}
		}

		// When building the net layout, this is for transforming neighbours' edges relative to the current face
		// 'this' is always aligned (this up = cube net's up)
		// Don't worry about it! It works, let's just move on!
		static char transformEdge(char edge, char facing)
		{
			switch (facing)
			{
				// Neighbour's facing the same way as this
				case 't' : return edge; 
				// Neighbour is upside-down relative to this
				case 'b' : switch (edge)
				{
					// Flip everything
					case 't' : return 'b';
					case 'b' : return 't';
					case 'l' : return 'r';
					case 'r' : return 'l';
					default : return '\0';
				}
				// Neighbour's upper edge faces left relative to this
				case 'l' : switch (edge)
				{
					// Rotated clockwise
					case 't' : return 'r';
					case 'b' : return 'l';
					case 'l' : return 't';
					case 'r' : return 'b';
					default : return '\0';
				}
				// Neighbour's upper edge faces right relative to this
				case 'r' : switch (edge)
				{
					// Rotated anticlockwise
					case 't' : return 'l';
					case 'b' : return 'r';
					case 'l' : return 'b';
					case 'r' : return 't';
					default : return '\0';
				}
				default : return '\0';
			}
		}

		// After finding touching neighbours in the cube net, 
		// we can say that two of this face's neighbours neighbour each other
		// IF they aren't on opposite sides of this face (ie not top and bottom or left and right)
		// But we need to account for which edges touch each other after the cube net is folded
		// True if any new neighbours' neighbours are found
		bool findAdjacentNeighbours()
		{
			bool bFoundOne{ false };
			if (leftFace && topFace)
			{
				// transformed left->top = transformed top->left
				auto leftdir{ leftNeighbourDir() };
				auto leftedge{ transformEdge('t', leftdir) };
				auto topdir{ topNeighbourDir() };
				auto topedge{ transformEdge('l', topdir ) };

				if(leftFace->setNeighbour(leftedge, topFace, topedge))
				{
					bFoundOne = true;
				}
			}
			if (leftFace && bottomFace)
			{
				auto leftdir{ leftNeighbourDir() };
				auto leftedge{ transformEdge('b', leftdir) };
				auto bottomdir{ bottomNeighbourDir() };
				auto bottomedge{ transformEdge('l', bottomdir ) };
				
				if(leftFace->setNeighbour(leftedge, bottomFace, bottomedge))
				{
					bFoundOne = true;
				}
			}
			if (topFace && rightFace)
			{
				auto topdir{ topNeighbourDir() };
				auto topedge{ transformEdge('r', topdir) };
				auto rightdir{ rightNeighbourDir() };
				auto rightedge{ transformEdge('t', rightdir ) };
				
				if(topFace->setNeighbour(topedge, rightFace, rightedge))
				{
					bFoundOne = true;
				}
			}
			if (bottomFace && rightFace)
			{
				auto bottomdir{ bottomNeighbourDir() };
				auto bottomedge{ transformEdge('r', bottomdir) };
				auto rightdir{ rightNeighbourDir() };
				auto rightedge{ transformEdge('b', rightdir ) };
				
				if(bottomFace->setNeighbour(bottomedge, rightFace, rightedge))
				{
					bFoundOne = true;
				}
			}
			return bFoundOne;
		}

	};
	int Face::edgeLength = 0;
	int Face::maxInd = 0;
	int Face::edgesLeftToFind = 24;

	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		auto cubeNet{ utils::bufferLinesWhile(inf, [](const std::string &line) -> bool { return line.length(); }) };

		// Count non-space characters first
		int coordCount{ 0 };
		for (auto &&line : cubeNet)
		{
			for (auto &&ch : line)
			{
				if (ch == '.' || ch == '#')
				{
					++coordCount;
				}
			}
		}

		// Use total number of spaces to work out cube dimensions
		const int faceArea{ coordCount / 6 };
		const auto edgeLength{ static_cast<int>(sqrt(faceArea)) };

		if (coordCount % 6 || edgeLength * edgeLength != faceArea)
		{
			throw std::runtime_error{ "input is not a valid cube net" };
		}

		Face::edgeLength = edgeLength;
		Face::maxInd = edgeLength - 1; // More useful than edgeLength really

		std::vector<Face> faces;

		size_t uEdgeLength{ static_cast<size_t>(edgeLength) }; // Is it better to use size_t for edgeLength in the first place I wonder
		for (size_t y{ 0 }; y < cubeNet.size(); y += uEdgeLength)
		{
			for (size_t x{ 0 }; x < cubeNet[y].size(); x += uEdgeLength)
			{
				if (cubeNet[y][x] == '.' || cubeNet[y][x] == '#')
				{
					faces.push_back(Face{ x, x + uEdgeLength - 1, y, y + uEdgeLength - 1 });
				}
			}
		}

		// Find directly touching faces first
		std::for_each(faces.begin(), faces.end() - 1, [&faces](Face &facea)
		{
			std::for_each(faces.begin(), faces.end(), [&facea](Face &faceb)
			{
				if (Face::edgesLeftToFind <= 0 || &facea == &faceb) return;
				if (facea.checkEdgesAligned(faceb))
				{
					DOUT << facea.id << " neighbours: " << faceb.id << '\n';
				}
			});
		});

		// Loop until all neighbouring faces are found
		while (Face::edgesLeftToFind > 0)
		{
			bool bFoundOne{ false };
			for (auto &&face : faces)
			{
				if (face.findAdjacentNeighbours())
				{
					bFoundOne = true;
				}
			}

			if (!bFoundOne)
			{
				if (Face::edgesLeftToFind)
				{
					throw std::runtime_error{ "could not build cube from input" };
				}
				break;
			}
		}

		Vector2d loc{ static_cast<int>(faces[0].left), static_cast<int>(faces[0].top) };
		while (cubeNet[static_cast<size_t>(loc.y)][static_cast<size_t>(loc.x)] == '#') ++loc.x;

		// To print / save the route taken
		std::vector<std::string> route{ cubeNet };
		route[static_cast<size_t>(loc.y)][static_cast<size_t>(loc.x)] = '>';
		
		std::string instructions;
		std::getline(inf, instructions);

		std::string_view instruction{ instructions };
		Vector2d dir{ 1, 0 }; // Right facing to start
		size_t instructionIndex{ 0 };

		Face* currentFace{ &faces[0] };
		while (instructionIndex < instructions.size())
		{
			// Number of moves in current direction
			auto count{ std::atoi(instruction.data() + instructionIndex) };

			DL("new moves: " << count << " * " << dir);

			while (--count >= 0)
			{
				// Do moves
				Vector2d next{ loc + dir };
	
				// Cube wrapping
				Face *nextFace{ currentFace };

				auto unrotatedLoc{ currentFace->faceLoc(loc) };

				Vector2d nextDir{ dir };
				if (next.y < static_cast<int>(currentFace->top)) // moved beolow current face
				{
					unrotatedLoc.y = Face::maxInd;

					
					nextDir = dirFromChar(currentFace->topFaceEdge) * -1;
					nextFace = currentFace->topFace;
				}
				else if (next.y > static_cast<int>(currentFace->bottom)) // moved up above current face
				{
					unrotatedLoc.y = 0;

					nextDir = dirFromChar(currentFace->bottomFaceEdge) * -1;
					nextFace = currentFace->bottomFace;
				}
				else if (next.x < static_cast<int>(currentFace->left)) // moved past left of current face
				{
					unrotatedLoc.x = Face::maxInd;

					nextDir = dirFromChar(currentFace->leftFaceEdge) * -1;
					nextFace = currentFace->leftFace;
				}
				else if (next.x > static_cast<int>(currentFace->right)) // moved past the right of current face
				{
					unrotatedLoc.x = 0;
					
					nextDir = dirFromChar(currentFace->rightFaceEdge) * -1;
					nextFace = currentFace->rightFace;
				}

				if (nextFace != currentFace)
				{
					
					DL("moved from face " << currentFace->id << " to " << nextFace->id);

					auto rotateDir{ dir };
					// auto faceRelative{ currentFace->faceLoc(loc) };

					DL("curr direction: " << dir << ", new direction: " << nextDir);

					// while (rotateDir != nextDir)
					// {
					// 	faceRelative = faceRelative.rotateClockwise() + Face::edgeLength;
					// 	rotateDir = rotateDir.rotateClockwise();
					// }

					while (rotateDir != nextDir)
					{
						DOUT << unrotatedLoc << " spin ";
						unrotatedLoc = unrotatedLoc.rotateClockwise() + Vector2d{ 0, Face::maxInd };
						DOUT << unrotatedLoc << '\n';

						rotateDir = rotateDir.rotateClockwise();
					}

					Vector2d leftuppy{ static_cast<int>(nextFace->left), static_cast<int>(nextFace->top) };

					DOUT << unrotatedLoc << " offset by: " << leftuppy << " = ";

					next = unrotatedLoc + Vector2d{ static_cast<int>(nextFace->left), static_cast<int>(nextFace->top) };

					DL(next);
					//next = faceRelative + Vector2d{ static_cast<int>(currentFace->left), static_cast<int>(currentFace->top) };

					// if (nextDir.x == 0)
					// {
					// 	if (nextDir.y == 1)
					// 	{
					// 		next.y = nextFace->top;
					// 	}
					// 	else
					// 	{
					// 		next.y = nextFace->bottom;
					// 	}
					// }
					// else
					// {
					// 	if (nextDir.x == 1)
					// 	{
					// 		next.x = nextFace->left;
					// 	}
					// 	else
					// 	{
					// 		next.x = nextFace->right;
					// 	}
					// }

				}

				// Break if '#' found (and don't move / undo move)
				if (cubeNet[static_cast<size_t>(next.y)][static_cast<size_t>(next.x)] == '#')
				{
					DL("collision at " << next);
					break;
				}

				if (next == Vector2d{ 0, 50 })
				{
					throw std::runtime_error{ "that should't happen!" };
				}
				
				if (nextFace != currentFace)
				{
					currentFace = nextFace;
					DL("new face: " << currentFace->id);
				}
				loc = next;
				dir = nextDir;

				char arrow{ '>' };
				if (dir.x == 0 && dir.y == -1) arrow = '^';
				else if (dir.x == 0 && dir.y == 1) arrow = 'v';
				else if (dir.x == -1) arrow = '<';
				route[static_cast<size_t>(next.y)][static_cast<size_t>(next.x)] = arrow;

				DOUT << loc << '\n';
				
			}
		
			auto turnIndex{ instruction.find_first_of("LR", instructionIndex + 1) };
			
			if (turnIndex == std::string::npos) break;

			dir = instructions[turnIndex] == 'L' ? dir.rotateClockwise() : dir.rotateAntiClockwise();

			instructionIndex = turnIndex + 1;
		}

		DL("col: " << loc.x << " row: " << loc.y << " facing: " << dir);

		int facingNumber{ dir.x == 1 ? 0 : 2 };
		if (dir.x == 0)
		{
			facingNumber = dir.y == 1 ? 1 : 3;
		}

		DL("facing number: " << facingNumber);

		int sum{ 1000 * (loc.y + 1) };
		sum += 4 * (loc.x + 1);
		sum += facingNumber;

		utils::printAnswer("password on a cube: ", sum);

		// int linenumber{ 0 };
		// for (auto &&line : route)
		// {
		// 	// if (line[0] == 'v')
		// 	// {
		// 	// 	DL("error at " << linenumber);
		// 	// 	break;
		// 	// }
		// 	// ++linenumber;
		// 	DL(line);
		// }

		
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
