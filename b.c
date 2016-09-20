#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <bluetooth/bluetooth.h> 
#include <bluetooth/rfcomm.h>
/***********************************************BLUETOOTH SERVER CODE*******************************************/
int main()
{
    int pFile;
    struct sockaddr_rc myAddress, remoteAddress;
    int listenfd, client,cc;
    unsigned long count;
    char buf[20];

    pFile = open("/dev/motor", O_WRONLY);					//DEVICE FILE TO WHICH THE BLUETOOTH MODULE WILL WRITE
    listenfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    myAddress.rc_family = AF_BLUETOOTH;
    myAddress.rc_bdaddr = *BDADDR_ANY;
    myAddress.rc_channel = (uint8_t) 20;					//USING SOCKET 20
    bind(listenfd, (struct sockaddr *)&myAddress, sizeof(myAddress));
    listen(listenfd, 20);							//LISTENING TO SOCKET 20 FOR ANY CONNECTIONS
    int fsize = sizeof(remoteAddress);
    client = accept(listenfd, (struct sockaddr *)&remoteAddress, &fsize);	//ACCEPTS THE CONNECTION
    memset(buf, 0, sizeof(buf));
    cc = ba2str( & remoteAddress.rc_bdaddr, buf );
    memset(buf, 0, sizeof(buf));
    cc  = read(client, buf, sizeof(buf));					//READS THE DATA SEND BY CLIENT
    if( cc >= 0 )
    {
    count = strlen(buf);
    write(pFile,buf, count);							//WRITING TO THE DEVICE FILE
    memset(buf,0,sizeof(buf));
    }
    close(pFile);
    close(client);
    close(listenfd);
    return 0;
}
