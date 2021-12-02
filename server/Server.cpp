#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>
#include "Semaphore.h"
#include <thread>

using namespace Sync;
using namespace std;

class SocketThread : public Thread
{
private:
    Socket &socket;        // client socket reference
    ByteArray data;        // a byte array of the data being sent and received
    int numberOfChatRooms; // self explanatory
    int port;              // the port our server is running on
    bool &isActive;        // a reference to the current state of the server (on or off) --> (true or false)

    vector<SocketThread *> &socketThreadsHolder; // this vector stores all the SocketThread pointers

public:
    SocketThread(Socket &socket, vector<SocketThread *> &clientSockThr, bool &isActive, int port) : socket(socket), socketThreadsHolder(clientSockThr), isActive(isActive), port(port)
    {
    }

    ~SocketThread()
    {
        this->terminationEvent.Wait();
    }

    Socket &GetSocket()
    {
        return socket;
    }

    const int GetChatRoom()
    {
        return numberOfChatRooms;
    }

    virtual long ThreadMain()
    {
        // Parse the integer value of the port number to a string.
        ::string stringPort = to_string(port);

        // Semaphore generated on each socket thread by referencing port number as the name.
        Semaphore clientBlock(stringPort);

        try
        {
            // Attempt to gather bytestream data.
            socket.Read(data);

            string chatRoomString = data.ToString();
            chatRoomString = chatRoomString.substr(1, chatRoomString.size() - 1);
            numberOfChatRooms = ::stoi(chatRoomString);
            cout << "Current chat room number from socket.Read(): " << numberOfChatRooms << ::endl;

            while (!isActive)
            {
                int socketResult = socket.Read(data);
                // If the socket is closed on the client side, isActive this socket thread.
                if (socketResult == 0)
                    break;

                string recv = data.ToString();
                if (recv == "shutdown\n")
                {
                    // Mutual exclusion assurance.
                    clientBlock.Wait();

                    // Iterator method to select and erase this socket thread.
                    socketThreadsHolder.erase(remove(socketThreadsHolder.begin(), socketThreadsHolder.end(), this), socketThreadsHolder.end());

                    // Exit critical section
                    clientBlock.Signal();

                    cout << "A client is shutting off from our server. Erase client!" << endl;
                    break;
                }

                // A forward slash is appended as the first character to change the chat room number.
                if (recv[0] == '/')
                {
                    // Split the received string.
                    string stringChat = recv.substr(1, recv.size() - 1);

                    // Parse the integer after the forward slash character, representing the chat room number.
                    numberOfChatRooms = stoi(stringChat);
                    cout << "A client just joined room " << numberOfChatRooms << ::endl;
                    continue;
                }

                // Call the semaphore blocking call so that the thread can enter the critical section.
                clientBlock.Wait();
                for (int i = 0; i < socketThreadsHolder.size(); i++)
                {
                    SocketThread *clientSocketThread = socketThreadsHolder[i];
                    if (clientSocketThread->GetChatRoom() == numberOfChatRooms)
                    {
                        Socket &clientSocket = clientSocketThread->GetSocket();
                        ByteArray sendBa(recv);
                        clientSocket.Write(sendBa);
                    }
                }
                // Exit critical section.
                clientBlock.Signal();
            }
        }
        // Catch string-thrown exceptions (e.g. stoi())
        catch (string &s)
        {
            cout << s << ::endl;
        }
        // Catch thrown exceptions and distinguish them in console.
        catch (exception &e)
        {
            cout << "A client has abruptly quit their messenger app!" << endl;
        }
        cout << "A client has left!" << endl;
    }
};

class ServerThread : public Thread
{
private:
    // Reference to socket server.
    SocketServer &server;

    //vector<Socket *> socketsHolder;
    vector<SocketThread *> socketThrHolder;

    // Given socket port number.
    int port;

    // Given chats number.
    int numberRooms;

    // Flag for termination.
    bool isActive = false;

public:
    ServerThread(SocketServer &server, int numberRooms, int port)
        : server(server), numberRooms(numberRooms), port(port)
    {
    }

    ~ServerThread()
    {
        // Cleanup
        //this->terminationEvent.Wait();

        // MAKE SURE TO CLOSE ALL SOCKETS (assigning null / deallocating memory).
        //vector<Socket *>().swap(socketsHolder);
        //vector<SocketThread *>().swap(socketThrHolder);

        //delete[] socketsHolder;
        //delete[] socketThrHolder;

        // Close the client sockets.
        for (auto thread : socketThrHolder)
        {
            try
            {
                // Close the socket.
                Socket &toClose = thread->GetSocket();
                toClose.Close();
            }
            catch (...)
            {
                // This will catch all exceptions.
            }
        }
        vector<SocketThread *>().swap(socketThrHolder);
        isActive = true;
    }

    virtual long ThreadMain()
    {
        while (true)
        {
            try
            {
                // Stringify port number.
                string stringPortNum = to_string(port);
                cout << "FlexWait/Natural blocking call on client!" << endl;

                // Main owner semaphore to block other semaphores by name.
                Semaphore serverBlock(stringPortNum, 1, true);

                // Front-end receives number of chats through socket.
                ::string allChats = to_string(numberRooms) + '\n';

                // Byte array conversion for number of chats.
                ByteArray allChats_conv(allChats);

                // Wait for a client socket connection
                Socket sock = server.Accept();

                // Send number of chats.
                sock.Write(allChats_conv);
                Socket *newConnection = new Socket(sock);

                //socketsHolder.push_back(newConnection);

                // Pass a reference to this pointer into a new socket thread.
                Socket &socketReference = *newConnection;
                socketThrHolder.push_back(new SocketThread(socketReference, ref(socketThrHolder), isActive, port));
            }
            // Catch string-thrown exception.
            catch (string error)
            {
                cout << "ERROR: " << error << endl;
                // Exit thread function.
                return 1;
            }
            // In case of unexpected shutdown.
            catch (TerminationException terminationException)
            {
                cout << "Server has shut down!" << endl;
                // Exit with exception thrown.
                return terminationException;
            }
        }
    }
};

int main(void)
{
    // AWS port.
    int port = 3005;

    int rooms = 20; // the number of chat rooms

    cout << "SE 3316 Server" << endl
         << "Type done to quit the server..." << endl;

    // Create our server.
    SocketServer server(port);

    // Need a thread to perform sever operations.
    ServerThread st(server, rooms, port);

    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    cin.get();

    // Cleanup, including exiting clients, when the user presses enter

    // Shut down and clean up the server
    server.Shutdown();

    cout << "Good-bye!" << endl;
}