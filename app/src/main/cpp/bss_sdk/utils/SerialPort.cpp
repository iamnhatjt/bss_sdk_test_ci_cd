//
// Created by vnbk on 13/09/2023.
//

#include "SerialPort.h"
#include "Logger.h"

#include "linux_serial.h"

#define TAG "SerialPort"

SerialPort::SerialPort(const std::string &_devFile, int32_t _baudRate, bool _isBlocking) {
    m_devFile = _devFile;
    m_baudRate = _baudRate;
    m_blocking = _isBlocking;
    m_fd = -1;
}

SerialPort::~SerialPort() {
    if(m_fd){
        serial_free(m_fd);
    }

    m_fd = -1;
    m_blocking = false;
    m_baudRate = -1;
    m_devFile.clear();
}

int32_t SerialPort::open(int32_t _baudRate) {
    if(m_devFile.empty() || _baudRate < 0){
        LOG_ERROR(TAG, "Serial Port open FAILURE, Parameters INVALID");
        return -1;
    }
    m_baudRate = _baudRate;

    m_fd = serial_init(m_devFile.c_str(), m_baudRate, m_blocking);

    LOG_INFO(TAG, "Open SerialPort %s %s, fd: %d", m_devFile.c_str(), m_fd > 0 ? "SUCCESS" : "FAILURE", m_fd);
    return (m_fd > 0) ? 0 : -1;
}

int32_t SerialPort::close() const {
    return serial_free(m_fd);
}

bool SerialPort::isOpen() const {
    return serial_is_open(m_fd);
}

int32_t SerialPort::config(int32_t _baudRate) {
    if(m_fd < 0){
        LOG_ERROR(TAG, "Port is NOT open");
        return -1;
    }
    int32_t ret = serial_config(m_fd, _baudRate);
    if(ret < 0){
        return -1;
    }
    m_baudRate = _baudRate;
    return ret;
}

int32_t SerialPort::getBaudRate() const {
    return m_baudRate;
}

int32_t SerialPort::dataIsAvailable() const {
    return serial_data_available(m_fd);
}

void SerialPort::flush() const {
    serial_flush(m_fd);
}

int32_t SerialPort::send(const uint8_t *_data, int32_t _len, int32_t _timeout) const {
    (int32_t)_timeout;
    return serial_send_bytes(m_fd, _data, _len);
}

int32_t SerialPort::receive(uint8_t *_data, int32_t _maxLen) const {
    return serial_recv_bytes(m_fd, _data, _maxLen);
}

int32_t SerialPort::receive(uint8_t *_data, int32_t _maxLen, int32_t _timeout) const {
    int32_t len = serial_recv_bytesz(m_fd, _data, _maxLen, _timeout);
    return len;
}