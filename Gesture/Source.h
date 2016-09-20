#include "stdafx.h"
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <strsafe.h>
#include <intsafe.h>

ULONG hi(_In_ SOCKADDR_BTH ululRemoteBthAddr, _In_ int iMaxCxnCycles);
void bluetooth(char time[50]);