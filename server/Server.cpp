#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
using namespace std;

#define MAX 26

void alternateULSort(string &s)
{
    int n = s.length();
    int lower[MAX] = {0};
    int upper[MAX] = {0};
    for (int i = 0; i < n; i++)
    {
        if (isupper(s[i]))
            upper[s[i] - 'A']++;
        else
            lower[s[i] - 'a']++;
    }
    int i = 0, j = 0, k = 0;
    while (k < n)
    {
        while (i < MAX && upper[i] == 0)
            i++;
        if (i < MAX)
        {
            s[k++] = 'A' + i;
            upper[i]--;
        }
        while (j < MAX && lower[j] == 0)
            j++;
        if (j < MAX)
        {
            s[k++] = 'a' + j;
            lower[j]--;
        }
    }
}
class SocketThread : public Thread
{
private:
    ByteArray recv;
    bool &threadIsActive;

public:
    Socket &inboundSocket; // this is the client socket opening the connection

    SocketThread(Socket &inboundSocket, bool &threadIsActive) : inboundSocket(inboundSocket), threadIsActive(threadIsActive) {}

    ~SocketThread() {}

    Socket &getSocket()
    {
        return inboundSocket;
    }

    virtual long ThreadMain()
    {
        cout << "\nnew client connection initiated...\n";

        while (threadIsActive)
        {

            string res;

            while (inboundSocket.Read(recv) == 0)
            {
                cout << "\nwaiting to recieve input from client\n";
                sleep(3);
            }

            cout << "\nclient input has been recieved...\n";

            res = recv.ToString();

            if (res == "Done")
            {
                cout << "\ninitiating graceful thread termination\n";
                threadIsActive = false;
                return 1;
            }
            else
            {
                alternateULSort(res);

                cout << "\nclient input processed. returning the following string..." + res + "\n";

                inboundSocket.Write(res);
            }
        }
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    vector<SocketThread *> socketThreads;
    bool threadIsActive = true;

public:
    SocketServer &server;

    ServerThread(SocketServer &server)
        : server(server)
    {
    }

    ~ServerThread()
    {
        cout << "\ngracefully terminating thread...\n";

        for (auto iterable : socketThreads)
        {
            try
            {
                Socket &inboundSocketRef = iterable->getSocket();
                inboundSocketRef.Close();
                delete iterable;
            }
            catch (...)
            {
                cout << "ERROR";
            }
        }
        threadIsActive = !threadIsActive;
    }

    virtual long ThreadMain()
    {
        while (threadIsActive)
        {
            cout << "\nAwaiting client socket connection\n";

            Socket *newConnection = new Socket(server.Accept());

            cout << "\nThe client socket has connected...\n";

            // A reference to this pointer
            Socket &socketReference = *newConnection;

            socketThreads.push_back(new SocketThread(socketReference, threadIsActive));
        }
        return 1;
    }
};

int main(void)
{
    cout << "I am a server." << std::endl;

    // Create our server
    SocketServer server(6758);

    pid_t cpid = fork();

    //child process
    if (cpid == 0)
    {
        bool running = true;
        while (running)
        {
            string serverInput;
            cin >> serverInput;
            if (serverInput == "Done")
            {
                running = false;
                // Shut down and clean up the server
                server.Shutdown();
            }
        }
    }
    // parent process
    else if (cpid > 0)
    {

        // Need a thread to perform server operations
        ServerThread serverThread(server);

        // This will wait for input to shutdown the server
        FlexWait cinWaiter(1, stdin);
        cinWaiter.Wait();
    }
}
