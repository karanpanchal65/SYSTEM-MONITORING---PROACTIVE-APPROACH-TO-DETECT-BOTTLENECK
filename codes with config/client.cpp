#include"sysinteract.h"

struct sockaddr_in srv; //contain details about family,port.ip adrress and some other info.

#define charSize 5000

int nClientSocket;    // socket Id

char buff[charSize] = { 0 };  // buffer to send data and information

//function to retreive values from confoguration file
void loadConfig(Config& config)
{
	ifstream fin("clientConfigfile.txt");
	string line;
	while (getline(fin, line)) {
		istringstream sin(line.substr(line.find("=") + 1));
		if (line.find("PORT") != -1)
			sin >> config.PORT;
		else if (line.find("SleepDuration") != -1)
			sin >> config.sleepDuration;
		else if (line.find("IPaddr") != -1)
			sin >> config.ipaddr;
		else if (line.find("memsetSize") != -1)
			sin >> config.memSize;
		else if (line.find("commandbuffSize") != -1)
			sin >> config.msgbuffSize;
	}
}

// This function Periodically sends the data to the server
void storeData::sendDataToServer(string uid)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(4000));
		string s = uid + "," + fetchNewData();

		//Parsing data at client side

		vector<string> chdb;
		string tmp = "";

		for (int k = 0; k < s.length() - 1; k++)
		{
			if (s[k] != ',')
			{
				tmp = tmp + s[k];
			}
			else
			{
				chdb.push_back(tmp);
				tmp = "";
			}
		}
		chdb.push_back(tmp);

		string hostname, clientUid, username, processorArchitecture, timeStamp;
		int totalDiskSpace, freeDiskSpace, totalRam, availRam, processorType, noOfProcessors;
		float cpuLoad;

		int checkSum = 0;

		clientUid = chdb[0];
		hostname = chdb[1];

		username = chdb[2];
		totalRam = stoi(chdb[3]);

		availRam = stoi(chdb[4]);
		totalDiskSpace = stoi(chdb[5]);

		freeDiskSpace = stoi(chdb[6]);
		cpuLoad = stof(chdb[7]);

		processorArchitecture = chdb[8];
		noOfProcessors = stoi(chdb[9]);
		processorType = stoi(chdb[10]);
		timeStamp = chdb[11];

		parseData pD(clientUid, hostname, username, totalRam, availRam, totalDiskSpace, freeDiskSpace, cpuLoad, processorArchitecture, noOfProcessors, processorType, timeStamp);
		checkSum = pD.checkData();

		s = to_string(checkSum) + "," + s;

		int x = 0;
		for (; x < s.length(); x++)
		{
			buff[x] = s[x];
		}

		send(nClientSocket, buff, 5000, 0);
	}
}

int main()
{
	//Configuration 
	Config config;
	loadConfig(config);

	int PORT = config.PORT;
	int duration = config.sleepDuration;
	const char* addr = config.ipaddr.c_str();
	int mSize = config.memSize;
	const int bSize = config.msgbuffSize;

	int nRet = 0;

	WSADATA ws;   //WSA is responsible for all the socket api working.

	cout << endl << "**********************************************System-Monitor-Client*****************************************************" << endl;

	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << endl << "WSA failed to initialize.";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);       //socket descriptor id.

	if (nClientSocket < 0)
	{
		cout << endl << "The Socket is not opened.";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "WSA is initialized AND The Socket is opened successfully.";
	}

	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr(addr);   // adress of machine where server is running
	memset(&(srv.sin_zero), 0, mSize);

	string uid;     //Unique Identifier for the Client 

	cout << endl << "Enter the name of client :";
	cin >> uid;
	for (int i = 0; i < uid.length(); i++)
	{
		uid[i] = tolower(uid[i]);
	}
	cout << "Hello .....   I am CLIENT " << uid << "  connecting to the Server.............";

	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));     //connecting to the server

	if (nRet < 0)
	{
		cout << endl << "Connection Failed";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		recv(nClientSocket, buff, 255, 0);  // acknowledgement that the connection is done successfully
		cout << endl << buff;
		cout << endl << "Connected to the server";
		cout << endl << "Now sending the System Information to the server............" << endl;

		// Intializing the object of storeData Class
		storeData obj;

		// Call to start the thread for fetching system attributes
		obj.intialiseFetchDataThread();

		std::this_thread::sleep_for(std::chrono::milliseconds(duration));

		// Call to start the thread for sending system attributes to the server
		obj.intialiseSendDataThread(uid);

		char input;

		while (true)
		{
			//Commands to interact with the Server
			cout << "Command to interact with Server." << endl;
			cout << "Press 1 to get Last five record sent to the Server." << endl;
			cout << "Press * to get Last ten record sent to the Server." << endl;
			cout << "Press # to delete my all data sent to the Server." << endl;
			cout << "Press $ to delete my all data sent to the Server AND CLOSE THE CLIENT." << endl;
			cout << "Press 2 to CLOSE the Client." << endl;
			//cout << "." << endl;
			cin >> input;
			char msg[charSize] = { 0 };

			msg[0] = '0';
			msg[1] = ',';
			int i = 2, j = 0;
			while (j < uid.length())
			{
				msg[i++] = uid[j++];

			}
			msg[i] = '~';

			// Sending Featured REQUEST to the Server
			switch (input)
			{
			case '1':
				cout << "Last 5 record sent to Server. " << endl;
				msg[0] = '1';
				send(nClientSocket, msg, bSize, 0);
				recv(nClientSocket, msg, bSize, 0);
				cout << msg << endl;
				break;
			case '*': cout << "Last 10 record sent to Server" << endl;
				msg[0] = '*';
				send(nClientSocket, msg, bSize, 0);
				recv(nClientSocket, msg, bSize, 0);
				cout << msg << endl;;
				break;
			case '#': cout << "Deleting my ALL data from the server" << endl;
				msg[0] = '#';
				send(nClientSocket, msg, 255, 0);
				break;
			case '$': cout << "Deleting my ALL data from the server AND CLOSING THE CLIENT  : " << uid << endl;
				msg[0] = '#';
				send(nClientSocket, msg, 255, 0);
				exit(0);
			default:
				exit(0);
			}
		}
	}
	return 0;
}