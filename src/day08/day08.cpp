// Copy and paste for each day for a quick start

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

void checkRow(const std::vector<std::string> &map, std::vector<std::vector<bool>> &visibility, const size_t row)
{
    size_t col{ 0 };
    char largest{ map[row][col] };
    size_t reverseEnd{ 1 };

    while(++col < map[row].length() - 1)
    {
        if (map[row][col] > largest)
        {
            visibility[row][col] = true;
            reverseEnd = col;

            if (map[row][col] == '9')
            {
                break;
            }

            largest = map[row][col];
        }
    }

    col = map[row].length() - 1;
    largest = map[row][col];

    while (--col > reverseEnd)
    {
        if (map[row][col] > largest)
        {
            visibility[row][col] = true;

            if (map[row][col] == '9')
            {
                break;
            }

            largest = map[row][col];
        }
    }
}

void checkCol(const std::vector<std::string> &map, std::vector<std::vector<bool>> &visibility, const size_t col)
{
    size_t row{ 0 };
    char largest{ map[row][col] };
    size_t reverseEnd{ 1 };

    while(++row < map.size() - 1)
    {
        if (map[row][col] > largest)
        {
            visibility[row][col] = true;
            reverseEnd = row;

            if (map[row][col] == '9')
            {
                break;
            }

            largest = map[row][col];
        }
    }

    row = map.size() - 1;
    largest = map[row][col];

    while (--row > reverseEnd)
    {
        if (map[row][col] > largest)
        {
            visibility[row][col] = true;
            if (map[row][col] == '9')
            {
                break;
            }
            largest = map[row][col];

        }
    }
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

        const std::vector<std::string> trees{ utils::bufferLines(infile) };

        // I can't think how to not count trees twice when checking rows then columns
        // So we're storing whether each tree is visible in a vector with matching row/col indices
        std::vector<std::vector<bool>> treeVisibility{ trees.size(), std::vector(trees[0].length(), false) };
        
        for (auto &row : treeVisibility)
        {
            *(row.begin()) = true;
            *(row.rbegin()) = true;
        }

        *(treeVisibility.begin()) = std::vector<bool>( trees[0].length(), true );
        *(treeVisibility.rbegin()) = *(treeVisibility.begin());

        for (size_t col{ 0 }; col < trees[0].length(); ++col)
        {
            checkCol(trees, treeVisibility, col);
        }
        int count{ 0 };
        for (size_t row{ 0 }; row < trees.size(); ++row)
        {
            checkRow(trees, treeVisibility, row);
            // std::cout << trees[row] << '\n';


            for (size_t i{ 0 }; i < treeVisibility[0].size(); ++i)
            {
                if (treeVisibility[row][i])
                {
                    ++count;
                    std::cout << trees[row][i];
                }
                else
                {
                    std::cout << ' ';
                }
            }
            std::cout << "\n";

        }
        std::cout << "\nTotal visible trees: " << count << '\n';

	}
};

namespace Puzzle2
{
    using TreeMap = std::vector<std::string>;
    using TreeRow = std::string;

    int treeScoreIterators(const TreeMap &map, TreeMap::const_iterator treerow, TreeRow::const_iterator tree)
    {
        const char treeHeight{ *tree };
        int score{ 0 };

        auto comptree{ tree };

        while (++comptree != (*treerow).end())
        {
            ++score;

            if (*comptree >= treeHeight)
            {
                break;
            }
        }

        comptree = tree;
        int visible { 0 };
        
        while (--comptree >= (*treerow).begin())
        {
            ++visible;

            if (*comptree >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        auto row{ treerow };
        visible = 0;

        const auto treeOffset{ tree - (*treerow).begin() };

        while (++row != map.end())
        {
            ++visible;

            if ((*row)[static_cast<size_t>(treeOffset)] >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        row = treerow;
        visible = 0;

        while (--row != map.begin() - 1)
        {
            ++visible;

            if ((*row)[static_cast<size_t>(treeOffset)] >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        return score;
    }

    int treeScore(const std::vector<std::string> &map, const size_t treerow, const size_t treecol)
    {
        const char treeHeight{ map[treerow][treecol] };
        int score{ 0 };

        int col{ static_cast<int>(treecol) };

        while (++col < static_cast<int>(map[treerow].length()))
        {
            ++score;

            if (map[treerow][static_cast<size_t>(col)] >= treeHeight)
            {
                break;
            }
        }

        col = treecol;
        int visible { 0 };
        
        while (--col >= 0)
        {
            ++visible;

            if (map[treerow][static_cast<size_t>(col)] >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        int row{ static_cast<int>(treerow) };
        visible = 0;

        while (++row < static_cast<int>(map.size()))
        {
            ++visible;

            if (map[static_cast<size_t>(row)][treecol] >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        row = treerow;
        visible = 0;

        while (--row >= 0)
        {
            ++visible;

            if (map[static_cast<size_t>(row)][treecol] >= treeHeight)
            {
                break;
            }
        }

        score *= visible;

        return score;

    }

	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

        const TreeMap trees{ utils::bufferLines(infile) };

        int highScore{ 0 };
        // for (size_t row{ 0 }; row < trees.size(); ++row)
        // {
        //     for (size_t tree{ 0 }; tree < trees[0].length(); ++tree)
        //     {
        //         auto score{ treeScore(trees, row, tree) };
        //         highScore = score > highScore ? score : highScore;
        //     }
        // }

        // std::cout << "Best tree score: " << highScore << '\n';


        for (auto rowiter{ trees.begin() }; rowiter != trees.end(); ++rowiter)
        {
            for (auto treeiter{ (*rowiter).begin() }; treeiter != (*rowiter).end(); ++treeiter)
            {
                auto score{ treeScoreIterators(trees, rowiter, treeiter) };
                highScore = score > highScore ? score : highScore;
            }
        }
        
        std::cout << "Best tree score (iterators) wow it worked: " << highScore << '\n';

	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}