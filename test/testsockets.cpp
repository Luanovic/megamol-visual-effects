/*
 * testsockets.cpp
 *
 * Copyright (C) 2006 - 2007 by Universitaet Stuttgart (VIS). 
 * Alle Rechte vorbehalten.
 */

#include "testsockets.h"

#include "vislib/AsyncSocketSender.h"
#include "vislib/Socket.h"
#include "vislib/SocketException.h"
#include "vislib/SystemException.h"
#include "vislib/Thread.h"
#include "testhelper.h"


/** Port used for test server. */
static const uint16_t TEST_PORT = 7134;


/** Reads data from 'socket' member. */
class SocketReader : public vislib::sys::Runnable {

public:
    inline SocketReader(vislib::net::Socket socket) : socket(socket) {}
    virtual ~SocketReader(void);
    virtual unsigned int Run(void *cntRepeat);

protected:
    vislib::net::Socket socket;
};


/*
 * SocketReader::~SocketReader 
 */
SocketReader::~SocketReader(void) {
}


/*
 * SocketReader::Run
 */
unsigned int SocketReader::Run(void *cntRepeat) {
    using namespace vislib::net;
    unsigned int cnt = *static_cast<unsigned int *>(cntRepeat);
    unsigned int recvBuf = 0;
    unsigned int retval = 0;

    try {
        Socket::Startup();

        for (retval = 0; retval < cnt; retval++) {
            this->socket.Receive(&recvBuf, sizeof(unsigned int));
            AssertEqual("Received expected data", retval, recvBuf);
            vislib::sys::Thread::Reschedule();
        }

        Socket::Cleanup();
    } catch (SocketException e) {
        std::cerr << e.GetMsgA() << std::endl;
    }

    return retval;
}


/** Writes data to 'socket' member. */
class SocketWriter : public vislib::sys::Runnable {

public:
    inline SocketWriter(vislib::net::Socket socket) : socket(socket) {}
    virtual ~SocketWriter(void);
    virtual unsigned int Run(void *cntRepeat);

protected:
    vislib::net::Socket socket;
};


/*
 * SocketWriter::~SocketWriter 
 */
SocketWriter::~SocketWriter(void) {
}


/*
 * SocketWriter::run
 */
unsigned int SocketWriter::Run(void *cntRepeat) {
    using namespace vislib::net;
    unsigned int cnt = *static_cast<unsigned int *>(cntRepeat);
    unsigned int retval = 0;

    try {
        Socket::Startup();

        for (retval = 0; retval < cnt; retval++) {
            this->socket.Send(&retval, sizeof(unsigned int));
        }

        Socket::Cleanup();
    } catch (SocketException e) {
        std::cerr << e.GetMsgA() << std::endl;
    }

    return retval;
}


class SocketServer : public vislib::sys::Runnable {

public:
    inline SocketServer(void) {}
    virtual ~SocketServer(void);
    virtual unsigned int Run(void *cntRepeat);
};


/*
 * SocketServer::~SocketServer 
 */
SocketServer::~SocketServer(void) {
}


/*
 * SocketServer::run
 */
unsigned int SocketServer::Run(void *cntRepeat) {
    using namespace vislib::net;
    using namespace vislib::sys;
    unsigned int retval = 0;
    Socket serverSocket;
    Socket clientSocket;
    
    try {
        Socket::Startup();

        serverSocket.Create(Socket::FAMILY_INET, Socket::TYPE_STREAM,
            Socket::PROTOCOL_TCP);
        serverSocket.SetReuseAddr(true);
        serverSocket.Bind(SocketAddress::CreateInet(TEST_PORT));
        serverSocket.Listen();
        clientSocket = serverSocket.Accept();
        serverSocket.Close();
    
        SocketWriter writer(clientSocket);
        Thread writerThread(&writer);
        SocketReader reader(clientSocket);
        Thread readerThread(&reader);

        writerThread.Start(cntRepeat);
        readerThread.Start(cntRepeat);

        writerThread.Join();
        readerThread.Join();

        retval = writerThread.GetExitCode() + readerThread.GetExitCode();

        clientSocket.Close();
        Socket::Cleanup();
    } catch (SystemException e) {
        std::cerr << e.GetMsgA() << std::endl;
    }
    return retval;
}


class SocketClient : public vislib::sys::Runnable {

public:
    inline SocketClient(void) {}
    virtual ~SocketClient(void);
    virtual unsigned int Run(void *cntRepeat);
};


/*
 * SocketClient::~SocketClient 
 */
SocketClient::~SocketClient(void) {
}


/*
 * SocketClient::run
 */
unsigned int SocketClient::Run(void *cntRepeat) {
    using namespace vislib::net;
    using namespace vislib::sys;
    unsigned int retval = 0;
    Socket clientSocket;
    
    try {
        Socket::Startup();

        clientSocket.Create(Socket::FAMILY_INET, Socket::TYPE_STREAM,
            Socket::PROTOCOL_TCP);
        clientSocket.SetNoDelay(true);
        clientSocket.Connect(SocketAddress::CreateInet("127.0.0.1", TEST_PORT));
    
        SocketWriter writer(clientSocket);
        Thread writerThread(&writer);
        SocketReader reader(clientSocket);
        Thread readerThread(&reader);

        writerThread.Start(cntRepeat);
        readerThread.Start(cntRepeat);

        writerThread.Join();
        readerThread.Join();

        retval = writerThread.GetExitCode() + readerThread.GetExitCode();

        clientSocket.Close();
        Socket::Cleanup();
    } catch (SystemException e) {
        std::cerr << e.GetMsgA() << std::endl;
    }
    return retval;
}


void TestAsyncSocketSender(void) {
    using namespace vislib::net;
    using namespace vislib::sys;

    unsigned int cntRepeat = 10;
    Socket socket;
    AsyncSocketSender sender;
    Thread senderThread(&sender);
    SocketServer server;
    Thread serverThread(&server);

    Socket::Startup();

    serverThread.Start(&cntRepeat);

    Thread::Sleep(1000);
    socket.Create(Socket::FAMILY_INET, Socket::TYPE_STREAM,
        Socket::PROTOCOL_TCP);
    socket.SetNoDelay(true);
    socket.Connect(SocketAddress::CreateInet("127.0.0.1", TEST_PORT));
    senderThread.Start(&socket);

    for (unsigned int i = 0; i < cntRepeat; i++) {
        sender.Send(&i, sizeof(unsigned int));
    }

    serverThread.Join();
    senderThread.Terminate(false);
    Socket::Cleanup();

    AssertEqual("Server got expected number of asynchronously sent messages", serverThread.GetExitCode(), 2 * cntRepeat);
}


void TestSockets(void) {
    using namespace vislib::sys;
    unsigned int cntRepeat = 10;
    SocketServer server;
    Thread serverThread(&server);
    SocketClient client;
    Thread clientThread(&client);

    serverThread.Start(&cntRepeat);
    clientThread.Start(&cntRepeat);

    serverThread.Join();
    clientThread.Join();

    AssertEqual("Server processed expected number of messages", serverThread.GetExitCode(), 2 * cntRepeat);
    AssertEqual("Client processed expected number of messages", serverThread.GetExitCode(), 2 * cntRepeat);

    TestAsyncSocketSender();
}
