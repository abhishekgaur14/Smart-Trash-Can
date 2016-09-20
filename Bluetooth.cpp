#include "stdafx.h"
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <strsafe.h>
#include <intsafe.h>
#include "Source.h"

DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

#define CXN_BDADDR_STR_LEN                17   // 6 two-digit hex values plus 5 colons
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15
#define CXN_SUCCESS                       0
#define CXN_ERROR                         1
#define CXN_DEFAULT_LISTEN_BACKLOG        4

wchar_t g_szRemoteName[BTH_MAX_NAME_SIZE + 1] = { 0 };  // 1 extra for trailing NULL character
wchar_t g_szRemoteAddr[CXN_BDADDR_STR_LEN + 1] = L"00:03:19:50:28:AD"; // 1 extra for trailing NULL character
int  g_ulMaxCxnCycles = 1;

char time_send[50];
ULONG hi(_In_ SOCKADDR_BTH ululRemoteBthAddr, _In_ int iMaxCxnCycles = 1);
void bluetooth(char time[50]);

void bluetooth(char time[50])
{
	ULONG       ulRetCode = CXN_SUCCESS;
	WSADATA     WSAData = { 0 };
	SOCKADDR_BTH RemoteBthAddr = { 0 };
	strcpy_s(time_send, time);
	// Ask for Winsock version 2.2.
	if (CXN_SUCCESS == ulRetCode)
	{
		ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
		if (CXN_SUCCESS != ulRetCode)
			wprintf(L"-FATAL- | Unable to initialize Winsock version 2.2\n");

		int iAddrLen = sizeof(RemoteBthAddr);
		ulRetCode = WSAStringToAddressW(g_szRemoteAddr, AF_BTH, NULL, (LPSOCKADDR)&RemoteBthAddr, &iAddrLen);
		if (CXN_SUCCESS != ulRetCode)
			wprintf(L"-FATAL- | Unable to get address of the remote radio having formated address-string %s\n", g_szRemoteAddr);
		if (CXN_SUCCESS == ulRetCode)
			ulRetCode = hi(RemoteBthAddr, g_ulMaxCxnCycles);
	}
}

ULONG hi(_In_ SOCKADDR_BTH RemoteAddr, _In_ int iMaxCxnCycles)
{
	ULONG           ulRetCode = CXN_SUCCESS;
	int             iCxnCount = 0;
	SOCKET          LocalSocket = INVALID_SOCKET;
	SOCKADDR_BTH    SockAddrBthServer = RemoteAddr;

	if (CXN_SUCCESS == ulRetCode)
	{
		SockAddrBthServer.addressFamily = AF_BTH;
		SockAddrBthServer.serviceClassId = g_guidServiceClass;
		SockAddrBthServer.port = 20;

		for (iCxnCount = 0; (0 == ulRetCode) && (iCxnCount < iMaxCxnCycles || iMaxCxnCycles == 0); iCxnCount++)
		{
			wprintf(L"\n");

			LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
			if (INVALID_SOCKET == LocalSocket)
			{
				wprintf(L"Cannot Define Socket");
				ulRetCode = CXN_ERROR;
				break;
			}

			if (SOCKET_ERROR == connect(LocalSocket, (struct sockaddr *) &SockAddrBthServer, sizeof(SOCKADDR_BTH)))
			{
				wprintf(L"Cannot Connect");
				ulRetCode = CXN_ERROR;
				break;
			}

			if (SOCKET_ERROR == send(LocalSocket, time_send, sizeof(time_send), 0))
			{
				wprintf(L"Send Does Not Work");
				ulRetCode = CXN_ERROR;
				break;
			}

			if (SOCKET_ERROR == closesocket(LocalSocket))
			{
				wprintf(L"Cannot Close The Socket");
				ulRetCode = CXN_ERROR;
				break;
			}

			LocalSocket = INVALID_SOCKET;

		}
	}

	if (INVALID_SOCKET != LocalSocket) {
		closesocket(LocalSocket);
		LocalSocket = INVALID_SOCKET;
	}
	return(ulRetCode);
