//-----------------------------------------------------------------------------
//  IFCB Project
//	tcp.cpp
//	Martin Cooper			martin@mcdesign.ca
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <winsock2.h>
#include <conio.h>
#include <iostream>
#include "tcp.h"
#include "config.h"
#include "IfcbDlg.h"
#include "Fluids.h"
#include "FileIo.h"
using namespace std;

static SOCKET	sServer;
static SOCKET	sClientSocket;
static CString	debugStr;

//-----------------------------------------------------------------------------
void SendTcpMessage(char *message) {

	int result = send(sClientSocket, message, (int)strlen(message), 0);
	if (result == SOCKET_ERROR) {
		debugStr.Format(_T("TCP send returned error %d\r\n"), WSAGetLastError());
		ERROR_MESSAGE_EXT(debugStr);
	}
}

//-----------------------------------------------------------------------------
static int CreateServer(void) {

	debugStr.Format(_T("Starting up TCP server on port %d\r\n"), tcpPort);
	DEBUG_MESSAGE_EXT(debugStr);

	WSADATA wsaData;			// WSADATA is a struct that is filled up by the call to WSAStartup
	sockaddr_in local;			// The sockaddr_in specifies the address of the socket for TCP/IP sockets.

    // initialise the program for calling WinSock.
    if (WSAStartup(0x101, &wsaData) != 0) {
		debugStr.Format(_T("WSAStartup returned error %d\r\n"), WSAGetLastError());
		ERROR_MESSAGE_EXT(debugStr);
		return false;
	}

    // Now populate the sockaddr_in structure
	local.sin_family = AF_INET;						// Address family
	local.sin_addr.s_addr = INADDR_ANY;				// Wild card IP address
	local.sin_port = htons(tcpPort);				// port to use
	
	sServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);		// the socket function creates our SOCKET
	unsigned long poot = 42;
	if (ioctlsocket(sServer, FIONBIO, &poot)) {		// poot != 0 sets socket to nonblocking mode
		ERROR_MESSAGE_EXT(_T("ioctlsocket returned error\r\n"));
		return false;
	}
	if (sServer == INVALID_SOCKET) {
		debugStr.Format(_T("socket() returned error %d\r\n"), WSAGetLastError());
		ERROR_MESSAGE_EXT(debugStr);
		return false;
	}

    // bind links the socket just created with the sockaddr_in structure
	if (bind(sServer, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		debugStr.Format(_T("bind() returned error %d\r\n"), WSAGetLastError());
		ERROR_MESSAGE_EXT(debugStr);
		return false;
	}

    return true;
}

//-----------------------------------------------------------------------------
//	returns 0 when a request is succesfully accepted, > 0 on error, < 0 for other non-error state
//-----------------------------------------------------------------------------
static int ListenForClient(void) {

	int error;

	// listen for incoming connections from clients. The second arg is the backlog
    if (listen(sServer, 2) == SOCKET_ERROR) {
		error = WSAGetLastError();
		debugStr.Format(_T("listen() returned error %d\r\n"), error);
		ERROR_MESSAGE_EXT(debugStr);
		return error;
	}

	// create the client
	sClientSocket = accept(sServer, NULL, NULL);
 	if (sClientSocket == INVALID_SOCKET) {
		error =  WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			debugStr.Format(_T("listen() returned error %d\r\n"), error);
			ERROR_MESSAGE_EXT(debugStr);
		} else
			error = TCP_WAITING;				// get here if there were no client requests
	} else {
		DEBUG_MESSAGE_EXT(_T("TCP client connected\r\n"));
		closesocket(sServer);			// No longer need server socket
		error = TCP_NO_ERROR;
	}

	return error;
}

//-----------------------------------------------------------------------------
static int GetMessage() {

	int result;
	int ret = TCP_NO_ERROR;
	static char command[TCP_RX_BUFLEN];
	static int nChars = 0;


	result = recv(sClientSocket, command + nChars, TCP_RX_BUFLEN, 0);
	if (result > 0) {											// received character(s)
		ret = TCP_NO_ERROR;
	} else if (result == 0) {
		DEBUG_MESSAGE_EXT(_T("Connection is closed\r\n"));
		ret = TCP_SOCKET_CLOSED;
	} else {													// error
		ret = WSAGetLastError();
		if (ret == WSAEWOULDBLOCK)								// rev() returns this even if the socket is closed
			ret = TCP_WAITING;
		else {
			debugStr.Format(_T("recv() returned error %d\r\n"), ret);
			ERROR_MESSAGE_EXT(debugStr);
		}
	}

	if (ret != TCP_NO_ERROR)									// exit if there's no characters
		return ret;

	ret = TCP_WAITING;
	for (int i = 0; i < result; i++) {							// look through the received characters
		if (command[nChars + i] == '\r') {
			ret = TCP_NO_ERROR;
			break;
		}
	}
	nChars += result;
	if (ret == TCP_WAITING)
		return ret;

	// got '\r' so terminate the string
	command[nChars] = '\0';

	// now act on the command
	CString str;
	switch (command[0]) {
		case 'A':										// biocide
			FluidicsRoutine(FLUIDICS_AZIDE, 0);
			break;

		case 'F':										// FlushSampleTube (e.g., detergent)
			FluidicsRoutine(FLUIDICS_FLUSH_SAMPLETUBE, 0);
			break;

		case 'P':										// PrimeSampleTube
			FluidicsRoutine(FLUIDICS_PRIME_SAMPLETUBE, 0);
			break;

		case 'Q':
			FluidicsRoutine(FLUIDICS_REPLACEFROMSEC, 0);
			break;

		// beads - format Bx where x = 1 or 0
		// toggles the beads button but does not start a run
		// returns the original command
		case 'B':										
			manualBeads = command[1] == '1';
			IfcbHandle->TabContainer.tabPages[fluidsTab]->OnShowTab();
			break;

		case 'C':										// clorox
			FluidicsRoutine(FLUIDICS_CLOROX, 0);
			break;

		// start - format S
		// returns S1 if starts successfully or S0 if not
		case 'S':										
			if (!IfcbHandle->grabbing)					// ignore if already running
				IfcbHandle->Run();
			sprintf_s(command + 1, TCP_RX_BUFLEN, "%c\r\n", IfcbHandle->grabbing ? '1' : '0');
			SendTcpMessage(command);					// send reponse;
			break;

		// stop - format T
		// returns T1 if still running or T0 if stopped
		case 'T':										// stop
			if (IfcbHandle->grabbing)					// ignore if stopped
				IfcbHandle->Stop();
			sprintf_s(command + 1, TCP_RX_BUFLEN, "%c\r\n", IfcbHandle->grabbing ? '1' : '0');
			SendTcpMessage(command);					// send reponse;
			break;

		case 'U':
			sprintf_s(command, TCP_RX_BUFLEN, "Manual beads %s\r\n", manualBeads ? "ON" : "OFF");
			SendTcpMessage(command);

			IfcbHandle->CreateSyringeNumString(&str);
			str += "\r\n";
			CStringToChar(command, &str);
			SendTcpMessage(command);

			if (fluidSyncBusy) {
				strcpy_s(command, TCP_RX_BUFLEN, "Fluidics SYNC operation in progress\r\n");
				SendTcpMessage(command);
			}
			break;

		case 'Z':										// shutdown windows
			if (IfcbHandle->grabbing) {					// ignore if running
				command[1] = TCP_RUNNING;
				SendTcpMessage(command);				// send reponse;
			} else {
				command[1] = TCP_OK;
				SendTcpMessage(command);				// send reponse;
				IfcbHandle->ShutdownWindows();
			}
			break;

		default:										// error
			command[1] = TCP_BAD_COMMAND;
			SendTcpMessage(command);					// send reponse;
			break;
	}
	nChars = 0;											// prepare for next command

	return ret;
}

//-----------------------------------------------------------------------------
void HandleTcp(void) {

	static enum TCP_STATE {
		TCP_IDLE,					// 0
		TCP_CREATE_SERVER,			// 1
		TCP_LISTEN,					// 2
		TCP_CONNECTED,				// 3
		TCP_DEAD,					// 4
		TCP_CLEANUP					// 5
	} sTcpState = TCP_IDLE;
	int ret;

	switch (sTcpState) {
		case TCP_IDLE:
			sTcpState = TCP_CREATE_SERVER;
			break;

		case TCP_CREATE_SERVER:
			if (CreateServer())
				sTcpState = TCP_LISTEN;
			else
				sTcpState = TCP_DEAD;
			break;

		case TCP_LISTEN:
			ret = ListenForClient();
			if (ret == 0)											// connection request received
				sTcpState = TCP_CONNECTED;
			else if (ret == TCP_WAITING)							// still waiting for request
				sTcpState = TCP_LISTEN;
			else													// error
				sTcpState = TCP_CLEANUP;
			break;

		case TCP_CONNECTED:
			// GetMessage gets the message and deals with it
			ret = GetMessage();
			if ((ret != TCP_WAITING) && (ret != TCP_NO_ERROR))		// error
				sTcpState = TCP_CLEANUP;
			break;

		case TCP_CLEANUP:
			closesocket(sServer);
			closesocket(sClientSocket);
			WSACleanup();
			sTcpState = TCP_IDLE;
			break;

		case TCP_DEAD:
			break;
	}
}

//-----------------------------------------------------------------------------
void CloseTcp(void) {

    closesocket(sServer);
    WSACleanup();
}

