/*
 * Kuumba C++ Core
 *
 * $Id: Socket.cpp 23304 2008-04-28 15:13:12Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   if !defined(MSG_NOSIGNAL)
#       define MSG_NOSIGNAL 0        
#   endif
    typedef int socklen_t;
    static struct WinSockets
    {
        WinSockets()  { WSADATA wsa = {0}; WSAStartup(MAKEWORD(2,0), &wsa); }
        ~WinSockets() { WSACleanup(); }
    } k_winSockets;
#   define KCC_SOCKET_ERRNO ::WSAGetLastError()    
#elif defined(KCC_LINUX)
#   include "errno.h"
#   include "unistd.h"
#   include "sys/types.h"
#   include "sys/socket.h"
#   include "netdb.h"
#   include "netinet/in.h"
#   define KCC_SOCKET_ERRNO errno
#endif

#define KCC_FILE "Socket"

namespace kcc
{
    // buf size for reading string
    static const int    k_szBuf    = 1024;         // 1K read buffer
    static const int    k_szBufRes = k_szBuf*10;   // 10K reserve
    static const int    k_szBufMax = k_szBuf*1024; // 1MB max
    static const String k_notConntected("socket handle not valid (has the socket been connected or listened?)"); 

    // k_ip: fetch ip address from sockaddr
    static inline String k_ip(const struct ::sockaddr_in& addr)
    {
        unsigned long  a = ntohl(addr.sin_addr.s_addr);
        unsigned long a1 = (a & 0xff000000) >> 24;
        unsigned long a2 = (a & 0x00ff0000) >> 16;
        unsigned long a3 = (a & 0x0000ff00) >> 8;
        unsigned long a4 = (a & 0x000000ff);
        return Strings::printf("%d.%d.%d.%d", a1, a2, a3, a4);
    }
    
    // k_option: map to BSD socket option
    static inline int k_option(Socket::OptionFlags o) throw (Socket::Failed)
    {
        switch (o)
        {
            case Socket::O_KEEPALIVE: return SO_KEEPALIVE;
            case Socket::O_LINGER:    return SO_LINGER;
            case Socket::O_SNDBUF:    return SO_SNDBUF;
            case Socket::O_RCVBUF:    return SO_RCVBUF;
        };
        throw Socket::Failed("option not implemented");
    }
    
    // k_timeout: map to BSD socket option
    static inline int k_timeout(Socket::TimeoutFlags t) throw (Socket::Failed)
    {
        switch (t)
        {
            case Socket::T_RECEIVE: return SO_RCVTIMEO;
            case Socket::T_SEND:    return SO_SNDTIMEO;
        };
        throw Socket::Failed("option not implemented");
    }
    
    // k_message: create socket message
    static inline String k_message(Socket* s, Socket::Handle h, const Char* msg) 
    {
        return Strings::printf("%s: host=[%s] addr=[%s:%d] handle=[%d] errno=[%d]", msg, s->host().c_str(), s->ip().c_str(), s->port(), h, KCC_SOCKET_ERRNO);
    }

    //
    // Socket implementation
    //

    // Socket: create or attach to socket
    Socket::Socket() : m_port(0), m_handle(-1)
    {}
    Socket::Socket(const String& host, int port) : m_host(host), m_port(port), m_handle(-1)
    {}
    Socket::Socket(Handle handle) throw (Socket::Failed) : m_port(0), m_handle(handle)
    {
        Log::Scope scope(KCC_FILE, "Socket::Socket");
        if (handle < 0) throw Socket::Failed(k_notConntected);
        struct ::sockaddr_in addr = {0};
        ::socklen_t sz = sizeof(addr);
        if (::getpeername(m_handle, (struct sockaddr*)&addr, &sz) < 0)
            throw Socket::Failed(Strings::printf("socket attach failed: handle=[%d]", handle));
        m_port = ntohs(addr.sin_port);
        m_host = m_ip = k_ip(addr);
    }

    // ~Socket: close socket
    Socket::~Socket() { close(); }
    
    // connect: connect to socket
    void Socket::connect(const String& host, int port) throw (Socket::Failed)
    {
        m_host = host;
        m_port = port;
        connect();
    }

    // connect: connect to socket
    void Socket::connect() throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "connect");
        if (m_handle >= 0) throw Socket::Failed(k_message(this, m_handle, "attempt to connect to socket twice"));

        // socket handle
        m_handle = ::socket(PF_INET, SOCK_STREAM, 0);
        if (m_handle < 0) throw Socket::Failed(k_message(this, m_handle, "open socket failed"));

        // socket address
        String port(Strings::printf("%d", m_port));
        struct addrinfo hints = {0};
        struct addrinfo *ai = NULL;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_flags    = AI_PASSIVE|AI_CANONNAME;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        if (::getaddrinfo(m_host.c_str(), port.c_str(), &hints, &ai) != 0) 
            throw Socket::Failed(k_message(this, m_handle, "unable to get host addr info"));
        m_ip = ai->ai_canonname;

        // connect to socket
        onConnect(ai->ai_addr);
        freeaddrinfo(ai);
    }

    // close: close socket
    void Socket::close()
    {
        if (m_handle >= 0)
        {
            Log::Scope scope(KCC_FILE, "close");
            #if defined(KCC_WINDOWS)
                ::shutdown(m_handle, 2);
                char c = 0;
                while (::recv(m_handle, &c, sizeof(char), 0) > 0);
                ::closesocket(m_handle);
            #elif defined(KCC_LINUX)
                ::shutdown(m_handle, 2);
                ::close(m_handle);
            #endif
            m_handle = -1;
        }
    }

    // read: read a long
    void Socket::read(long& l) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "read");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        int  actual = 0;
        long in     = 0L;
        read((char*)&in, sizeof(long), actual);
        if (actual > 0) l = ntohl(in);
    }

    // read: read into buffer
    void Socket::read(char* buf, int sz, int& actual) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "read");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        actual = ::recv(m_handle, buf, sz, 0);
        if (actual < 0) throw Socket::Failed("read failed");
    }

    // write: write long
    void Socket::write(long l) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "write");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        long hl = htonl(l);
        write((const char*)&hl, sizeof(hl));
    }

    // write: write string
    void Socket::write(const String& s) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "write");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        write(s.c_str(), s.size());
    }

    // write: write buffer
    void Socket::write(const char* buf, int sz) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "write");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        if (::send(m_handle, buf, sz, MSG_NOSIGNAL) < 0) throw Socket::Failed("write failed");
    }

    // getOption: get socket option
    int Socket::getOption(OptionFlags o) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "getOption");
        int value = 0;
        ::socklen_t len = sizeof(int);
        if (::getsockopt(m_handle, SOL_SOCKET, k_option(o), (char*)&value, &len) < 0)
            throw Socket::Failed("get socket option");
        return value;
    }
    
    // setOption: set socket option
    void Socket::setOption(OptionFlags o, int value) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "setOption");
        if (::setsockopt(m_handle, SOL_SOCKET, k_option(o), (const char*)&value, sizeof(int)) < 0)
            throw Socket::Failed("set socket option");
    }

    // getTimeout: get socket timeout
    void Socket::getTimeout(TimeoutFlags t, long& sec, long& microsec) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "getTimeout");
        ::timeval value;
        value.tv_sec  = 0L;
        value.tv_usec = 0L;
        ::socklen_t len = sizeof(::timeval);
        if (::getsockopt(m_handle, SOL_SOCKET, k_timeout(t), (char*)&value, &len) < 0)
            throw Socket::Failed("get socket option");
        sec      = value.tv_sec;
        microsec = value.tv_usec;
    }
    
    // setOption: set socket option
    void Socket::setTimeout(TimeoutFlags t, long sec, long microsec) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "setTimeout");
        ::timeval value;
        value.tv_sec  = sec;
        value.tv_usec = microsec;
        if (::setsockopt(m_handle, SOL_SOCKET, k_timeout(t), (const char*)&value, sizeof(::timeval)) < 0)
            throw Socket::Failed("set socket option");
    }

    // onConnect: connect to client socket
    void Socket::onConnect(Address addr) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "Socket::onConnect");
        if (::connect(m_handle, (struct ::sockaddr*)addr, sizeof(struct ::sockaddr)) < 0)
            throw Socket::Failed(k_message(this, m_handle, "socket connect failed"));
    }

    // getHostName: retrieve host name for machine
    String Socket::getHostName() throw (Socket::Failed)
    {
        static const std::size_t k_maxHostName = 255;
        Char buf[k_maxHostName+1];
        if (::gethostname(buf, k_maxHostName) < 0) throw Socket::Failed("hostname not available");
        buf[k_maxHostName] = 0;
        return buf;
    }

    //
    // SocketServer implementation
    //

    // create socker server
    SocketServer::SocketServer(const String& host, int port, int queue)
        : Socket(host, port), m_queue(queue)
    {}

    // listen: begin listening for clients
    void SocketServer::listen() throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "SocketServer::listen");
        if (m_handle < 0) connect();
        if (::listen(m_handle, m_queue) < 0)
            throw Socket::Failed(k_message(this, m_handle, "socket server listen failed"));
    }

    // accept: accept client request
    Socket::Handle SocketServer::accept() throw (SocketServer::Closed)
    {
        Log::Scope scope(KCC_FILE, "SocketServer::accept");
        if (m_handle < 0) throw Socket::Failed(k_notConntected);
        struct ::sockaddr_in addr = {0};
        ::socklen_t sz = sizeof(struct ::sockaddr);
        Handle handle = ::accept(m_handle, (struct ::sockaddr*)&addr, &sz);
        if (handle < 0) throw SocketServer::Closed("socket server closed"); // accept w/ invalid handle == server closed
        return handle;
    }

    // onConnect: connect to server socket
    void SocketServer::onConnect(Address addr) throw (Socket::Failed)
    { 
        Log::Scope scope(KCC_FILE, "SocketServer::onConnect");

        // bind
        if (::bind(m_handle, (struct ::sockaddr*)addr, sizeof(struct ::sockaddr)) < 0)
            throw Socket::Failed(k_message(this, m_handle, "socket server bind failed"));

        // ephemeral port
        if (m_port == 0)
        {
            struct ::sockaddr_in addr = {0};
            ::socklen_t sz = sizeof(addr);
            if (::getsockname(m_handle, (struct sockaddr*)&addr, &sz) < 0)
                throw Socket::Failed(k_message(this, m_handle, "unable to get ephemeral port"));
            m_port = ntohs(addr.sin_port);
        }
    }
}
