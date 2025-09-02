#pragma once

#include <string>
#include <chrono>
#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

#include "byte_tools.h"


class SocketRAII {
    public:
        SocketRAII() : sock_(-1) {}

        explicit SocketRAII(int sock) : sock_(sock) {}

        void Close() {
            if (sock_ != -1) {
                close(sock_);
                sock_ = -1;
            }
        }

        ~SocketRAII() {
            Close();
        }
    
        SocketRAII(const SocketRAII&) = delete;
        SocketRAII& operator=(const SocketRAII&) = delete;
    
        SocketRAII(SocketRAII&& other) noexcept : sock_(other.sock_) {
            other.sock_ = -1;
        }
    
        SocketRAII& operator=(SocketRAII&& other) noexcept {
            if (this != &other) {
                Close();
                sock_ = other.sock_;
                other.sock_ = -1;
            }
            return *this;
        }
    
        void Reset(int newSock) {
            Close();
            sock_ = newSock;
        }

        int Get() const { return sock_; }
    
        int Release() {
            int temp = sock_;
            sock_ = -1;
            return temp;
        }
    
    private:
        int sock_;
    };

/*
 * Обертка над низкоуровневой структурой сокета.
 */
class TcpConnect {
public:
    TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout)
    : ip_(std::move(ip)), port_(port), connectTimeout_(connectTimeout), readTimeout_(readTimeout), rawSock_(-1), sock_(-1) {}

    ~TcpConnect() {
        sock_.Close();
    };

    /*
     * Установить tcp соединение.
     * Если соединение занимает более `connectTimeout` времени, то прервать подключение и выбросить исключение.
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man7/socket.7.html
     * - https://man7.org/linux/man-pages/man2/connect.2.html
     * - https://man7.org/linux/man-pages/man2/fcntl.2.html (чтобы включить неблокирующий режим работы операций)
     * - https://man7.org/linux/man-pages/man2/select.2.html
     * - https://man7.org/linux/man-pages/man2/setsockopt.2.html
     * - https://man7.org/linux/man-pages/man2/close.2.html
     * - https://man7.org/linux/man-pages/man3/errno.3.html
     * - https://man7.org/linux/man-pages/man3/strerror.3.html
     */
    void EstablishConnection(){
        int rawSock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (rawSock_ == -1) {
            throw std::runtime_error(std::string(strerror(errno)));
        }
        sock_.Reset(rawSock_);

        int flags = fcntl(sock_.Get(), F_GETFL, 0);  // old flags of socket
        if (flags == -1 || fcntl(sock_.Get(), F_SETFL, flags | O_NONBLOCK) == -1) { // non-block status
            throw std::runtime_error(std::string(strerror(errno)));
        }   

        sockaddr_in addr {}; // give ip
        addr.sin_family = AF_INET; //IPv4
        addr.sin_port = htons(port_); // mahcnine to bigEndian

        if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) <= 0) { // from str to real ip
            throw std::runtime_error("Invalid IP address");
        }

        int result = connect(sock_.Get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        // reinterpret_cast<sockaddr*>(&addr) bcs sockaddr_in need to be sockaddr that can work with IPv4/6
        if (result == 0) {
            // connected
        } else if (errno == EINPROGRESS) { // wait of server
            fd_set setOfFileDes; // set of file descryptors 
            FD_ZERO(&setOfFileDes); // make {}
            FD_SET(sock_.Get(), &setOfFileDes); // set our listening socket

            timeval timeToConn;
            timeToConn.tv_sec = connectTimeout_.count() / 1000;
            timeToConn.tv_usec = (connectTimeout_.count() % 1000) * 1000;

            int selection = select(sock_.Get() + 1, nullptr, &setOfFileDes, nullptr, &timeToConn); // select do fd on [0, n)
            if (selection == 0) {
                throw std::runtime_error("Connection time out");
            } else if (selection < 0) {
                throw std::runtime_error(std::string(strerror(errno)));
            }

            int error_code = 0;
            socklen_t len = sizeof(error_code);
            if (getsockopt(sock_.Get(), SOL_SOCKET, SO_ERROR, &error_code, &len) < 0 || error_code != 0) { // getseckopt is UNIX lowlvl call for check of errors on socket last operation
                throw std::runtime_error("Connect failed: " + std::string(strerror(error_code)));
            }
        } else {
            throw std::runtime_error("Connect failed: " + std::string(strerror(errno)));
        }

        if (fcntl(sock_.Get(), F_SETFL, flags) == -1) { // block status of sock
            throw std::runtime_error(std::string(strerror(errno)));
        }
    }

    /*
     * Послать данные в сокет
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/send.2.html
     */
    void SendData(const std::string& data) const{
        size_t bytesSent = send(sock_.Get(), data.c_str(), data.size(), 0);
        if (bytesSent < 0) {
            throw std::runtime_error("Send failed: " + std::string(strerror(errno)));
        }
    }

    /*
     * Прочитать данные из сокета.
     * Если передан `bufferSize`, то прочитать `bufferSize` байт.
     * Если параметр `bufferSize` не передан, то сначала прочитать 4 байта, а затем прочитать количество байт, равное
     * прочитанному значению.
     * Первые 4 байта (в которых хранится длина сообщения) интерпретируются как целое число в формате big endian,
     * см https://wiki.theory.org/BitTorrentSpecification#Data_Types
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/poll.2.html
     * - https://man7.org/linux/man-pages/man2/recv.2.html
     */
    std::string ReceiveData(size_t bufferSize = 0) const{
        if (bufferSize == 0) {
            char lengthBytes[4];
            ssize_t bytesRead = recv(sock_.Get(), lengthBytes, sizeof(lengthBytes), 0);
            if (bytesRead < 0) {
                throw std::runtime_error(std::string(strerror(errno)));
            }
            uint32_t messageLength = BytesToInt(std::string_view(lengthBytes, 4));
            bufferSize = messageLength;
        }

        std::string data(bufferSize, '\0');
        ssize_t bytesRead = recv(sock_.Get(), &data[0], bufferSize, 0);
        if (bytesRead < 0) {
            throw std::runtime_error(std::string(strerror(errno)));
        }

        return data;
    }


    void CloseConnection(){
        if (sock_.Get() != -1) {
            if (close(sock_.Get()) < 0) {
                throw std::runtime_error("Close failed: " + std::string(strerror(errno)));
            }
            sock_.Release();
        }
    }

    const std::string& GetIp() const{
        return ip_;
    }

    int GetPort() const{
        return port_;
    }
    
private:
    const std::string ip_;
    const int port_;
    std::chrono::milliseconds connectTimeout_, readTimeout_;
    int rawSock_;
    SocketRAII sock_;
};
