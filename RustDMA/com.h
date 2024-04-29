#pragma once
#include <string>
#include <array>
#include <cassert>
#include <windows.h>
#include <memory>
#include <mutex>

class BaseCom
{
protected:
    int _port = 0;  // Serial port number
    HANDLE _com_handle = INVALID_HANDLE_VALUE; // Serial port handle
    std::wstring _com_str;
    DCB _dcb{}; // Baud rate, stop bits, etc.
    COMMTIMEOUTS _co{}; // Timeout times

    virtual bool open_port() = 0;

    void init()
    {
        _com_str.clear();
        _com_handle = INVALID_HANDLE_VALUE;
    }

    virtual bool setup_port()
    {
        if (_com_handle == INVALID_HANDLE_VALUE)
            return false;

        if (!SetupComm(_com_handle, 8192, 8192))
            return false; // Set recommended buffer size

        if (!GetCommTimeouts(_com_handle, &_co))
            return false;

        // Settings for timeouts
        _co.ReadIntervalTimeout = 0xFFFFFFFF;
        _co.ReadTotalTimeoutMultiplier = 0;
        _co.ReadTotalTimeoutConstant = 0;
        _co.WriteTotalTimeoutMultiplier = 0;
        _co.WriteTotalTimeoutConstant = 2000;

        if (!SetCommTimeouts(_com_handle, &_co))
            return false; // Set timeout times

        if (!PurgeComm(_com_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
            return false; // Clear serial port buffer

        return true;
    }

    void set_com_port(int port)
    {
        _port = port;
        _com_str = L"\\\\.\\COM" + std::to_wstring(_port);
    }

public:
    BaseCom() = default;
    virtual ~BaseCom() { close(); }

    virtual bool open(int port)
    {
        if (is_open())
            close();

        set_com_port(port);
        if (!open_port())
            return false;

        return setup_port();
    }

    virtual bool is_open() const
    {
        return _com_handle != INVALID_HANDLE_VALUE;
    }

    virtual void close()
    {
        if (is_open())
        {
            CloseHandle(_com_handle);
            _com_handle = INVALID_HANDLE_VALUE;
        }
    }

    virtual bool send_data(const std::string& data) = 0;
    virtual bool receive_data(std::string& data, size_t size) = 0;
};

class ComPort : public BaseCom
{
protected:
    bool open_port() override
    {
        _com_handle = CreateFile(
            _com_str.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        return _com_handle != INVALID_HANDLE_VALUE;
    }

public:
    ComPort() = default;

    bool send_data(const std::string& data) override
    {
        if (!is_open())
            return false;

        DWORD bytes_written;
        return WriteFile(_com_handle, data.c_str(), data.size(), &bytes_written, nullptr) && bytes_written == data.size();
    }

    bool receive_data(std::string& data, size_t size) override
    {
        if (!is_open() || size == 0)
            return false;

        std::array<char, 8192> buffer;
        assert(size <= buffer.size()); // Ensure we don't overflow the buffer

        DWORD bytes_read;
        if (!ReadFile(_com_handle, buffer.data(), size, &bytes_read, nullptr))
            return false;

        data.assign(buffer.data(), bytes_read);
        return true;
    }
};

class BaseDevice
{
public:
    virtual ~BaseDevice() = default;

    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool send_data(const std::string& data) = 0;
    virtual bool receive_data(std::string& data, size_t size) = 0;
};

class ComDevice : public BaseDevice
{
private:
    std::unique_ptr<BaseCom> _com;
    int _port;

public:
    explicit ComDevice(int port, std::unique_ptr<BaseCom> com)
        : _port(port), _com(std::move(com)) {}

    bool connect() override
    {
        return _com->open(_port);
    }

    bool disconnect() override
    {
        _com->close();
        return !_com->is_open();
    }

    bool send_data(const std::string& data) override
    {
        return _com->send_data(data);
    }

    bool receive_data(std::string& data, size_t size) override
    {
        return _com->receive_data(data, size);
    }
};

class AsyncSender
{
private:
    ComPort& _comPort;
    std::string _latestCmd;
    std::mutex _cmdMutex;
    std::condition_variable _cmdCondition;
    std::atomic<bool> _hasNewCmd = false;
    std::atomic<bool> _running = true;
    std::thread _senderThread;

    void senderThreadFunction()
    {
        while (_running)
        {
            std::unique_lock<std::mutex> lock(_cmdMutex);
            _cmdCondition.wait(lock, [this] { return _hasNewCmd || !_running; });

            if (!_running) return;

            std::string cmdToSend = _latestCmd; // Grab the latest command
            _hasNewCmd = false; // Reset the flag

            lock.unlock();

            _comPort.send_data(cmdToSend); // Send the command
        }
    }

public:
    explicit AsyncSender(ComPort& comPort) : _comPort(comPort)
    {
        _senderThread = std::thread(&AsyncSender::senderThreadFunction, this);
    }

    ~AsyncSender()
    {
        _running = false;
        _cmdCondition.notify_all(); // Wake up the thread
        _senderThread.join(); // Wait for it to finish
    }

    void sendAsync(const std::string& cmd)
    {
        std::lock_guard<std::mutex> lock(_cmdMutex);
        _latestCmd = cmd;
        _hasNewCmd = true;
        _cmdCondition.notify_one(); // Notify the sender thread
    }
};