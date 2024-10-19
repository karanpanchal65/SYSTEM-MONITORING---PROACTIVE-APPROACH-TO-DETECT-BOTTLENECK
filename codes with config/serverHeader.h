#include<iostream>
#include<winsock.h>
#include<vector>
#include<string>
#include <Windows.h>
#include <mysql.h>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include<type_traits>
#include<cstdint>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class server {

    int qstate;

public:

    void insert(MYSQL*, vector<string>&);                    // This function is used for deletion from the DB

    void ProcessNewMessage(int, MYSQL*, int[]);             // function where data tranfer occurs

    void ProcessNewRequest(MYSQL*, fd_set&, int, int[]);   //  This function is used only to accept new request from Clients no data tranfer

    void deletedb(MYSQL*, string, int);                    // This function is used for insertion into the DB
};

class parseData {

    string cliUid;
    string hstName;
    string usrName;
    int totlRam;
    int avlblRam;
    int totlDiskSpace;
    int freeDiskSpace;
    float cpuLd;
    //int cpuIdTime;
    string prcssArcht;
    int noOfPrc;
    int prcssType;
    string timeStmp;
    int checkS;

public:

    parseData(
        string cuid,
        string hsN,
        string usN,
        int totRam,
        int avRam,
        int tDskSpce,
        int fDskSpce,
        float cpuL,
        string prcArch,
        int nOP,
        int prcType,
        string timstmp,
        int chkS
    )
    {
        cliUid = cuid;
        hstName = hsN;
        usrName = usN;
        totlRam = totRam;
        avlblRam = avRam;
        totlDiskSpace = tDskSpce;
        freeDiskSpace = fDskSpce;
        cpuLd = cpuL;
        prcssArcht = prcArch;
        noOfPrc = nOP;
        prcssType = prcType;
        timeStmp = timstmp;
        checkS = chkS;
    }


    int checkData() {

        int cS = 0;
        int temp = 0;

        for (int i = 0; i < cliUid.length(); i++)
        {
            temp = '|' - cliUid[i];
            cS = temp ^ cS;
        }
        temp = 0;

        for (int i = 0; i < hstName.length(); i++)
        {
            temp = '|' - hstName[i];
            cS = temp ^ cS;
        }
        temp = 0;

        for (int i = 0; i < usrName.length(); i++)
        {
            temp = '|' - usrName[i];
            cS = temp ^ cS;
        }
        temp = 0;

        cS += totlRam;
        cS += avlblRam;
        cS += totlDiskSpace;
        cS += freeDiskSpace;
        cS += cpuLd;

        for (int i = 0; i < prcssArcht.length(); i++)
        {
            temp = '|' - prcssArcht[i];
            cS = temp ^ cS;
        }
        temp = 0;

        cS += noOfPrc;
        cS += prcssType;

        for (int i = 0; i < timeStmp.length(); i++)
        {
            temp = '|' - timeStmp[i];
            cS = temp ^ cS;
        }
        temp = 0;

        return cS;
    }
};

class Config {
public:
    int PORT;
    int memSize;
};
