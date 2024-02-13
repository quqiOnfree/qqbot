#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <functional>

namespace Log
{
    template<typename T>
    concept StreamType = requires (T t) {
        std::cout << t;
    };

    class Logger
    {
    public:
        enum class LogMode
        {
            INFO = 0,
            WARNING,
            ERROR,
            CRITICAL,
            DEBUG
        };

        Logger() :
            m_isRunning(true),
            m_thread(std::bind(&Logger::workFunction, this))
        {
            //单例模式
            if (m_hasCreate)
                throw std::runtime_error("This logger has create");
            else
                m_hasCreate = true;

            if (!openFile())
                throw std::runtime_error("Can't no open log file.");
        }

        ~Logger()
        {
            m_isRunning = false;
            m_cv.notify_all();

            if (m_thread.joinable())
                m_thread.join();
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void info(Args&&... args)
        {
            this->print(LogMode::INFO, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void warning(Args&&... args)
        {
            this->print(LogMode::WARNING, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void error(Args&&... args)
        {
            this->print(LogMode::ERROR, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void critical(Args&&... args)
        {
            this->print(LogMode::CRITICAL, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void debug(Args&&... args)
        {
#ifdef _DEBUG
            this->print(LogMode::DEBUG, std::forward<Args>(args)...);
#endif // _DEBUG
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void print(LogMode mode, Args&&... args)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_msgQueue.push([=]() {
                std::string modeString;

                switch (mode)
                {
                case Logger::LogMode::INFO:
                    modeString = "[INFO]";
                    break;
                case Logger::LogMode::WARNING:
                    modeString = "[WRANING]";
                    break;
                case Logger::LogMode::ERROR:
                    modeString = "[ERROR]";
                    break;
                case Logger::LogMode::CRITICAL:
                    modeString = "[CRITICAL]";
                    break;
                case Logger::LogMode::DEBUG:
                    modeString = "[DEBUG]";
                    break;
                default:
                    break;
                }

                std::cout << generateTimeFormatString() << modeString;
                ((std::cout << args << ' '), ...);
                std::cout << '\n';

                m_file << generateTimeFormatString() << modeString;
                ((m_file << args << ' '), ...);
                m_file << std::endl;
                });
            m_cv.notify_all();
        }

    protected:
        static std::string generateFileName()
        {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "%Y-%m-%d.log");
            return ss.str();
        }

        static std::string generateTimeFormatString()
        {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "[%H:%M:%S]");
            return ss.str();
        }

        bool openFile()
        {
            std::filesystem::create_directory("./logs");
            m_file.open("./logs/" + generateFileName(), std::ios_base::app);
            return static_cast<bool>(m_file);
        }

        void workFunction()
        {
            while (m_isRunning)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [&]() {return !m_msgQueue.empty() || !m_isRunning; });
                if (!m_isRunning && m_msgQueue.empty())
                    return;
                else if (m_msgQueue.empty())
                    continue;
                std::function<void()> getFunc = std::move(m_msgQueue.front());
                m_msgQueue.pop();
                getFunc();
            }
        }

    private:
        std::ofstream                        m_file;
        std::condition_variable                m_cv;
        std::mutex                            m_mutex;
        std::atomic<bool>                    m_isRunning;
        std::queue<std::function<void()>>    m_msgQueue;
        std::thread                            m_thread;

        static std::atomic<bool>            m_hasCreate;
    };

    std::atomic<bool> Logger::m_hasCreate(false);
}

#endif // !LOGGER_HPP
