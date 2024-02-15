//
// Created by vnbk on 13/09/2023.
//

#ifndef BSS_SDK_SERIALPORT_H
#define BSS_SDK_SERIALPORT_H

#include <cstdint>
#include <string>

class SerialPort {
public:
    explicit SerialPort(const std::string& _devFile, int32_t _baudRate = 115200, bool _isBlocking = false);

    ~SerialPort();

    int32_t open(int32_t _baudRate);

    int32_t close() const;

    bool isOpen() const;

    int32_t config(int32_t _baudRate);

    int32_t getBaudRate() const;

    int32_t dataIsAvailable() const;

    void flush() const;

    int32_t send(const uint8_t* _data, int32_t _len, int32_t _timeout) const;

    int32_t receive(uint8_t* _data, int32_t _maxLen) const;

    int32_t receive(uint8_t* _data, int32_t _maxLen, int32_t _timeout) const;

private:
    int32_t m_fd;
    std::string m_devFile;
    bool m_blocking;
    int32_t m_baudRate;
};


#endif //BSS_SDK_SERIALPORT_H
