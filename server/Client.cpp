
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;
using namespace std;

// function conn() calls itself recursively until a successful connection is made
Socket conn()
{
	Socket socket("127.0.0.1", 6758);

	try
	{
		socket.Open();
		return socket;
	}
	catch (...)
	{
		cout << "connection failed please try again...\n";
		sleep(2);
		return conn();
	}
}

int main(void)
{
	// Welcome the user
	cout << "SE3313 Lab 3 Client" << std::endl;

	// Create our socket
	Socket socket = conn();

	// define a string that the client will send to be processed by the server
	// must be defined in the scope of the main while loop and not just inside of the
	// main function so that the text variable is assigned to each new connection that is
	// made through calling conn()
	while (1)
	{

		// get the input from the user for the message being sent
		string text;
		cout << "\nplease enter the string you wish to send to the server \n";
		cin >> text;

		//if the input is "Done" the terminate gracefully
		if (text == "Done")
		{
			socket.Write(text);
			socket.Close();
			break;
		}
		else
		{
			cout << "\nText recorded. Sending...\n";
			//To write to socket and read from socket. You may use ByteArray
			socket.Write(ByteArray(text));

			ByteArray res;
			string response;

			while (socket.Read(res) == 0)
			{
				cout << "pending response...";
				//check every second for the response
				sleep(1);
			}

			// response is returned as a byte array, have to convert back to string
			response = res.ToString();

			cout << "\nResponse recieved. Displaying response: " + response + "\n";
		}
	}

	socket.Close();
	cout << "\nThe connection has been closed\n";

	return 0;
}
