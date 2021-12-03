#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;
using namespace std;

class ClientThread : public Thread //thread for handling connection to server
{
private:
	Socket &socket; // ref to socket

	// whats sent to the server
	ByteArray input;
	string input_str;

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

		// input collection
		getline(cin, input_str);
		input = ByteArray(input_str);

		//writing to socket
		socket.Write(input);

		//response collection
		socket.Read(input);
		input_str = input.ToString();
		cout << "Response: " << input_str << endl;
		return 0;
	}
};

int main(void)
{
	cout << "SE3313 Project" << endl;

	// socket creation
	Socket socket("172.23.40.250", 3000);
	ClientThread clientThread(socket);
	while (1)
	{
		sleep(1);
	}
	socket.Close();

	return 0;
}
