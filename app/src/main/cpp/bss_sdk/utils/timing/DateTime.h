//
// Created by vnbk on 31/05/2023.
//

#ifndef BSS_BASE_DATETIME_H
#define BSS_BASE_DATETIME_H

#include <string>
#include <ctime>

typedef enum {
    YYYYMMDDhhmmss,
    DDMMYYYYhhmmss,
    hhmmssDDMMYYYY,
    hhmmssYYYYMMDD,
    DD_MM_YYYY_hh_mm_ss,
    DDMMYYYY,
    YYYYMMDD
}DATE_TIME_TYPE;

class DateTime {
public:
    static DateTime m_currentDateTime;
    static DateTime* getCurrentDateTime();
    static long int getcurrentTimeStamp();
    static void convertTimeStampToDateTime(long int _timestamp, DateTime& _datetime);
    static long int convertDateTimeToTimeStamp(const DateTime& _datetime);
    static void convertStringToDateTime(DateTime& _datetime, const char* strDateTime, DATE_TIME_TYPE _format);

    DateTime& operator=(const DateTime& _datetime);
    DateTime& operator=(const DateTime* _datetime);
    DateTime operator+(int second);
    DateTime operator-(int second);
    DateTime& operator+=(int second);
    DateTime& operator-=(int second);
    bool operator<=(const DateTime& _datetime);
    bool operator<(const DateTime& _datetime);
    bool operator==(const DateTime& _datetime);
    bool operator>=(const DateTime& _datetime);
    bool operator>(const DateTime& _datetime);

    std::string convertDatetimeToString(DATE_TIME_TYPE _type) const;
    DateTime& convertStringToDateTime(const char* strDateTime, DATE_TIME_TYPE _format);
    DateTime& convertToDateTime(long int _timestamp);
    long int  convertToTimeStamp() const;

    int m_year;
    int m_month;
    int m_day;
    int m_hour;
    int m_min;
    int m_second;
    int m_timezone;

    DateTime();

private:
    DateTime& operator=(const std::tm* datetimeSystem);
    void convertToSystemTime(std::tm* datetimeSystem) const;
};

inline std::string convertDateTimeTypeToString(DATE_TIME_TYPE _type) {
    switch (_type) {
        case YYYYMMDDhhmmss:
            return "%Y%m%d%H%M%S";
        case DDMMYYYYhhmmss:
            return "%d%m%Y%H%M%S";
        case hhmmssYYYYMMDD:
            return "%H%M%S%Y%m%d";
        case hhmmssDDMMYYYY:
            return "%H%M%S%d%m%Y";
        case DD_MM_YYYY_hh_mm_ss:
            return "%d-%m-%Y:%H-%M-%S";
        case DDMMYYYY:
            return "%d%m%Y";
        case YYYYMMDD:
            return "%Y%m%d";
        default:
            return "";
    }
}

#endif //BSS_BASE_DATETIME_H
