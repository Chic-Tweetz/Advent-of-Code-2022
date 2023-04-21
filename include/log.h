#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "utils.h" // Allows enabling with -l flag

class Logger
{
    std::string m_filePath;
    std::ofstream m_logFile;
    bool m_bFileEnabled;
    bool m_bFileOpened; // Disable / Enable logging of particular instance
    
    static std::string defaultFilePath()
    {
        return utils::allLogsDir().append(utils::DayInfo::day()).append(utils::DayInfo::day() + (utils::bPuzzle2 ? "_log_pt2" : "_log_pt1" ));
    }

public:
    Logger(std::string filePath = defaultFilePath(), bool bFileEnabled = true, bool bOverwrite = false) : m_filePath{ filePath }, m_bFileEnabled{ bFileEnabled }, m_bFileOpened{ bFileEnabled }
    {
        if (!bOverwrite)
        {
            int postFix{ 0 };
            while (std::filesystem::exists(m_filePath))
            {
                m_filePath = filePath + "_" + std::to_string(postFix++);
            }
        }

        if (m_bFileEnabled && flags::isSet(flags::Flag::log))
        {
            open();
        }
    }

    void log(const char* text)
    {
        if (m_bFileEnabled && flags::isSet(flags::Flag::log))
        {
            m_logFile << text;
        }
    }

    void ln(const char* text)
    {
        if (m_bFileEnabled && flags::isSet(flags::Flag::log))
        {
            m_logFile << text << '\n';
        }
    }

    bool open()
    {
        auto fp { std::filesystem::path(m_filePath) };
        std::filesystem::create_directories(fp.parent_path());

        m_logFile = std::ofstream{ m_filePath }; 
        if (!m_logFile)
        {
            std::cerr << "log.h could not open " << m_filePath << " for writing\n";
        }
        return m_logFile.good();
    }

    void toggleEnabled()
    {
        m_bFileEnabled = !m_bFileEnabled;

        if (flags::isSet(flags::Flag::log) && m_bFileEnabled && !m_bFileOpened)
        {
            open();
        }
    }

    void setEnabled(bool enable)
    {
        m_bFileEnabled = enable;

        if (m_bFileEnabled && flags::isSet(flags::Flag::log) && !m_bFileOpened)
        {
            open();
        }
    }

    void enable()
    {
        m_bFileEnabled = true;
        if (flags::isSet(flags::Flag::log) && !m_bFileOpened)
        {
            open();
        }
    }

    void disable()
    {
        m_bFileEnabled = false;
    }

    template<typename T>
    Logger &operator<<(T text)
    {
        if (m_bFileEnabled && flags::isSet(flags::Flag::log))
        {
            m_logFile << text;
        }
        return *this;
    }
};
