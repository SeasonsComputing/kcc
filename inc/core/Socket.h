/*
 * Kuumba C++ Core
 *
 * $Id: Socket.h 23304 2008-04-28 15:13:12Z tvk $
 */
#ifndef Socket_h
#define Socket_h

namespace kcc
{
    /**
     * Socket client
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Socket
    {
    public:
        /** Socket types */
        typedef int   Handle;
        typedef void* Address;

        /** Socket operation failed exception */
        KCC_COMPONENT_EXCEPTION(Failed);

        /**
         * Socket client ctor
         */
        Socket();

        /**
         * Socket client ctor
         * @param host host name
         * @param port port number
         */
        Socket(const String& host, int port);

        /**
         * Socket client ctor. Attach to handle.
         * @param handle socket to attach to
         * @throw Socket::Failed if invalid handle
         */
        Socket(Handle handle) throw (Socket::Failed);

        /** Close socket */
        virtual ~Socket();

        /** Socket lifecycle */
        void connect(const String& host, int port) throw (Socket::Failed);
        void connect()                             throw (Socket::Failed);
        void close();

        /** Reader methods */
        void read(long& l)                        throw (Socket::Failed);
        void read(char* buf, int sz, int& actual) throw (Socket::Failed);

        /** Writer methods */
        void write(long l)                  throw (Socket::Failed);
        void write(const String& s)         throw (Socket::Failed);
        void write(const char* buf, int sz) throw (Socket::Failed);

        /** Accessors */
        inline const String& ip()   { return m_ip;   }
        inline const String& host() { return m_host; }
        inline int           port() { return m_port; }
        
        /** Option Flags */
        enum OptionFlags
        {
            O_KEEPALIVE,
            O_LINGER,
            O_SNDBUF,
            O_RCVBUF
        };
        int  getOption(OptionFlags o)            throw (Socket::Failed);
        void setOption(OptionFlags o, int value) throw (Socket::Failed);

        /** Timeout Flags */
        enum TimeoutFlags
        {
            T_SEND,
            T_RECEIVE
        };
        void getTimeout(TimeoutFlags t, long& sec, long& microsec) throw (Socket::Failed);
        void setTimeout(TimeoutFlags t, long sec, long microsec)   throw (Socket::Failed);

        /**
         * Accessor to host name
         * @returns host name
         * @throws Socket::Failled if unable to get host name
         */
        static String getHostName() throw (Socket::Failed);

    protected:
        // Attributes
        String m_ip;
        String m_host;
        int    m_port;
        Handle m_handle;

        /** onConnect: Template method (GOF) - client connection to server */
        virtual void onConnect(Address addr) throw (Socket::Failed);

    private:
        Socket(const Socket&);
        Socket& operator = (const Socket&);
    };

    /**
     * Socket server
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT SocketServer : public Socket
    {
    public:
        /** Socket closed */
        KCC_COMPONENT_EXCEPTION(Closed);

        /**
         * Socket server ctor
         * @param host host name
         * @param port port number
         * @param queue listen queue size
         */
        SocketServer(const String& host, int port, int queue = 32);

        /** Initiate client listening (will connect if not already) */
        void listen() throw (Socket::Failed);

        /**
         * Accept a client request (blocking call)
         * @return handle to client socket
         */
        Handle accept() throw (SocketServer::Closed);

    protected:
        /** onConnect: Template method (GOF) - server binding connection */
        virtual void onConnect(Address addr) throw (Socket::Failed);

    private:
        SocketServer(const SocketServer&);
        SocketServer& operator = (const SocketServer&);

        // Attributes
        int m_queue;
    };
}

#endif // Socket_h
