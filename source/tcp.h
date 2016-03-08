//-----------------------------------------------------------------------------
//  IFCB Project
//	tcp.h
//	Martin Cooper			martin@mcdesign.ca
//-----------------------------------------------------------------------------
#pragma once

#define TCP_CMD_LEN			100
#define TCP_REPORT_LEN		200
#define TCP_RX_BUFLEN		256

// Function return codes
#define TCP_NO_ERROR		0
#define TCP_SOCKET_CLOSED	-1
#define TCP_WAITING			-2
#define TCP_SEND_FAILED		-3

// TCP command response codes
#define TCP_OK					'1'
#define TCP_BAD_COMMAND			'2'
#define TCP_RUNNING				'3'
#define TCP_STOPPED				'4'

void CloseTcp(void);
void HandleTcp(void);
void SendTcpMessage(char *message);
