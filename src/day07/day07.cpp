// Turns out the first one only needed a -1 changed to a 0 and now it works!

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "utils.h"

// Redid this messily to try and get an answer because it was annoying me
// Turned out I needed to change a -1 to a 0 in the first attempt lol

// directories from cd
// with parents
// up to the root
// where we will start

// root/jssbn/lfrctthp

//etc

// heirarchy needs to be preserved somehow

// file sizes need to be calculated

// can kind of assume that we don't need to create directories until we cd into them

struct File
{
    File(const std::string &_name, int _size) : name{ _name }, size{ _size } 
    {
    }

    std::string name;
    int size;
};

class Dir
{
public: 
    Dir(const std::string &name, Dir* parent) : m_name{ name }, m_parent{ parent } {}
    ~Dir()
    {
        for(auto file : m_files)
        {
            delete file;
        }
        for(auto directory : m_chlidDirectories)
        {
            delete directory;
        }
    }

    const std::string m_name;
    Dir* m_parent;
    std::vector<Dir*> m_chlidDirectories;
    std::vector<File*> m_files;

};

class FileTree
{
public:
    FileTree() : m_root{ new Dir{ "/", nullptr } }, m_pwd{ m_root } { }

    void touch(const std::string &instr)
    {
        uint i{ 0 };
        while(instr[i++] != ' ');

        auto name{ instr.substr(i, instr.length()) };

        for (auto file : m_pwd->m_files)
        {
            if (file->name == name)
            {
                std::cout << '\n';
                return;
            }
        }

        int size{ 0 };
        
        auto sizestr{ instr.substr(0, i) };
        try
        {
            size = std::stoi(sizestr);
            /* code */
        }
        catch(const std::exception& e)
        {
            std::cout << "sizestr: " << sizestr << '\n';
            std::cerr << e.what() << '\n';
        }
        
        // std::cout << name << ": " << size << ".\n";

        std::cout << "touch " << name << ", " << size << '\n';

        m_pwd->m_files.push_back(new File(name, size));
    }

    void mkdir(const std::string &name)
    {
        for (auto dir : m_pwd->m_chlidDirectories)
        {
            if (dir->m_name == name)
            {
                std::cout << '\n';
                return;
            }
        }

        std::cout << "mkdir " << name << '\n';

        m_pwd->m_chlidDirectories.push_back(new Dir{ name, m_pwd });
    }

    void parse(const std::string &instr)
    {
        std::cout << instr;

        for (size_t i{ instr.length() }; i < 25; ++i)
        {
            std::cout << ' ';
        }
        std::cout << "|| ";

        if (instr[0] == '$')
        {
            if (instr[2] == 'c')
            {
                auto dir{ instr.substr(5, instr.length()) };
                // std::cout << dir << " : ";
                
                if (dir == "/")
                {
                    std::cout << "cd /" << '\n';
                    m_pwd = m_root;
                    return;
                    
                }
                else if (dir == "..")
                {
                    if (m_pwd->m_parent)
                    {
                        std::cout << "cd " << m_pwd->m_parent->m_name << '\n';
                        m_pwd = m_pwd->m_parent;

                    }
                        return;
                }
                else
                {
                    for (auto subdir : m_pwd->m_chlidDirectories)
                    {
                        // std::cout << subdir->m_name << " vs " << dir << '\n';
                        if (subdir->m_name == dir)
                        {
                            std::cout << "cd " << subdir->m_name << '\n';
                            m_pwd = subdir;
                            return;
                        }
                    }
                    std::cout << "no match for cd " << dir << '\n';
                    return;
                }  
            }
            else if (instr[2] == 'l')
            {
                std::cout << '\n';
                return;
            }        
        }

        else if (instr[0] == 'd')
        {
            mkdir(instr.substr(4, instr.length()));
            return;
        }
        
        touch(instr);
    }

//private:
    Dir* m_root;
    Dir* m_pwd;
};

std::vector<std::pair<Dir*, int>> dirsUnderLimit;

int thinking(Dir *dir)
{
    int acc{ 0 };
    for (auto directory : dir->m_chlidDirectories)
    {
        acc += thinking(directory);
    }
    for (auto file : dir->m_files)
    {
        acc += file->size;
    }
    if (acc <= 100000)
    {
        dirsUnderLimit.push_back(std::pair<Dir*, int>{ dir, acc });
    }
    return acc;
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

        FileTree ft;
        int limitLines{ 5000000 };
        while(inf && limitLines-- > 0)
        {
            std::string instr;
            std::getline(inf, instr);
            if (instr.length() > 0)
            {
                ft.parse(instr);
                // std::cout << instr << " | gets:\n";
                // File{ instr }; 
            }
        }

        thinking(ft.m_root);

        int total{ 0 };
        for (auto dir : dirsUnderLimit)
        {
            total += dir.second;
        }
        std::cout << "answer: " << total << '\n';
        // How the hell was I off by 32 the first time?!

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
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}