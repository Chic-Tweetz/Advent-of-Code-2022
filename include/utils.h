#pragma once

#include <algorithm>
#include <chrono> // for std::chrono functions
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>

// Debug macros - DPRINT, DPRINTLN can be toggled on or off, DERR prints in RED text :O
#ifdef DEBUG
#  define DERR(x) std::cerr << "\033[31m" << x << "\033[0m"; 
	
namespace debug
{
	bool bPrintEnabled{ true }; // Toggle DPRINT when needed
}

#  define DPRINT(x) if (debug::bPrintEnabled) { std::cout << x; }
#  define DPRINTLN(x) if (debug::bPrintEnabled) { std::cout << x << '\n'; }
#  define DTOGGLEPRINT() debug::bPrintEnabled = !debug::bPrintEnabled;
#  define DSETPRINTENABLED(b) debug::bPrintEnabled = b;
#else
#  define DPRINT(x)
#  define DPRINTLN(x)
#  define DERR(x)
#  define DTOGGLEPRINT()
#  define DSETPRINTENABLED(b)
#endif

namespace utils
{
    template<typename T>
    void printAnswer(const T answer, const T correctAnswer, std::string_view flavourStart = "", std::string_view flavourEnd = "")
    {
        if (answer == correctAnswer)
        {
            std::cout << flavourStart << "\033[32m" << answer << "\033[0m" << flavourEnd << '\n';
        }
        else
        {
            std::cout << "\033[31m" << answer << "\033[0m" << " is incorrect. Answer is: " << correctAnswer << '\n';
        }
    }
    // Copied & pasted from learncpp.com
    class Timer
    {
    private:
        // Type aliases to make accessing nested type easier
        using Clock = std::chrono::steady_clock;
        using Second = std::chrono::duration<double, std::ratio<1> >;

        std::chrono::time_point<Clock> m_beg { Clock::now() };

    public:
        void reset()
        {
            m_beg = Clock::now();
        }

        double elapsed() const
        {
            return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
        }
        
        void printElapsed() const
        {
            const double time{ elapsed() }; 
            std::cout << "Elapsed: " << time << '\n';
        }
    };

    // Pass the __FILE__ macro for the first arg to get files relative to source cpp file dir
    // I've been using #ifdef TESTINPUT for a few files already so I'll put it here too
    // Now I'll relax with the preprocessor directives because this is getting out of hand
#ifdef TESTINPUT
    // Gets fileDir/input by default or fileDir/test with #TESTINPUT defined
    const std::string getFilePath(const std::string &srcFilePath, const std::string &fileName = "test")
#else
    // Gets fileDir/input by default or fileDir/test with #TESTINPUT defined
    const std::string getFilePath(const std::string &srcFilePath, const std::string &fileName = "input")
#endif
    {
        return std::filesystem::path{ srcFilePath }.parent_path().append(fileName).string();
    }

    void forEachLine(const std::string &file, std::function<void(std::string_view)> fnc)
    {
        std::ifstream inf{ file }; 
        if (!inf.good())
        {
            throw std::runtime_error{ "Could not open file: " + file };
        }
        while (inf)
        {
            std::string line;
            std::getline(inf, line);
            fnc(line);
        }
    }

    std::string bufferInput(const std::string &file)
    {
        std::ifstream inf{ file }; 
        if (!inf.good())
        {
            throw std::runtime_error{ "Could not open file: " + file };
        }
        
        std::string buffer;
        // std::stringstream ss;

        inf.seekg(0, std::ios::end);
        // auto end{ inf.tellg() };
        buffer.reserve(static_cast<unsigned int>(inf.tellg()));
        inf.seekg(0, std::ios::beg);

        for (char ch{ static_cast<char>(inf.get()) }; !inf.eof(); ch = static_cast<char>(inf.get()))
        {
            buffer.push_back(ch);
        }

        buffer.shrink_to_fit();
        return buffer;

    }
    
    std::vector<std::string> bufferLines(const std::string &file)
    {
        std::ifstream inf{ file }; 
        if (!inf.good())
        {
            throw std::runtime_error{ "Could not open file: " + file };
        }
        std::vector<std::string> buffer;

        while(inf)
        {
            std::string line;
            std::getline(inf, line);
            if (line.length())
            {
                buffer.push_back(line);
            }
        }

        return buffer;
    }

    std::vector<std::string> bufferLinesIf(const std::string &file, bool (*condition)(const std::string&))
    {
        std::ifstream inf{ file }; 
        if (!inf.good())
        {
            throw std::runtime_error{ "Could not open file: " + file };
        }
        std::vector<std::string> buffer;

        while(inf)
        {
            std::string line;
            std::getline(inf, line);
            if (condition(line))
            {
                buffer.push_back(line);
            }
        }

        return buffer;
    }

    // Uses an out parameter to behave like std::getline. Discards lines until a condition is met
    void skipLinesUntil(std::ifstream &inf, std::string &line, bool (*condition) (const std::string&))
    {
        do
        {
			std::getline(inf, line);
        } while (!condition(line));
    }

    std::vector<std::string> getLinesUntil(std::ifstream &inf, bool (*condition) (const std::string&))
    {
        std::vector<std::string> block;
        do
        {
            block.push_back("");
			std::getline(inf, block.back());
        } while (!condition(block.back()));
        return block;
    }

    std::vector<std::string> split(const std::string &str, const std::string &splitOn)
    {
        std::vector<std::string> list;

        if (splitOn.length() == 0)
        {
            std::cerr << ("utils::split called with empty split string\n");
            return list;
        }

        auto begin{ str.begin() };
        while (begin < str.end())
        {
            auto match{ std::search(begin, str.end(), splitOn.begin(), splitOn.end()) };

            list.push_back( std::string{ begin, match } );
            begin = match + static_cast<int>(splitOn.length());

        }
        return list;
    }

    void doOnSplit(const std::string &str, const std::string &splitOn, std::function<void(const std::string&)>fnc)
    {
        if (splitOn.length() == 0)
        {
            std::cerr << ("utils::split called with empty split string\n");
            return;
        }

        auto begin{ str.begin() };
        while (begin < str.end())
        {
            auto match{ std::search(begin, str.end(), splitOn.begin(), splitOn.end()) };

            fnc(std::string{ begin, match });

            begin = match + static_cast<int>(splitOn.length());

        }
    }

    // I think string_view makes more sense
    void doOnSplit(std::string_view str, std::string_view splitOn, std::function<void(std::string_view)>fnc)
    {
        if (splitOn.length() == 0)
        {
            std::cerr << ("utils::split called with empty split string\n");
            return;
        }

        auto begin{ str.begin() };
        while (begin < str.end())
        {
            auto match{ std::search(begin, str.end(), splitOn.begin(), splitOn.end()) };

            fnc(std::string_view{ begin, match });

            begin = match + static_cast<int>(splitOn.length());

        }
    }
};