#include"serverHeader.h"

void server::deletedb(MYSQL* conn, string m, int nClientSocket)
{
	MYSQL_ROW row;
	MYSQL_RES* res;
	string j;
	j = m;
	stringstream ss, sss;
	string id;
	id = to_string(nClientSocket);

	//this query checks that the data we need to delete exists in the table or not
	sss << "SELECT * FROM sysmonitor where clientUid = '" + j + "' and socketID = '" + id + "'";

	//convert sss into string
	string query = sss.str();
	const char* q = query.c_str();
	qstate = mysql_query(conn, q);
	res = mysql_store_result(conn); // result is stored in res
	int count = mysql_num_fields(res); // counts no of coulmns
	my_ulonglong x = mysql_num_rows(res); // counts no of rows

	if (x > 0)
	{
		// this query deletes the required data
		ss << "DELETE FROM sysmonitor where clientUid = '" + j + "' and socketID = '" + id + "'";
		string query = ss.str();
		const char* q = query.c_str();
		//query result is stored in qstate
		qstate = mysql_query(conn, q);

	}
	else
	{
		cout << "No record Found";
	}
}

void server::insert(MYSQL* conn, vector<string>& db)
{
	int qstate = 0;

	// Creating Query Stream for passing as String
	stringstream ss;

	string hostname, clientUid, username, processorArchitecture, timeStamp;
	int socketID, totalDiskSpace, freeDiskSpace, totalRam, availRam, cpuIdleTime, processorType, noOfProcessors;
	float cpuLoad;

	//variables are getting changed into respective types
	socketID = stoi(db[0]);
	clientUid = db[1];
	hostname = db[2];
	username = db[3];
	totalRam = stoi(db[4]);
	availRam = stoi(db[5]);
	totalDiskSpace = stoi(db[6]);
	freeDiskSpace = stoi(db[7]);
	cpuLoad = stof(db[8]);
	processorArchitecture = db[9];
	processorType = stoi(db[10]);
	noOfProcessors = stoi(db[11]);
	timeStamp = db[12];

	//insertion is done using this MySQL query
	ss << " INSERT INTO sysmonitor (socketID,clientUid, hostname, username, totalRam, availRam, totalDiskSpace,"
		"freeDiskSpace, cpuLoad, processorArchitecture, processorType, noOfProcessors, timeStamp) "
		"values ('" << socketID << "','" << clientUid << "','" << hostname << "','" << username << " ','"
		<< totalRam << "','" << availRam << "','" << totalDiskSpace << "','" << freeDiskSpace << "','"
		<< cpuLoad << "','" << processorArchitecture << "','" << processorType << "','" << noOfProcessors << "','" << timeStamp << "')";

	//convert ss into string
	string query = ss.str();
	const char* q = query.c_str();

	//query result is stored in qstate
	qstate = mysql_query(conn, q);

	// checks if data insertion is successful
	if (qstate == 0)
	{

		cout << "Record inserted successfully ..." << endl;
	}
	else
	{
		cout << " Error, data not inserted..." << endl;
	}
}




void server::ProcessNewRequest(MYSQL* conn, fd_set& fr, int nSocket, int nArrClient[])
{
	//new connection request
	if (FD_ISSET(nSocket, &fr))
	{
		int nLen = sizeof(struct sockaddr);

		int nClientSocket = accept(nSocket, NULL, &nLen);    // here we get new socketid where tranfer of data will occur

		if (nClientSocket > 0)
		{
			int indx = 0;

			//put the new client into the client fd set

			for (; indx < 100; indx++)
			{
				if (nArrClient[indx] == 0)
				{
					nArrClient[indx] = nClientSocket;
					send(nClientSocket, "Got the connection successfully", 35, 0);
					break;
				}
			}
			if (indx == 100)
			{
				cout << endl << "No space for a new client/connection";

			}

		}
	}
	else
	{

		for (int nIndex = 0; nIndex < 100; nIndex++)
		{
			if (FD_ISSET(nArrClient[nIndex], &fr))
			{

				//got the new msg from client just recieve msg and process it accordingly

				ProcessNewMessage(nArrClient[nIndex], conn, nArrClient);

			}
		}

	}
}

void server::ProcessNewMessage(int nClientSocket, MYSQL* conn, int nArrClient[])
{
	// receiving data from the client

	cout << endl << "Processing the Data for Client Socket :" << nClientSocket << endl;
	char buff[5000] = { 0 };
	int nRet = recv(nClientSocket, buff, 5000, 0);

	if (nRet < 0)
	{
		// recv call fails if client is closed
		cout << endl << "Client Disconnected......";
		closesocket(nClientSocket);
		for (int indx = 0; indx < 10; indx++)
		{
			if (nArrClient[indx] == nClientSocket)
			{
				nArrClient[indx] = 0; // removing client from the client fd set
				break;
			}
		}

	}
	else
	{
		// checking if it's command or data
		if (buff[0] == '1' || buff[0] == '*' || buff[0] == '#')
		{
			// command from the clent processing it accordingly
			MYSQL_ROW row;
			MYSQL_RES* res;
			string nCs;
			string clientid;
			vector <string> req;
			string tmp;
			for (int i = 0; i < 5000; i++)
			{
				if (buff[i] == '~')
				{
					break;
				}
				if (buff[i] != ',')
				{
					tmp = tmp + buff[i];

				}
				else {
					req.push_back(tmp);
					tmp = "";
				}
			}

			req.push_back(tmp);
			clientid = req[1];
			string m;
			m = clientid;
			string id;
			id = to_string(nClientSocket);
			string query;

			// if condition is met deletedb is called
			if (buff[0] == '#')
			{
				deletedb(conn, m, nClientSocket);

			}
			else
			{
				if (buff[0] == '1')
				{
					//this query fetches latest 5 values of a client
					query = "SELECT * FROM sysmonitor where clientUid = '" + m + "' and socketID = '" + id + "' order by timeStamp desc limit 5";
				}
				else
				{
					//this query fetches latest 10 values of a client
					query = "SELECT * FROM sysmonitor where clientUid = '" + m + "' and socketID = '" + id + "' order by timeStamp desc limit 10";
				}

				const char* q = query.c_str();
				string s;

				//query result is stored in qstate
				qstate = mysql_query(conn, q);

				int tempLarFreeRam = INT_MIN;
				int tempMinFreeRam = INT_MAX;
				float tempMinCpuLoad = 100000.0;
				float tempMaxCpuLoad = 0;
				string time[4];

				if (!qstate)
				{
					// storing result in res
					res = mysql_store_result(conn);

					while (row = mysql_fetch_row(res))   // fetching row from res
					{
						// row is getting stored in string s
						s = s + "Socket ID: " + row[0] + "," + "Client UID: " + row[1] + "," + "Host Name: " + row[2] + ","
							+ "Username: " + row[3] + "," + "Total RAM: " + row[4] + "," + "Available RAM: " + row[5] + ","
							+ "Total Disk Space: " + row[6] + "," + "Free Disk Space: " + row[7] + "," + "CPU Load: " + row[8] + ","
							+ "Processor Architecture: " + row[9] + "," + "Processor Type: " + row[10] + "," + "No of Processors: " + row[11] + ","
							+ "Timestamp: " + row[12] + " \n";

						// analysing rows to get maxRAM, minRAM, maxCpuLoad, minCpuLoad
						if (tempMinFreeRam > stoi(row[5]))
						{
							tempMinFreeRam = stoi(row[5]);
							time[0] = row[12];
						}

						if (tempLarFreeRam < stoi(row[5]))
						{
							tempLarFreeRam = stoi(row[5]);
							time[1] = row[12];
						}
						if (tempMinCpuLoad > stof(row[8]))
						{
							tempMinCpuLoad = stof(row[8]);
							time[2] = row[12];
						}

						if (tempMaxCpuLoad < stof(row[8]))
						{
							tempMaxCpuLoad = stof(row[8]);
							time[3] = row[12];
						}

					}
				}

				else
				{
					cout << "Query failed: " << mysql_error(conn) << endl;
				}

				s = s + "\n";
				s = s + "MINIMUM FREE RAM  :  " + to_string(tempMinFreeRam) + " At TimeStamp : " + time[0] + "\n" +"MAXIMUM FREE RAM  :  " + to_string(tempLarFreeRam) + " At TimeStamp : " + time[1]
					+ "\n\n" + "MINIMUN CPU LOAD  :  " + to_string(tempMinCpuLoad) + " At TimeStamp : " + time[2] + +"\n"  "MAXIMUM CPU LOAD  :  " + to_string(tempMaxCpuLoad) + " At TimeStamp : " + time[3] + "\n";
				s = s + "\n";
				int x = 0;
				for (; x < s.length(); x++)
				{
					buff[x] = s[x];
				}

				send(nClientSocket, buff, 5000, 0);
			}
		}
		else
		{
			//Data Integrity Check
			//Parsing data:

			vector<string> chdb;
			string tmp = "";
			string sto = buff;

			for (int i = 0; i < sto.length() - 1; i++)
			{
				if (buff[i] != ',')
				{
					tmp = tmp + buff[i];
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
			int chkS = 0;
			int checkSum = 0;

			chkS = stoi(chdb[0]);
			clientUid = chdb[1];
			hostname = chdb[2];

			username = chdb[3];
			totalRam = stoi(chdb[4]);

			availRam = stoi(chdb[5]);
			totalDiskSpace = stoi(chdb[6]);

			freeDiskSpace = stoi(chdb[7]);
			cpuLoad = stof(chdb[8]);

			processorArchitecture = chdb[9];
			noOfProcessors = stoi(chdb[10]);
			processorType = stoi(chdb[11]);
			timeStamp = chdb[12];

			//calculating Checksum
			parseData pD(clientUid, hostname, username, totalRam, availRam, totalDiskSpace, freeDiskSpace, cpuLoad, processorArchitecture, noOfProcessors, processorType, timeStamp, chkS);
			checkSum = pD.checkData();

			//Comparing Checksum
			if (chkS == checkSum)
				cout << "Data fetched is untampered" << endl;
			else
				cout << "Data is tampered" << endl;

			//Parsing data to store data into Database

			string cid;
			string HN;
			string UN;
			string TR;
			string AR;
			string TDS;
			string FDS;
			string CL;
			string CIT;
			string PA;
			string NOP;
			string PT;
			string TS;
			string chk;

			stringstream ss(buff);
			vector<string> db;
			string socketid = to_string(nClientSocket);
			db.push_back(socketid);

			string line1 = "";


			while (getline(ss, line1))
			{
				stringstream inputString(line1);

				getline(inputString, chk, ',');

				getline(inputString, cid, ',');
				db.push_back(cid);
				getline(inputString, HN, ',');
				db.push_back(HN);
				getline(inputString, UN, ',');
				db.push_back(UN);
				getline(inputString, TR, ',');
				db.push_back(TR);
				getline(inputString, AR, ',');
				db.push_back(AR);
				getline(inputString, TDS, ',');
				db.push_back(TDS);
				getline(inputString, FDS, ',');
				db.push_back(FDS);
				getline(inputString, CL, ',');
				db.push_back(CL);
				getline(inputString, PA, ',');
				db.push_back(PA);
				getline(inputString, NOP, ',');
				db.push_back(NOP);
				getline(inputString, PT, ',');
				db.push_back(PT);
				getline(inputString, TS, ',');
				db.push_back(TS);

				line1 = "";
			}

			// checks if DB connection is successful
			if (conn) {

				puts("Successful connection to database!");

				// vector <db> containing data is passed
				insert(conn, db);
			}
			else {
				puts("Connection to database has failed!");
			}
		}
	}
}