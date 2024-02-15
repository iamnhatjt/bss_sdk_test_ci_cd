//
// Created by vnbk on 31/05/2023.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>

#include "DateTime.h"

typedef void (*logger_puts_fn_t)(const char*);

#define LOG_TRACE(tag, log, ...)      Logger::show(Logger::LOG_LEVEL_TRACE, tag, log,  ##__VA_ARGS__)
#define LOG_DEBUG(tag, log, ...)      Logger::show(Logger::LOG_LEVEL_DEBUG, tag, log,  ##__VA_ARGS__)
#define LOG_INFO(tag,  log, ...)      Logger::show(Logger::LOG_LEVEL_INFO, tag, log,  ##__VA_ARGS__)
#define LOG_WARN(tag,  log, ...)      Logger::show(Logger::LOG_LEVEL_WARN, tag, log,  ##__VA_ARGS__)
#define LOG_ERROR(tag, log, ...)      Logger::show(Logger::LOG_LEVEL_ERROR, tag, log,  ##__VA_ARGS__)

class Logger{
public:
    enum LOG_LEVEL{
        LOG_LEVEL_ERROR,
        LOG_LEVEL_WARN,
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_TRACE,
        LOG_LEVEL_NUMBER
    };

    explicit Logger(LOG_LEVEL _level = LOG_LEVEL_DEBUG, logger_puts_fn_t _log_put_fn_t = nullptr, bool _isSave = false);
    ~Logger();

    void writeLog(const DateTime& _datetime, const std::string& _log);

    static void show(LOG_LEVEL _level, const char* _tag, const char* format, ...);

    void setLogLevel(LOG_LEVEL _level);

    LOG_LEVEL getLogLevel();

    void setLogPutFn(logger_puts_fn_t _log_fn);

private:
    std::string buildLog(const DateTime& _datetime, const std::string& _log);

    void saveLog(const std::string& _log);

    bool checkExistedLogFile();

    bool createNewLogFile();

    void run();

    static Logger *m_instance;

    std::queue<std::string> m_logs;
    bool m_isSave;

    std::mutex m_lock;
    std::condition_variable m_trigger;
    std::thread m_thread;
    std::atomic<bool> m_shutdown;

    std::string m_path;
    std::string m_currentLogFile;

    std::vector<logger_puts_fn_t> m_logPutFns;

    LOG_LEVEL m_level;
};

#endif //LOGGER_H
