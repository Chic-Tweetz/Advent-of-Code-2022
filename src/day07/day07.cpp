#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "utils.h"

class DirTree
{
public:
    DirTree() : m_root{ new Dir{ "/", nullptr } }, m_pwd{ m_root } {}
    ~DirTree() { delete m_root; }

    void parseInput(const std::string &line)
    {
        // cmd
        if (line[0] == '$')
        {
            // There are only two commands, so:
            if (line[2] == 'c') // cd
            {
                // 6th char onwards = dir name
                auto dir{ line.substr(5, line.length()) };

                // std::cout << "Parsed cd: " << dir << '\n';

                cd(dir);
                return;
            }
            else if (line[2] == 'l') // ls
            {
                // Don't need to do anything really
                // although you could keep track of whether a directory
                // has had its contents listed
                // and then you could skip until the next command
                return;
            }

            return;
        }
        // directory listed by ls
        else if (line[0] == 'd') // dir dirname
        {
            // 5th char onwards = dir name
            auto dir{ line.substr(4, line.length()) };
            // std::cout << "Parsed dir name: " << dir << '\n';
            mkdir(dir);
            return;
        }
        // file listed by ls
        else
        {
            // Get the filesize
            size_t i{ 0 };
            while(line[i++] != ' ');

            int size{ std::stoi(line.substr(0, i)) };
            auto name{ line.substr(i, line.length()) };

            touch(name, size); 
        }
    }

    void sumPuzzleDirsBelow(int limit = 100000) const
    {
        int acc{ 0 };
        for (auto & dir : m_allDirectories)
        {
            // std::cout << dir->name << " | " << dir->size;
            if (dir->size <= limit)
            {
                acc += dir->size;
                // std::cout << " ****** YES **  " << acc;
            }
            // std::cout << '\n';
        }
        std::cout << "Sum of directory sizes of at most " << limit << ": " << acc << '\n';
    }

    void chooseDirectoryToDelete(const int totalStorage = 70000000, const int requiredStorage = 30000000) const
    {
        std::cout << "Storage used: " << m_root->size;

        const int freeStorage{ totalStorage - m_root->size };
        const int storageToFree{ requiredStorage - freeStorage };

        auto minDir{ std::min_element(m_allDirectories.begin(), m_allDirectories.end(),
            [storageToFree](const Dir* const a, const Dir* const b)
            {
                return a->size < b->size && a->size > storageToFree;
            }) 
            };

        std::cout << ", free: " << freeStorage << '\n'
            << "amount to delete: " << storageToFree << '\n' 
            << "Dir choice: " << (*minDir)->name 
            << ", with size: " << (*minDir)->size << '\n';
    }
    
private:

    struct File
    {
        // size was defaulting to -1 for a sentinel value I never used (something like if size == -1, recount or ignore you know)
        // but this somehow ended up with the answer being off by -32 
        // setting default of _size = 0 gets the right answer!
        File(const std::string &_name, const int _size = 0) : name{ _name }, size{ _size } {};
        std::string name;
        int size{ 0 }; // Perhaps a sentinel value (-1 or something) could be useful
    };
    struct Dir : public File
    {
        Dir(const std::string &_name, Dir *_parent) : File{ _name }, parent{ _parent } {}
        ~Dir()
        {
            for (auto file : files)
            {
                // std::cout << "deleting: " << file.first << '\n';
                delete file.second;
            }
            for (auto dir : directories)
            {
                // std::cout << "deleting: " << dir.first << '\n';
                delete dir.second;
            }
        }

        // std::string name;
        Dir* parent;
        // std::map<std::string &, File*> children; // Dir inherits from Files so they could be in one container
        // I don't think map was the right choice
        std::map<const std::string, Dir*> directories;
        
        std::map<const std::string, File*> files;

        // int size{ -1 }; // Sentinel value suggesting size hasn't been calculated
    };

    Dir* m_root;
    Dir* m_pwd;
    
    // Because I still want to get the solution after all
    std::vector<Dir*> m_allDirectories;

    const std::array<const std::string, 3> reserved{ ".", "..", "/" };

    Dir* cd(const std::string &_name)
    {
        if (_name == ".")
        {
            return m_pwd;
        }
        if (_name == "..")
        {
            m_pwd = m_pwd->parent ? m_pwd->parent : m_root;
            return m_pwd;
        }
        if (_name == "/")
        {
            m_pwd = m_root;
            return m_pwd;
        }

        if (m_pwd->directories.contains(_name))
        {
            m_pwd = m_pwd->directories[_name];
            return m_pwd;
        }
        else
        {
            std::cout << "no such directory: " << _name << '\n';
            return m_pwd;
        }
    }

    Dir* mkdir(const std::string &_name)
    {
        if (m_pwd->directories.contains(_name))
        {
            std::cout << "directory already exists: " << _name << '\n'; 
            return cd(_name);
        }

        if (std::find(reserved.begin(), reserved.end(), _name) != reserved.end())
        {
            std::cout << "reserved dir name: " << _name << '\n';
            return cd(_name);
        }

        Dir* newdir{ new Dir{ _name, m_pwd } };
        m_pwd->directories[newdir->name] = newdir;

        m_allDirectories.push_back(newdir);
         
        return newdir;
    }

    File* touch(const std::string &name, int size)
    {
        if (std::find(reserved.begin(), reserved.end(), name) != reserved.end())
        {
            std::cout << "reserved names: " << name << '\n';
            return nullptr; // Maybe return null?
        }

        if (m_pwd->files.contains(name))
        {
            std::cout << "file already exists: " << name << '\n'; 
            return nullptr;
        }

        auto newfile{ new File(name, size) };

        m_pwd->files[newfile->name] = newfile;
        // m_pwd->size += size;

        addToSize(size, m_pwd);
        
        return m_pwd->files[name];
    }

    void addToSize(const int size, Dir* const dir) const
    {
        dir->size += size;
        if(dir->parent)
        {
            addToSize(size, dir->parent);
        }
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

        DirTree root;

        while(inf)
        {
            std::string instr;
            std::getline(inf, instr);
            if (instr.length() > 0)
            {
                root.parseInput(instr);
            }
        }

        root.sumPuzzleDirsBelow(100000);
    
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

        DirTree root;

        while(inf)
        {
            std::string instr;
            std::getline(inf, instr);
            if (instr.length() > 0)
            {
                root.parseInput(instr);
            }
        }

        root.chooseDirectoryToDelete(70000000, 30000000);
    
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

	Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}   