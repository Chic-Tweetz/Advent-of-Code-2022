#pragma once

// This is chunky enough to split into (probably multiple) header & source files

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>

#define RELPATH(x) utils::getFilePath(__FILE__, x)
#define FOR(n) for (int i{ 0 }; i < n; ++i)
#define ST(i) static_cast<size_t>(i) // Speeding up these common casts
#define TOI(i) static_cast<int>(i) // Beware overflow

// Just for fun
namespace style
{
    inline const char* prefix      { "\033["    };
    inline const char* postfix     { "m"        };
    inline const char* reset       { "\033[0m"  };
    inline const char* black       { "\033[30m" };
    inline const char* red         { "\033[31m" };
    inline const char* green       { "\033[32m" };
    inline const char* yellow      { "\033[33m" };
    inline const char* blue        { "\033[34m" };
    inline const char* magenta     { "\033[35m" }; 
    inline const char* cyan        { "\033[36m" }; 
    inline const char* white       { "\033[37m" }; 
    inline const char* bgBlack     { "\033[40m" };
    inline const char* bgRed       { "\033[41m" };
    inline const char* bgGreen     { "\033[42m" };
    inline const char* bgYellow    { "\033[43m" };
    inline const char* bgBlue      { "\033[44m" };
    inline const char* bgMagenta   { "\033[45m" };
    inline const char* bgCyan      { "\033[46m" };
    inline const char* bgWhite     { "\033[47m" };
    inline const char* bold        { "\033[1m"  };
    inline const char* underline   { "\033[4m"  };
    inline const char* invert      { "\033[7m"  };
    inline const char* unBold      { "\033[21m" };
    inline const char* noUnderline { "\033[24m" };
    inline const char* unInvert    { "\033[27m" };
};

namespace flags
{
    enum class Flag : std::uint8_t
    {
        none             = 0,     
        p1               = 1 << 0,  // run puzzle1
        p2               = 1 << 1,  // run puzzle2
        test             = 1 << 2,  // use test input
        debug            = 1 << 3,  // enable debug macros
        log              = 1 << 4,  // enable logging
        save_answer      = 1 << 5,  // write puzzle answers to files
        overwrite_answer = 1 << 6,  // overwrite puzzle answer files
        custom_input     = 1 << 7,  // follow with input file name
                     //  = 1 << 7   // room for 1 more in 8 bit flag
    };


    using flag_t = std::underlying_type_t<Flag>;

    inline flag_t fcast(Flag f) { return static_cast<flag_t>(f); }
    inline Flag fcast(flag_t ft) { return static_cast<Flag>(ft); }

    Flag operator|(Flag lhs, Flag rhs) { return fcast(fcast(lhs) | fcast(rhs)); }
    Flag operator&(Flag lhs, Flag rhs) { return fcast(fcast(lhs) & fcast(rhs)); }
    Flag operator^(Flag lhs, Flag rhs) { return fcast(fcast(lhs) ^ fcast(rhs)); }
    Flag operator~(Flag f) { return fcast(~fcast(f)); }

    Flag& operator|=(Flag& lhs, Flag rhs) { lhs = lhs | rhs; return lhs; }
    Flag& operator&=(Flag& lhs, Flag rhs) { lhs = lhs & rhs; return lhs; }
    Flag& operator^=(Flag& lhs, Flag rhs) { lhs = lhs ^ rhs; return lhs; }

    Flag flags{ Flag::none };

    // Flags are only single characters for now
    Flag flagFromChar(char f)
    {
        switch (f)
        {
        case '1' : return Flag::p1;
        case '2' : return Flag::p2;
        case 't' : return Flag::test;
        case 'd' : return Flag::debug;
        case 'l' : return Flag::log;
        case 's' : return Flag::save_answer;
        case 'o' : return Flag::overwrite_answer;
        case 'i' : return Flag::custom_input;
        default  : return Flag::none;
        }
    }

    std::map<Flag, std::string> args;

    void set(Flag f) { flags |= f; }
    void set(char c) { flags |= flagFromChar(c); }

    void reset(Flag f) { flags &= ~f; }
    void reset(char c) { flags &= ~flagFromChar(c); }

    bool isSet(Flag f) { return fcast(flags & f); }
    bool isSet(char c) { return isSet(flagFromChar(c)); }

    void setFlagArgs(Flag f, int argInd, int argc, char* argv[])
    {
        if (f == Flag::custom_input)
        {
            if (argInd >= argc)
            {
                std::cerr << "custom input flag (i) is set but no filename given\n";
                reset(f);
            }
            else if (argv[argInd][0] == '-')
            {
                std::cerr << "custom input flag (i) is set but followed by a flag (hyphen -prefix)\n";
                reset(f);
            }
            else
            {
                args[f] = argv[argInd];
            }   
        }
    }

    void set(int argc, char* argv[])
    {
        for (int i{ 1 }; i < argc; ++i)
        {
            if (argv[i][0] == '-')
            {
                int j{ 0 };
                while (argv[i][++j] != '\0')
                {
                    flags |= flagFromChar(argv[i][j]);
                    setFlagArgs(flagFromChar(argv[i][j]), i + 1, argc, argv);
                }
            }
        }
    }



    bool d() { return fcast(flags & Flag::debug); } // Quick debug flag check with if(flags::d())
    bool t() { return fcast(flags & Flag::test); } // Quick debug flag check with if(flags::t())
}
// We have file helpers
// We also have some methods for common string & input file uses - could make two headers
namespace utils
{
    // By default (neither p1 or p2 flags set) we run both: if one is set, check if the other is to decide if that runs
    bool doP1() { return flags::isSet(flags::Flag::p1) || !flags::isSet(flags::Flag::p2); }
    
    // Use this var to automatically have printAnswer use puzzle2
    bool bPuzzle2{ false };
    // Call doP2 before running the puzzle to have bPuzzle2 set
    bool doP2() { bPuzzle2 = true; return flags::isSet(flags::Flag::p2) || !flags::isSet(flags::Flag::p1); }

    std::string defaultInputFile()
    {
        if (flags::isSet(flags::Flag::custom_input))
        {
            return flags::args[flags::Flag::custom_input];
        }
        if (flags::isSet(flags::Flag::test))
        {
            return "test";
        }
        return "input";
    }
    
    class DayInfo
    {
    public:
        static void init(std::filesystem::path cppFile, const std::string& input);

        static bool initialised()
        {
            return m_singletonInstance.get(); // False while nullptr
        }

        template <typename T>
        static void tryAnswer1(T answer, const std::string &flavourStart = "", const std::string &flavourEnd = "");

        template <typename T>
        static void tryAnswer2(T answer, const std::string &flavourStart = "", const std::string &flavourEnd = "");
        
        // Getters from m_singletonInstance (no checks for null here just remember to use init() first)
        static const std::string &day() { return m_singletonInstance->m_day; } 
        static const std::string &inputFilePath() { return m_singletonInstance->m_inputFilePath; } 
        static const std::string &puzzleSolutionsDir() { return m_singletonInstance->m_puzzleSolutionsDir; } 
        static const std::string &pt1SolutionFile() { return m_singletonInstance->m_pt1SolutionFile; } 
        static const std::string &pt2SolutionFile() { return m_singletonInstance->m_pt2SolutionFile; } 

    private:
        const std::string m_day;
        const std::string m_inputFilePath;
        const std::string m_puzzleSolutionsDir;
        const std::string m_pt1SolutionFile;
        const std::string m_pt2SolutionFile;

        DayInfo(std::filesystem::path cppFile, const std::string& input);

        static std::unique_ptr<DayInfo> m_singletonInstance;

        template<typename T>
        static void tryAnswer(T answer, bool bPart2 = false, const std::string &flavourStart = "", const std::string &flavourEnd = "");
        
        static void saveAnswer(const std::string &answer, bool bPart2 = false);
    };

    std::unique_ptr<DayInfo> DayInfo::m_singletonInstance = nullptr;

    // Initialise with __FILE__ macro and optional input file name
    void init(std::filesystem::path cppFile, const std::string& input = defaultInputFile())
    {
        DayInfo::init(cppFile, input);
    }

    // We usually (always?) want to get the input file after init, so we can call init through this method and return input
    const std::string& inputFile(std::filesystem::path cppFile, const std::string& input = defaultInputFile())
    {
        if (!DayInfo::initialised())
            DayInfo::init(cppFile, input);

        return DayInfo::inputFilePath();
    }

    const std::string& inputFile()
    {
        return DayInfo::inputFilePath();
    }

    std::filesystem::path projectPath()
    {
        return std::filesystem::path{ __FILE__ }.parent_path().parent_path().string();
    }

    std::filesystem::path allSolutionsDir()
    {
        return projectPath().append("solutions");
    }

    std::filesystem::path allLogsDir()
    {
        return projectPath().append("logs");
    }

    // If answer is a string, it will be used for flavourStart - coerce the correct params by adding an empty flavourEnd arg ""
    template<typename T>
    void printAnswer(const std::string &flavourStart, const T answer, const std::string &flavourEnd = "")
    {
        if (bPuzzle2)
            DayInfo::tryAnswer2(answer, flavourStart, flavourEnd);
        else
            DayInfo::tryAnswer1(answer, flavourStart, flavourEnd);
    }

    template<typename T>
    void printAnswer(const T answer, const std::string &flavourEnd = "")
    {
        printAnswer("", answer, flavourEnd);
    }

    // Blank version for dayTemplate
    void printAnswer()
    {
    }

    // Gets fileDir/input by default or fileDir/test with -t cmd arg
    const std::string getFilePath(const std::string &srcFilePath, const std::string &fileName = defaultInputFile())
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
            if(!inf) break; // Was getting the last empty line without this
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

    std::vector<std::string> split(std::string_view str, const std::string &splitOn)
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

    std::vector<int> splitInts(std::string_view str, const std::string &splitOn)
    {
        std::vector<int> list;

        if (splitOn.length() == 0)
        {
            std::cerr << ("utils::split called with empty split string\n");
            return list;
        }

        auto begin{ str.begin() };
        while (begin < str.end())
        {
            auto match{ std::search(begin, str.end(), splitOn.begin(), splitOn.end()) };

            list.push_back( std::stoi(std::string{ begin, match }) );
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

// Compare answers against those stored in files in solutions/dayxx, optionally save answer to new file / overwrite
template<typename T>
void utils::DayInfo::tryAnswer(T answer, bool bPart2, const std::string &flavourStart, const std::string &flavourEnd)
{
    std::stringstream ss;
    ss << answer;
    const std::string answerStr{ ss.str() };

    std::ifstream correctAnswerFile{ bPart2 ? pt2SolutionFile() : pt1SolutionFile() };
    if (correctAnswerFile.good())
    {
        correctAnswerFile.close();
        
        auto correctStr{ bufferInput( ( bPart2 ? pt2SolutionFile() : pt1SolutionFile() ) ) };

        if (correctStr != answerStr)
        {
            if (flags::isSet(flags::Flag::overwrite_answer))
            {
                saveAnswer(answerStr, bPart2);
            }
            else if (flags::isSet(flags::Flag::save_answer))
            {
                std::cout << "save flag set but file exists, use -o flag to overwrite " << ( bPart2 ? pt2SolutionFile() : pt1SolutionFile() ) << '\n';
            }

            std::cout << "\033[31m" << answer << "\033[0m" << " is incorrect.\nThe correct answer is: " << correctStr << '\n';
            return;
        }
    }
    if (flags::isSet(flags::Flag::save_answer | flags::Flag::overwrite_answer))
    {
        saveAnswer(answerStr, bPart2);
    }

    std::cout << flavourStart << style::green << style::bold << answerStr << style::reset << flavourEnd << '\n';
}

template <typename T>
void utils::DayInfo::tryAnswer1(T answer, const std::string &flavourStart, const std::string &flavourEnd)
{
    tryAnswer(answer, false, flavourStart, flavourEnd);
}

template <typename T>
void utils::DayInfo::tryAnswer2(T answer, const std::string &flavourStart, const std::string &flavourEnd)
{
    tryAnswer(answer, true, flavourStart, flavourEnd);
}

void utils::DayInfo::saveAnswer(const std::string &answer, bool bPart2)
{
    if (!std::filesystem::exists(utils::allSolutionsDir()))
    {
        std::cout << "creating dir: " << utils::allSolutionsDir() << '\n';
        std::filesystem::create_directory(utils::allSolutionsDir());
    }
    if (!std::filesystem::exists(puzzleSolutionsDir()))
    {
        std::cout << "creating dir: " << puzzleSolutionsDir() << '\n';
        std::filesystem::create_directory(puzzleSolutionsDir());
    }

    auto outFilePath{ bPart2 ? pt2SolutionFile() : pt1SolutionFile() };
    if (!flags::isSet(flags::Flag::overwrite_answer) && std::filesystem::exists(outFilePath))
    {
        std::cerr << style::yellow << "overwrite disabled, not writing answer to " << style::reset << outFilePath << '\n';
        std::cout << "use " << style::bold << style::magenta << "-o " << style::reset << "flag to save and overwrite\n";
        return;
    }

    std::ofstream of{ outFilePath };
    if (!of)
    {
        std::cerr << "could not write to file " << outFilePath << '\n';
    }
    else
    {
        std::cout << "writing " << answer << "\nto " << outFilePath << '\n';
        of << answer;
    }
}

void utils::DayInfo::init(std::filesystem::path cppFile, const std::string& input)
{
    if (!m_singletonInstance)
    {
        m_singletonInstance = std::unique_ptr<DayInfo>{ new DayInfo{ cppFile, input } };
    }
}

utils::DayInfo::DayInfo(std::filesystem::path cppFile, const std::string& input) :
    m_day{ cppFile.filename().replace_extension("").string() },
    m_inputFilePath{ utils::getFilePath(cppFile, input) },
    m_puzzleSolutionsDir{ utils::allSolutionsDir().append(m_day).string() },
    m_pt1SolutionFile{ utils::allSolutionsDir().append(m_day).append(input + "_solution_pt1") },
    m_pt2SolutionFile{ utils::allSolutionsDir().append(m_day).append(input + "_solution_pt2") }
{
}
