//
// Created by vnbk on 31/05/2023.
//

#include "DateTime.h"
#ifdef WIN32
#include <iomanip>
#include <sstream>
static char* strptime(const char* _stringDateTime, const char* _format, std::tm* _systime){
    std::istringstream input(_stringDateTime);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(_systime, _format);
    if (input.fail()) {
        return nullptr;
    }
    return (char*)(_stringDateTime + input.tellg());
}
#endif

DateTime DateTime::m_currentDateTime;

DateTime::DateTime() {
    m_day = 0;
    m_hour = 0;
    m_min = 0;
    m_month = 0;
    m_second = 0;
    m_year = 0;
    m_timezone = 0;
}

DateTime& DateTime::operator=(const DateTime& dateTime){
    m_year = dateTime.m_year;
    m_month = dateTime.m_month;
    m_day = dateTime.m_day;
    m_hour = dateTime.m_hour;
    m_min = dateTime.m_min;
    m_second = dateTime.m_second;
    m_timezone = dateTime.m_timezone;
    return *this;
}

DateTime &DateTime::operator=(const DateTime *_datetime) {
    m_year = _datetime->m_year;
    m_month = _datetime->m_month;
    m_day = _datetime->m_day;
    m_hour = _datetime->m_hour;
    m_min = _datetime->m_min;
    m_second = _datetime->m_second;
    m_timezone = _datetime->m_timezone;
    return *this;
}

DateTime DateTime::operator+(int second) {
    long int timestamp = convertToTimeStamp() + second;
    DateTime tmp;
    return tmp.convertToDateTime(timestamp);
}

DateTime DateTime::operator-(int second) {
    long int timestamp = convertToTimeStamp() - second;
    DateTime tmp;
    return tmp.convertToDateTime(timestamp);
}

DateTime & DateTime::operator+=(int second) {
    long int timestamp = convertToTimeStamp() + second;
    return convertToDateTime(timestamp);
}

DateTime & DateTime::operator-=(int second) {
    long int timestamp = convertToTimeStamp() - second;
    return convertToDateTime(timestamp);
}

bool DateTime::operator<(const DateTime &_datetime){
    return this->convertToTimeStamp() < _datetime.convertDateTimeToTimeStamp(_datetime);
}

bool DateTime::operator<=(const DateTime &_datetime) {
    return this->convertToTimeStamp() <= _datetime.convertDateTimeToTimeStamp(_datetime);
}

bool DateTime::operator==(const DateTime &_datetime) {
    return this->convertToTimeStamp() == _datetime.convertDateTimeToTimeStamp(_datetime);
}

bool DateTime::operator>(const DateTime &_datetime) {
    return this->convertToTimeStamp() > _datetime.convertDateTimeToTimeStamp(_datetime);
}

bool DateTime::operator>=(const DateTime &_datetime) {
    return this->convertToTimeStamp() >= _datetime.convertDateTimeToTimeStamp(_datetime);
}

std::string DateTime::convertDatetimeToString(DATE_TIME_TYPE _type) const{
    std::tm datetimeSys = {0,};
    convertToSystemTime(&datetimeSys);
    char buffer[80] = {0,};
    switch (_type) {
        case YYYYMMDDhhmmss:
            strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &datetimeSys);
            break;
        case DDMMYYYYhhmmss:
            strftime(buffer, sizeof(buffer), "%d%m%Y%H%M%S", &datetimeSys);
            break;
        case hhmmssYYYYMMDD:
            strftime(buffer, sizeof(buffer), "%H%M%S%Y%m%d", &datetimeSys);
            break;
        case hhmmssDDMMYYYY:
            strftime(buffer, sizeof(buffer), "%H%M%S%d%m%Y", &datetimeSys);
            break;
        case DD_MM_YYYY_hh_mm_ss:
            strftime(buffer, sizeof(buffer), "%d-%m-%Y:%H-%M-%S", &datetimeSys);
            break;
        case DDMMYYYY:
            strftime(buffer, sizeof(buffer), "%d%m%Y", &datetimeSys);
            break;
        case YYYYMMDD:
            strftime(buffer, sizeof(buffer), "%Y%m%d", &datetimeSys);
            break;
        default:
            break;
    }
    std::string str(buffer);
    return str;
}

void DateTime::convertStringToDateTime(DateTime& dateTime, const char *strDateTime, DATE_TIME_TYPE _format) {
    std::tm datetimeSys;
    std::string format = convertDateTimeTypeToString(_format);
    strptime(strDateTime, format.c_str(), &datetimeSys);
    dateTime = &datetimeSys;
}

DateTime& DateTime::convertStringToDateTime(const char* strDateTime, DATE_TIME_TYPE _format){
    std::tm datetimeSys;
    std::string format = convertDateTimeTypeToString(_format);
    strptime(strDateTime, format.c_str(), &datetimeSys);

    *this = &datetimeSys;
    return *this;
}

DateTime & DateTime::convertToDateTime(long int _timestamp) {
    const time_t rawtime = (const time_t)_timestamp;
    tm* pTm = localtime(&rawtime);
    *this = pTm;
    return *this;
}

long int DateTime::convertToTimeStamp() const {
    std::tm systemTime = {0,};
    convertToSystemTime(&systemTime);
    return mktime(&systemTime);
}

DateTime* DateTime::getCurrentDateTime(){
    time_t now = time(0);
    tm *currentTime = localtime(&now);
    DateTime::m_currentDateTime = currentTime;

    return &DateTime::m_currentDateTime;
}

long int DateTime::getcurrentTimeStamp(){
    return time(0);
}

void DateTime::convertTimeStampToDateTime(long int _timestamp, DateTime& dateTime){
    const time_t rawtime = (const time_t)_timestamp;
    tm* pTm = localtime(&rawtime);
    dateTime = pTm;
}

long int DateTime::convertDateTimeToTimeStamp(const DateTime &_datetime) {
    std::tm systemTime = {0,};
    _datetime.convertToSystemTime(&systemTime);
    return mktime(&systemTime);
}


DateTime& DateTime::operator=(const std::tm* dateTimeSystem){
    m_year = dateTimeSystem->tm_year + 1900;
    m_month = dateTimeSystem->tm_mon + 1;
    m_day = dateTimeSystem->tm_mday;
    m_hour = dateTimeSystem->tm_hour;
    m_min = dateTimeSystem->tm_min;
    m_second = dateTimeSystem->tm_sec;
    return *this;
}

void DateTime::convertToSystemTime(std::tm* datetimeSystem) const{
    datetimeSystem->tm_year = m_year - 1900;
    datetimeSystem->tm_mon = m_month - 1;
    datetimeSystem->tm_mday = m_day;
    datetimeSystem->tm_hour = m_hour;
    datetimeSystem->tm_min = m_min;
    datetimeSystem->tm_sec = m_second;
}