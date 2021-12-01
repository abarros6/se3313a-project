#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;
using namespace std;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket &socket;

	// Data to send to server
	ByteArray data;
	string data_str;

public:
	ClientThread(Socket &socket)
		: socket(socket)
	{
	}

	~ClientThread()
	{
	}

	virtual long ThreadMain()
	{
		int res = socket.Open();
		cout << "Enter a message to send (enter done to end): ";
		cout.flush();

		// Get the data
		getline(cin, input_str);
		input = ByteArray(input_str);

		// Write to the server
		socket.Write(input);

		// Get the response
		socket.Read(input);
		input_str = input.ToString();
		cout << "Response: " << input_str << endl;
		return 0;
	}
};

int main(void)
{
	// Welcome the user
	cout << "SE3313 Project" << endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket);
	while (1)
	{
		sleep(1);
	}
	socket.Close();

	return 0;
}
