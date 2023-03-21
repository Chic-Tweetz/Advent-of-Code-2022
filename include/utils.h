#pragma once

#include <chrono> // for std::chrono functions
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace utils
{
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
    const std::string getFilePath(const std::string &srcFilePath, const std::string &fileName = "input")
    {
        return std::filesystem::path{ srcFilePath }.parent_path().append(fileName).string();
    }
    
    /* This version tested much slower than the other
    const std::string bufferInput(const std::string& file)
    {
        std::ifstream inf{ file }; // what's the issue?
        if (!inf.good())
        {
            throw std::runtime_error{ "Could not open file: " + file };
        }
        
        // std::string buffer;
        std::stringstream ss;

        inf.seekg(0, std::ios::end);
        auto end{ inf.tellg() };
        // buffer.reserve(inf.tellg());
        inf.seekg(0, std::ios::beg);

        while(inf.tellg() != end)
        {
            ss << static_cast<char> (inf.get());
            // buffer.push_back(inf.get());
        }

        // buffer.shrink_to_fit();
        return ss.str();

    }
    */

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

};