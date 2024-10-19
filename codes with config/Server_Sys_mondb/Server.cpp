#include"serverHeader.h"

#define NOC 100

using namespace std;

struct sockaddr_in srv;     // contain details about family,port.ip adrress and some other info.

int nSocket;                // listener socket

fd_set fr, fw, fe;          // socket descriptors(ready to read, ready to write to network, exception thowing errors)

int nmaxFd;

int nArrClient[NOC];		// for handling 100 clients

//function to retreive values from confoguration file
void loadConfig(Config& config)
{
	ifstream fin("serverConfigfile.txt");
	string line;
	while (getline(fin, line)) {
		istringstream sin(line.substr(line.find("=") + 1));
		if (line.find("PORT") != -1)
			sin >> config.PORT;
		else if (line.find("memsetSize") != -1)
			sin >> config.memSize;
	}
}

int main()
{
	//Configuration 
	Config config;
	loadConfig(config);

	int PORT = config.PORT;
	int mSize = config.memSize;

	server ob1;

	// Database code begins
	MYSQL* conn;
	MYSQL_ROW row;
	MYSQL_RES* res;


	// We initialize MySQL here
	conn = mysql_init(0);

	// Variables are passed in this function for connection to the DB
	conn = mysql_real_connect(conn, "localhost", "root", "123456", "testdb", 3306, NULL, 0);
	int nRet = 0;

	WSADATA ws;            //WSA is responsible for all the socket api working.

	cout << endl << "**********************************************System-Monitor-Server****************************************************" << endl;
	cout << endl << "Hello ..... I am SERVER" << endl;

	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << endl << "WSA failed to initialize." << endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "WSA is initialized." << endl;
	}

	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    //socket descriptor id.

	if (nSocket < 0)
	{
		cout << "The Socket is not opened." << endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "The Socket is opened successfully." << endl;
	}

	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = INADDR_ANY; // assigning this machine address (ip) bcz we will running this server on local host
	memset(&(srv.sin_zero), 0, mSize);
	u_long optval = 0;
	nRet = ioctlsocket(nSocket, FIONBIO, &optval);

	if (nRet != 0)
	{
		cout << endl << "fail blocking socket ioctl call";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{

	}

	nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));   // binding the socket to local port

	if (nRet < 0)
	{
		cout << "Fail to bind to Local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{

	}

	nRet = listen(nSocket, 100);   //listen the request from client(queues the request);
	// how many request can be there in the server queue(backlog ,100)

	if (nRet < 0)
	{
		cout << endl << "Fail to start to listen to Local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "Start Successfully able to listening to local port : " << PORT;
	}


	//keep waiting for new request and proceed as per the request
	// select is as system call for socket descriptors

	nmaxFd = nSocket;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while (1)
	{

		FD_ZERO(&fr);
		FD_ZERO(&fw);
		FD_ZERO(&fe);

		FD_SET(nSocket, &fr);
		FD_SET(nSocket, &fe);

		for (int nIndex = 0; nIndex < 100; nIndex++)
		{
			if (nArrClient[nIndex] != 0)
			{
				FD_SET(nArrClient[nIndex], &fr);
				FD_SET(nArrClient[nIndex], &fe);
			}
		}

		nRet = select(nmaxFd + 1, &fr, &fw, &fe, &tv);

		if (nRet > 0)
		{
			// when someone connect or coomunicate over a dedicated msg
			cout << endl << "Data on PORT ..... Processing now...";
			//processing the request as per the need

			if (FD_ISSET(nSocket, &fe))
			{
				cout << endl << "There is exception....";
			}
			if (FD_ISSET(nSocket, &fw))
			{
				cout << endl << "Ready to write something....";
			}
			if (FD_ISSET(nSocket, &fr))
			{
				// it is listener socket accept the connection
				cout << endl << "Ready to read. Something at the PORT....";
			}


			ob1.ProcessNewRequest(conn, fr, nSocket, nArrClient);

		}
		else if (nRet == 0)
		{
			// no connection request
			// none of the socket descriptors are ready
			cout << endl << "Nothing on the port:\t" << PORT;
		}

		else
		{
			// select function call fails
			cout << endl << "failed select call";
			WSACleanup();
			exit(EXIT_FAILURE);
		}
	}

	return 0;


}