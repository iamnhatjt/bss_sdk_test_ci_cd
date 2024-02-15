//
// Created by vnbk on 31/05/2023.
//

#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>

#include "Logger.h"
#include "FileUtils.h"

#define LOG_FILE_FORMAT   "Log-%s.txt"
#define LOG_FOLDER         "Logs"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define LOG_BUFFER_SIZE 2048

static const char *g_log_level_colors[Logger::LOG_LEVEL_NUMBER] = {ANSI_COLOR_RED,ANSI_COLOR_YELLOW,ANSI_COLOR_GREEN,ANSI_COLOR_CYAN, ANSI_COLOR_RESET };

static const char *g_log_level_names[Logger::LOG_LEVEL_NUMBER] = {"ERROR","WARN ", "INFO ", "DEBUG", "TRACE" };

Logger *Logger::m_instance = nullptr;

//Logger::LOG_LEVEL Logger::m_level = LOG_LEVEL_DEBUG;

Logger::Logger(LOG_LEVEL _level, logger_puts_fn_t _log_put_fn_t, bool _isSave) : m_isSave(_isSave), m_shutdown(false), m_level(_level){
    m_path = getCurrentDir() + '/' + LOG_FOLDER + '/';
    if(!checkFileExist(getCurrentDir(), LOG_FOLDER)){
        createDir(m_path);
    }

    char fileName[FILENAME_MAX] = {0};
    DateTime *currentTime = DateTime::getCurrentDateTime();
    sprintf(fileName, LOG_FILE_FORMAT, currentTime->convertDatetimeToString(DDMMYYYY).c_str());
    m_currentLogFile = fileName;

    if(!checkExistedLogFile()){
        createNewLogFile();
    }

    m_thread = std::thread(&Logger::run, this);

    m_logPutFns.push_back(_log_put_fn_t);

    m_instance = this;
}

Logger::~Logger() noexcept {
    std::lock_guard<std::mutex> lock(m_lock);
    while (!m_logs.empty()){
        auto item = m_logs.front();
        m_logs.pop();
        this->saveLog(item);
    }
}

void Logger::setLogLevel(Logger::LOG_LEVEL _level) {
    m_level = _level;
}

Logger::LOG_LEVEL Logger::getLogLevel() {
    return m_level;
}

void Logger::setLogPutFn(logger_puts_fn_t _log_fn) {
    if(_log_fn){
        m_logPutFns.push_back(_log_fn);
    }
}

void Logger::writeLog(const DateTime& _datetime, const std::string& _log) {
    m_lock.lock();
    std::string log = buildLog(_datetime, _log);
    m_logs.push(log);
    m_lock.unlock();
    m_trigger.notify_all();
}

void Logger::show(LOG_LEVEL _level, const char *_tag, const char* _format, ...) {
    if(!m_instance){
        m_instance = new Logger();
    }

    va_list ap;
    va_start(ap, _format);
    char str[LOG_BUFFER_SIZE];

    if(_level > m_instance->m_level){
        return;
    }

    int len = snprintf(str, LOG_BUFFER_SIZE, "%s#%s: %s: ",g_log_level_colors[_level], g_log_level_names[_level], _tag);
    len += vsnprintf(str + len, LOG_BUFFER_SIZE - len, _format, ap);
    va_end(ap);

    if(len >= LOG_BUFFER_SIZE){
        len = LOG_BUFFER_SIZE;
        str[len - 1] = '\0';
    }else{
        str[len] = '\0';
    }

//    printf("%s\n", str);

    if(!m_instance){
        m_instance = new Logger();
    }

    m_instance->writeLog(*DateTime::getCurrentDateTime(), str);
}

void Logger::run() {
    while(!m_shutdown){
        std::unique_lock<std::mutex> lock(m_lock);
        m_trigger.wait(lock, [&](){return !m_logs.empty();});

        auto item = m_logs.front();
        m_logs.pop();

        lock.unlock();

        for(auto & logPutFn : this->m_logPutFns){
            if(logPutFn){
                logPutFn(item.c_str());
            }
        }

        if(m_instance->m_isSave){
            this->saveLog(item);
        }
    }
}

std::string Logger::buildLog(const DateTime &_datetime, const std::string &_log) {
    std::string log = _datetime.convertDatetimeToString(DATE_TIME_TYPE::DD_MM_YYYY_hh_mm_ss);
    log += ":\t";
    log += _log;
    return log;
}

void Logger::saveLog(const std::string &_log) {
    char fileName[FILENAME_MAX];
    DateTime* currentTime = DateTime::getCurrentDateTime();
    sprintf(fileName, LOG_FILE_FORMAT, currentTime->convertDatetimeToString(DDMMYYYY).c_str());

    if(m_currentLogFile != fileName){
        m_currentLogFile = fileName;
        if(!createNewLogFile()){
            return;
        }
    }

    std::ofstream logFile(m_path + m_currentLogFile, std::ios::app);
    if(logFile.is_open()){
        logFile << _log << std::endl;
        logFile.close();
    }
}


bool Logger::checkExistedLogFile() {
    std::ifstream logFile(m_path + m_currentLogFile);
    if(logFile.good()){
        return true;
    }else{
        return false;
    }
}

bool Logger::createNewLogFile() {
    std::ofstream logFile(m_path + m_currentLogFile);
    if(logFile.is_open()){
        logFile.close();
        return true;
    }
    return false;
}