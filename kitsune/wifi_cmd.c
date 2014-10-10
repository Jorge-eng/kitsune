#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "uartstdio.h"

#include "wlan.h"
#include "socket.h"
#include "simplelink.h"
#include "protocol.h"

#include "ota_api.h"

#include "wifi_cmd.h"

#define ROLE_INVALID (-5)

int sl_mode = ROLE_INVALID;

unsigned int sl_status = 0;

#include "rom_map.h"
#include "prcm.h"
#include "utils.h"

#include "hw_memmap.h"
#include "rom_map.h"
#include "gpio.h"

SyncResponse_Alarm alarm;

void mcu_reset()
{
#define SLOW_CLK_FREQ           (32*1024)
    //
    // Configure the HIB module RTC wake time
    //
    MAP_PRCMHibernateIntervalSet(5 * SLOW_CLK_FREQ);
    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

    //DBG_PRINT("HIB: Entering HIBernate...\n\r");
    UtilsDelay(80000);
    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}

#define SL_STOP_TIMEOUT                 (30)
void nwp_reset() {
    sl_WlanSetMode(ROLE_STA);
    sl_Stop(SL_STOP_TIMEOUT);
    sl_mode = sl_Start(NULL, NULL, NULL);
}

#include "ota_usr.h"


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    // Unused in this application
}

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    //
    // This application doesn't work w/ socket - Events are not expected
    //
}

//****************************************************************************
//
//!    \brief This function handles WLAN events
//!
//! \param  pSlWlanEvent is the event passed to the handler
//!
//! \return None
//
//****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent) {
    switch (pSlWlanEvent->Event) {
#if 0 //todo bring this back after ti realises they've mucked it up
    case SL_WLAN_SMART_CONFIG_START_EVENT:
        /* SmartConfig operation finished */
        /*The new SSID that was acquired is: pWlanEventHandler->EventData.smartConfigStartResponse.ssid */
        /* We have the possiblity that also a private token was sent to the Host:
         *  if (pWlanEventHandler->EventData.smartConfigStartResponse.private_token_len)
         *    then the private token is populated: pWlanEventHandler->EventData.smartConfigStartResponse.private_token
         */
        UARTprintf("SL_WLAN_SMART_CONFIG_START_EVENT\n\r");
        break;
#endif
    case SL_WLAN_SMART_CONFIG_STOP_EVENT:
        UARTprintf("SL_WLAN_SMART_CONFIG_STOP_EVENT\n\r");
        break;
    case SL_WLAN_CONNECT_EVENT:
        UARTprintf("SL_WLAN_CONNECT_EVENT\n\r");
        sl_status |= CONNECT;
        break;
    case SL_WLAN_DISCONNECT_EVENT:
        UARTprintf("SL_WLAN_DISCONNECT_EVENT\n\r");
        sl_status &= ~CONNECT;
        sl_status &= ~HAS_IP;
        break;
    default:
        break;
    }
}

//****************************************************************************
//
//!    \brief This function handles events for IP address acquisition via DHCP
//!           indication
//!
//! \param  pNetAppEvent is the event passed to the Handler
//!
//! \return None
//
//****************************************************************************

int Cmd_led(int argc, char *argv[]);

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {

    switch (pNetAppEvent->Event) {
	case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
	case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
		UARTprintf("SL_NETAPP_IPV4_ACQUIRED\n\r");
		{
			int seed = (unsigned) PRCMSlowClkCtrGet();
			UARTprintf("seeding %d\r\n", seed);
			srand(seed); //seed with low bits of lf clock when connecting(not sure when it happens, gives some more entropy).
		}

        Cmd_led(0,0);

		sl_status |= HAS_IP;
		break;

	case SL_NETAPP_IP_LEASED_EVENT:
		sl_status |= IP_LEASED;
        break;
    default:
        break;
    }
}

void antsel(unsigned char a)
{
    if(a == 1)
    {
         MAP_GPIOPinWrite(GPIOA3_BASE, 0xC, 0x8);
    }
    else if(a == 2)
    {
        MAP_GPIOPinWrite(GPIOA3_BASE, 0xC, 0x4);
    }
    return;
}


int Cmd_antsel(int argc, char *argv[]) {
    if (argc != 2) {
        UARTprintf( "usage: antsel <1=IFA or 2=chip>\n\r");
        return -1;
    }
    antsel( *argv[1] ==  '1' ? 1 : 2 );

    return 0;
}

int Cmd_disconnect(int argc, char *argv[]) {

    _i16 del_ret = sl_WlanProfileDel(0xFF);

	sl_WlanDisconnect();
    return (0);
}
int Cmd_connect(int argc, char *argv[]) {
    SlSecParams_t secParams;

    if (argc != 4) {
        UARTprintf(
                "usage: connect <ssid> <key> <security: 0=open, 1=wep, 2=wpa>\n\r");
    }

    secParams.Key = (_i8*)argv[2];
    secParams.KeyLen = strlen(argv[2]);
    secParams.Type = atoi(argv[3]);

    sl_WlanConnect((_i8*)argv[1], strlen(argv[1]), NULL, &secParams, 0);
    sl_WlanProfileAdd((_i8*)argv[1], strlen(argv[1]), NULL, &secParams, NULL, 0, 0 );
    return (0);
}

int Cmd_status(int argc, char *argv[]) {
    unsigned char ucDHCP = 0;
    unsigned char len = sizeof(SlNetCfgIpV4Args_t);
    //
    // Get IP address
    //    unsigned char len = sizeof(_NetCfgIpV4Args_t);
    SlNetCfgIpV4Args_t ipv4 = { 0 };

    sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO, &ucDHCP, &len,
            (unsigned char *) &ipv4);
    //
    // Send the information
    //
    UARTprintf("%x ip 0x%x submask 0x%x gateway 0x%x dns 0x%x\n\r", sl_status,
            ipv4.ipV4, ipv4.ipV4Mask, ipv4.ipV4Gateway, ipv4.ipV4DnsServer);
    return 0;
}

// callback routine
void pingRes(SlPingReport_t* pUARTprintf) {
    // handle ping results
    UARTprintf(
            "Ping tx:%d rx:%d min time:%d max time:%d avg time:%d test time:%d\n",
            pUARTprintf->PacketsSent, pUARTprintf->PacketsReceived,
            pUARTprintf->MinRoundTime, pUARTprintf->MaxRoundTime, pUARTprintf->AvgRoundTime,
            pUARTprintf->TestTime);
}

int Cmd_ping(int argc, char *argv[]) {
    static SlPingReport_t UARTprintf;
    SlPingStartCommand_t pingCommand;

    pingCommand.Ip = SL_IPV4_VAL(192, 3, 116, 75); // destination IP address is my router's IP
    pingCommand.PingSize = 32;                     // size of ping, in bytes
    pingCommand.PingIntervalTime = 100;   // delay between pings, in miliseconds
    pingCommand.PingRequestTimeout = 1000; // timeout for every ping in miliseconds
    pingCommand.TotalNumberOfAttempts = 3; // max number of ping requests. 0 - forever
    pingCommand.Flags = 1;                        // UARTprintf after each ping

    sl_NetAppPingStart(&pingCommand, SL_AF_INET, &UARTprintf, pingRes);
    return (0);
}

unsigned long unix_time() {
    char buffer[48];
    int rv = 0;
    SlSockAddr_t sAddr;
    SlSockAddrIn_t sLocalAddr;
    int iAddrSize;
    unsigned long long ntp;
    unsigned long ipaddr;
    int sock;

    SlTimeval_t tv;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    tv.tv_sec = 2;             // Seconds
    tv.tv_usec = 0;             // Microseconds. 10000 microseconds resolution
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // Enable receive timeout

    if (sock < 0) {
        UARTprintf("Socket create failed\n\r");
        return 0;
    }
    UARTprintf("Socket created\n\r");

//
    // Send a query ? to the NTP server to get the NTP time
    //
    memset(buffer, 0, sizeof(buffer));

#define NTP_SERVER "pool.ntp.org"
    if (!(rv = gethostbyname(NTP_SERVER, strlen(NTP_SERVER), &ipaddr, AF_INET))) {
        UARTprintf(
                "Get Host IP succeeded.\n\rHost: %s IP: %d.%d.%d.%d \n\r\n\r",
                NTP_SERVER, SL_IPV4_BYTE(ipaddr, 3), SL_IPV4_BYTE(ipaddr, 2),
                SL_IPV4_BYTE(ipaddr, 1), SL_IPV4_BYTE(ipaddr, 0));
    } else {
        UARTprintf("failed to resolve ntp addr rv %d\n", rv);
        close(sock);
        return 0;
    }

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 0x00;
    sAddr.sa_data[1] = 0x7B;    // UDP port number for NTP is 123
    sAddr.sa_data[2] = (char) ((ipaddr >> 24) & 0xff);
    sAddr.sa_data[3] = (char) ((ipaddr >> 16) & 0xff);
    sAddr.sa_data[4] = (char) ((ipaddr >> 8) & 0xff);
    sAddr.sa_data[5] = (char) (ipaddr & 0xff);

    buffer[0] = 0b11100011;   // LI, Version, Mode
    buffer[1] = 0;     // Stratum, or type of clock
    buffer[2] = 6;     // Polling Interval
    buffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    buffer[12] = 49;
    buffer[13] = 0x4E;
    buffer[14] = 49;
    buffer[15] = 52;

    UARTprintf("Sending request\n\r\n\r");
    rv = sendto(sock, buffer, sizeof(buffer), 0, &sAddr, sizeof(sAddr));
    if (rv != sizeof(buffer)) {
        UARTprintf("Could not send SNTP request\n\r\n\r");
        close(sock);
        return 0;    // could not send SNTP request
    }

    //
    // Wait to receive the NTP time from the server
    //
    iAddrSize = sizeof(SlSockAddrIn_t);
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = 0;
    sLocalAddr.sin_addr.s_addr = 0;
    bind(sock, (SlSockAddr_t *) &sLocalAddr, iAddrSize);

    UARTprintf("receiving reply\n\r\n\r");

    rv = recvfrom(sock, buffer, sizeof(buffer), 0, (SlSockAddr_t *) &sLocalAddr,
            (SlSocklen_t*) &iAddrSize);
    if (rv <= 0) {
        UARTprintf("Did not receive\n\r");
        close(sock);
        return 0;
    }

    //
    // Confirm that the MODE is 4 --> server
    if ((buffer[0] & 0x7) != 4)    // expect only server response
            {
        UARTprintf("Expecting response from Server Only!\n\r");
        close(sock);
        return 0;    // MODE is not server, abort
    } else {
        //
        // Getting the data from the Transmit Timestamp (seconds) field
        // This is the time at which the reply departed the
        // server for the client
        //
        ntp = buffer[40];
        ntp <<= 8;
        ntp += buffer[41];
        ntp <<= 8;
        ntp += buffer[42];
        ntp <<= 8;
        ntp += buffer[43];

        ntp -= 2208988800UL;

        close(sock);
    }
    return (unsigned long) ntp;
}

int Cmd_time(int argc, char*argv[]) {

    UARTprintf(" time is %u \n", unix_time());

    return 0;
}

int Cmd_mode(int argc, char*argv[]) {
    int ap = 0;
    if (argc != 2) {
        UARTprintf("mode <1=ap 0=station>\n");
    }
    ap = atoi(argv[1]);
    if (ap && sl_mode != ROLE_AP) {
        //Switch to AP Mode
        sl_WlanSetMode(ROLE_AP);
        sl_Stop(SL_STOP_TIMEOUT);
        sl_mode = sl_Start(NULL, NULL, NULL);
    }
    if (!ap && sl_mode != ROLE_STA) {
        //Switch to STA Mode
        nwp_reset();
    }

    return 0;
}

#include "crypto.h"
static uint8_t aes_key[AES_BLOCKSIZE + 1] = "1234567891234567";

void load_aes() {
	long DeviceFileHandle = -1;
	int RetVal, Offset;

	// read in aes key
	RetVal = sl_FsOpen("/cert/key.aes", FS_MODE_OPEN_READ, NULL,
			&DeviceFileHandle);
	if (RetVal != 0) {
		UARTprintf("failed to open aes key file\n");
	}

	Offset = 0;
	RetVal = sl_FsRead(DeviceFileHandle, Offset, (unsigned char *) aes_key,
			AES_BLOCKSIZE);
	if (RetVal != AES_BLOCKSIZE) {
		UARTprintf("failed to read aes key file\n");
	}
	aes_key[AES_BLOCKSIZE] = 0;
	UARTprintf("read key %s\n", aes_key);

	RetVal = sl_FsClose(DeviceFileHandle, NULL, NULL, 0);
}

/* protobuf includes */
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "envelope.pb.h"
#include "periodic.pb.h"
#include "audio_data.pb.h"

static SHA1_CTX sha1ctx;

static bool write_callback_sha(pb_ostream_t *stream, const uint8_t *buf,
        size_t count) {
    int fd = (intptr_t) stream->state;
    int i;

    SHA1_Update(&sha1ctx, buf, count);

    for (i = 0; i < count; ++i) {
        UARTprintf("%x", buf);
    }
    return send(fd, buf, count, 0) == count;
}

static bool read_callback_sha(pb_istream_t *stream, uint8_t *buf, size_t count) {
    int fd = (intptr_t) stream->state;
    int result,i;

    result = recv(fd, buf, count, 0);

    SHA1_Update(&sha1ctx, buf, count);

    for (i = 0; i < count; ++i) {
        UARTprintf("%x", buf);
    }

    if (result == 0)
        stream->bytes_left = 0; /* EOF */

    return result == count;
}


//WARNING not re-entrant! Only 1 of these can be going at a time!
pb_ostream_t pb_ostream_from_sha_socket(int fd) {
    pb_ostream_t stream =
            { &write_callback_sha, (void*) (intptr_t) fd, SIZE_MAX, 0 };
    return stream;
}

pb_istream_t pb_istream_from_sha_socket(int fd) {
    pb_istream_t stream = { &read_callback_sha, (void*) (intptr_t) fd, SIZE_MAX };
    return stream;
}

static int sock = -1;
static unsigned long ipaddr = 0;

#include "fault.h"

void UARTprintfFaults() {
#define minval( a,b ) a < b ? a : b
#define BUF_SZ 600
    size_t message_length;

    faultInfo * info = (faultInfo*)SHUTDOWN_MEM;

    if (info->magic == SHUTDOWN_MAGIC) {
        char buffer[BUF_SZ];
        if (sock > 0) {
            message_length = sizeof(faultInfo);
            snprintf(buffer, sizeof(buffer), "POST /in/morpheus/fault HTTP/1.1\r\n"
                    "Host: in.skeletor.com\r\n"
                    "Content-type: application/x-protobuf\r\n"
                    "Content-length: %d\r\n"
                    "\r\n", message_length);
            memcpy(buffer + strlen(buffer), info, sizeof(faultInfo));

            UARTprintf("sending faultdata\n\r\n\r");

            send(sock, buffer, strlen(buffer), 0);

            recv(sock, buffer, sizeof(buffer), 0);

            UARTprintf("Reply is:\n\r\n\r");
            buffer[127] = 0; //make sure it terminates..
            UARTprintf("%s", buffer);

            info->magic = 0;
        }
    }
}

int stop_connection() {
    close(sock);
    sock = -1;
    return sock;
}
int start_connection() {
    sockaddr sAddr;
    timeval tv;
    int rv;
    int sock_begin = sock;

    if (sock < 0) {
        sock = socket(AF_INET, SOCK_STREAM, SL_SEC_SOCKET);
        tv.tv_sec = 2;             // Seconds
        tv.tv_usec = 0;           // Microseconds. 10000 microseconds resolution
        setsockopt(sock, SOL_SOCKET, SL_SO_RCVTIMEO, &tv, sizeof(tv)); // Enable receive timeout

        #define SL_SSL_CA_CERT_FILE_NAME "/cert/ca.der"
        // configure the socket as SSLV3.0
        // configure the socket as RSA with RC4 128 SHA
        // setup certificate
        unsigned char method = SL_SO_SEC_METHOD_SSLV3;
        unsigned int cipher = SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA;
        if( sl_SetSockOpt(sock, SL_SOL_SOCKET, SL_SO_SECMETHOD, &method, sizeof(method) ) < 0 ||
            sl_SetSockOpt(sock, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &cipher, sizeof(cipher)) < 0 ||
            sl_SetSockOpt(sock, SL_SOL_SOCKET, \
                                   SL_SO_SECURE_FILES_CA_FILE_NAME, \
                                   SL_SSL_CA_CERT_FILE_NAME, \
                                   strlen(SL_SSL_CA_CERT_FILE_NAME))  < 0  )
        {
        UARTprintf( "error setting ssl options\r\n" );
        }
    }

    if (sock < 0) {
        UARTprintf("Socket create failed %d\n\r", sock);
        return -1;
    }
    UARTprintf("Socket created\n\r");

#define DATA_SERVER "dev-in.hello.is"
#if !LOCAL_TEST
    if (ipaddr == 0) {
        if (!(rv = gethostbyname(DATA_SERVER, strlen(DATA_SERVER), &ipaddr,
        SL_AF_INET))) {
            /*    UARTprintf(
             "Get Host IP succeeded.\n\rHost: %s IP: %d.%d.%d.%d \n\r\n\r",
             DATA_SERVER, SL_IPV4_BYTE(ipaddr, 3), SL_IPV4_BYTE(ipaddr, 2),
             SL_IPV4_BYTE(ipaddr, 1), SL_IPV4_BYTE(ipaddr, 0));
             */
        } else {
            UARTprintf("failed to resolve ntp addr rv %d\n");
            return -1;
        }
    }

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 1;    //port 443, ssl
    sAddr.sa_data[1] = 0xbb; //port 443, ssl
    sAddr.sa_data[2] = (char) ((ipaddr >> 24) & 0xff);
    sAddr.sa_data[3] = (char) ((ipaddr >> 16) & 0xff);
    sAddr.sa_data[4] = (char) ((ipaddr >> 8) & 0xff);
    sAddr.sa_data[5] = (char) (ipaddr & 0xff);
#else

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 0;//0xf;
    sAddr.sa_data[1] = 80;//0xa0; //4k
    sAddr.sa_data[2] = (char) () & 0xff);
    sAddr.sa_data[3] = (char) () & 0xff);
    sAddr.sa_data[4] = (char) () & 0xff);
    sAddr.sa_data[5] = (char) () & 0xff);

#endif

    //connect it up
    //UARTprintf("Connecting \n\r\n\r");
    if (sock > 0 && sock_begin < 0 && (rv = connect(sock, &sAddr, sizeof(sAddr)))) {
        UARTprintf("connect returned %d\n\r\n\r", rv);
        if (rv != SL_ESECSNOVERIFY) {
            UARTprintf("Could not connect %d\n\r\n\r", rv);
            return stop_connection();    // could not send SNTP request
        }
    }
    return 0;
}

//takes the buffer and reads <return value> bytes, up to buffer size
typedef int(*audio_read_cb)(char * buffer, int buffer_size);

//buffer needs to be at least 128 bytes...
int send_audio_wifi(char * buffer, int buffer_size, audio_read_cb arcb) {
    int send_length;
    int rv = 0;
    int message_length;
    unsigned char mac[6];
    unsigned char mac_len = 6;
#if 0
    mac[0] = 0xab;
    mac[1] = 0xcd;
    mac[2] = 0xab;
    mac[3] = 0xcd;
    mac[4] = 0xab;
    mac[5] = 0xcd;
#else
    sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &mac_len, mac);
#endif

    message_length = 110000;

    snprintf(buffer, buffer_size, "POST /audio/%x%x%x%x%x%x HTTP/1.1\r\n"
            "Host: in.skeletor.com\r\n"
            "Content-type: application/octet-stream\r\n"
            "Content-length: %d\r\n"
            "\r\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], message_length);
    send_length = strlen(buffer);

    UARTprintf("%s\n\r\n\r", buffer);

    //setup the connection
    if( start_connection() < 0 ) {
        UARTprintf("failed to start connection\n\r\n\r");
        return -1;
    }

    //UARTprintf("Sending request\n\r%s\n\r", buffer);
    rv = send(sock, buffer, send_length, 0);
    if ( rv <= 0) {
        UARTprintf("send error %d\n\r\n\r", rv);
        return stop_connection();
    }
    UARTprintf("sent %d\n\r\n\r", rv);

    while( message_length > 0 ) {
        int buffer_sent, buffer_read;
        buffer_sent = buffer_read = arcb( buffer, buffer_size );
        UARTprintf("read %d\n\r", buffer_read);

        while( buffer_read > 0 ) {
            send_length = minval(buffer_size, buffer_read);
            UARTprintf("attempting to send %d\n\r", send_length );

            rv = send(sock, buffer, send_length, 0);
            if (rv <= 0) {
                UARTprintf("send error %d\n\r", rv);
                return stop_connection();
            }
            UARTprintf("sent %d, %d left in buffer\n\r", rv, buffer_read);
            buffer_read -= rv;
        }
        message_length -= buffer_sent;
        UARTprintf("sent buffer, %d left\n\r\n\r", message_length);
    }

    memset(buffer, 0, buffer_size);

    //UARTprintf("Waiting for reply\n\r\n\r");
    rv = recv(sock, buffer, buffer_size, 0);
    if (rv <= 0) {
        UARTprintf("recv error %d\n\r\n\r", rv);
        return stop_connection();
    }
    UARTprintf("recv %d\n\r\n\r", rv);

    UARTprintf("Reply is:\n\r\n\r");
    buffer[127] = 0; //make sure it terminates..
    UARTprintf("%s", buffer);


    return 0;
}

#if 1
#define SIG_SIZE 32
#include "SyncResponse.pb.h"
unsigned long get_time();

int rx_data_pb(unsigned char * buffer, int buffer_size ) {
	AES_CTX aesctx;
	unsigned char * buf_pos = buffer;
	unsigned char sig[SIG_SIZE];
	unsigned char sig_test[SIG_SIZE];
	int i;
	int status;
	pb_istream_t stream;
	SyncResponse SyncResponse_data;
    memset(&SyncResponse_data, 0, sizeof(SyncResponse_data));

	//memset( aesctx.iv, 0, sizeof( aesctx.iv ) );

	UARTprintf("iv ");
	for (i = 0; i < AES_IV_SIZE; ++i) {
		aesctx.iv[i] = *buf_pos++;
		UARTprintf("%x", aesctx.iv[i]);
		if (buf_pos > (buffer + buffer_size)) {
			return -1;
		}
	}
	UARTprintf("\n");
	buffer_size -= AES_IV_SIZE;
	UARTprintf("sig");
	for (i = 0; i < SIG_SIZE; ++i) {
		sig[i] = *buf_pos++;
		UARTprintf("%x", sig[i]);
		if (buf_pos > (buffer + buffer_size)) {
			return -1;
		}
	}
	UARTprintf("\n");
	buffer_size -= SIG_SIZE;

	AES_set_key(&aesctx, aes_key, aesctx.iv, AES_MODE_128); //todo real key
	AES_convert_key(&aesctx);
	AES_cbc_decrypt(&aesctx, sig, sig, SIG_SIZE);

	SHA1_Init(&sha1ctx);
	SHA1_Update(&sha1ctx, buf_pos, buffer_size);
	//now verify sig
	SHA1_Final(sig_test, &sha1ctx);
	if (memcmp(sig, sig_test, SHA1_SIZE) != 0) {
		UARTprintf("signatures do not match\n");
		for (i = 0; i < SHA1_SIZE; ++i) {
			UARTprintf("%x", sig[i]);
		}
		UARTprintf("\nvs\n");
		for (i = 0; i < SHA1_SIZE; ++i) {
			UARTprintf("%x", sig_test[i]);
		}
		UARTprintf("\n");

		return -1; //todo uncomment
	}

	/* Create a stream that will read from our buffer. */
	stream = pb_istream_from_buffer(buf_pos, buffer_size);
	/* Now we are ready to decode the message! */

	UARTprintf("data ");
	status = pb_decode(&stream, SyncResponse_fields, &SyncResponse_data);
	UARTprintf("\n");

	/* Then just check for any errors.. */
	if (!status) {
		UARTprintf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
		return -1;
	}

	UARTprintf("Decoding success: %d %d %d %d %d\n",
					SyncResponse_data.has_acc_sampling_interval,
					SyncResponse_data.has_acc_scan_cyle,
					SyncResponse_data.has_alarm,
					SyncResponse_data.has_device_sampling_interval,
					SyncResponse_data.has_flash_action);

	if( SyncResponse_data.has_alarm ) {
		alarm = SyncResponse_data.alarm;

		if(alarm.has_start_time) {
			UARTprintf( "Got alarm %d to %d in %d minutes\n", alarm.start_time, alarm.end_time, (alarm.start_time - get_time())/60 );
		}
	}
	//now act on incoming data!
	return 0;
}
#endif


int matchhere(char *regexp, char *text);
/* matchstar: search for c*regexp at beginning of text */
int matchstar(int c, char *regexp, char *text)
{
    do {    /* a * matches zero or more instances */
        if (matchhere(regexp, text))
            return 1;
    } while (*text != '\n' && (*text++ == c || c == '.'));
    return 0;
}

/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text)
{
    if (regexp[0] == '\0')
        return 1;
    if (regexp[1] == '*')
        return matchstar(regexp[0], regexp+2, text);
    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\n';
    if (*text!='\n' && (regexp[0]=='.' || regexp[0]==*text))
        return matchhere(regexp+1, text+1);
    return 0;
}


/* match: search for regexp anywhere in text */
int match(char *regexp, char *text)
{
    if (regexp[0] == '^')
        return matchhere(regexp+1, text);
    do {    /* must look even if string is empty */
        if (matchhere(regexp, text))
            return 1;
    } while (*text++ != '\n');
    return 0;
}

//buffer needs to be at least 128 bytes...
int send_data_pb(char * buffer, int buffer_size, const pb_field_t fields[], const void *src_struct) {
    int send_length;
    int rv = 0;
    uint8_t sig[32]={0};

    size_t message_length;
    bool status;

    {
        pb_ostream_t stream = {0};
        status = pb_encode(&stream, fields, src_struct);
        message_length = stream.bytes_written + sizeof(sig) + AES_IV_SIZE;
        UARTprintf("message len %d sig len %d\n\r\n\r", stream.bytes_written, sizeof(sig));
    }

    snprintf(buffer, buffer_size, "POST /in/morpheus/pb2 HTTP/1.1\r\n"
            "Host: in.skeletor.com\r\n"
            "Content-type: application/x-protobuf\r\n"
            "Content-length: %d\r\n"
            "\r\n", message_length);
    send_length = strlen(buffer);

    //setup the connection
    if( start_connection() < 0 ) {
        UARTprintf("failed to start connection\n\r\n\r");
        return -1;
    }

    //UARTprintf("Sending request\n\r%s\n\r", buffer);
    rv = send(sock, buffer, send_length, 0);
    if (rv <= 0) {
        UARTprintf("send error %d\n\r\n\r", rv);
        return stop_connection();
    }
    UARTprintf("sent %d\n\r%s\n\r", rv, buffer);

    {
        pb_ostream_t stream;
        int i;

        //todo guard sha1ctx with semaphore...
        SHA1_Init(&sha1ctx);

        /* Create a stream that will write to our buffer. */
        stream = pb_ostream_from_sha_socket(sock);
        /* Now we are ready to encode the message! */

        UARTprintf("data ");
        status = pb_encode(&stream, fields, src_struct);
        UARTprintf("\n");

        /* Then just check for any errors.. */
        if (!status) {
            UARTprintf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return -1;
        }

        //now sign it
        SHA1_Final(sig, &sha1ctx);

        for (i = SHA1_SIZE; i < sizeof(sig); ++i) {
            sig[i] = (uint8_t)rand();
        }

        UARTprintf("SHA ");
        for (i = 0; i < sizeof(sig); ++i) {
            UARTprintf("%x", sig[i]);
        }
        UARTprintf("\n");

        AES_CTX aesctx;
        //memset( aesctx.iv, 0, sizeof( aesctx.iv ) );

        UARTprintf("iv ");
        for (i = 0; i < sizeof(aesctx.iv); ++i) {
            aesctx.iv[i] = (uint8_t)rand();
            UARTprintf("%x", aesctx.iv[i]);
        }
        UARTprintf("\n");
        rv = send(sock, aesctx.iv, AES_IV_SIZE, 0);
        if (rv != AES_IV_SIZE) {
            UARTprintf("Sending IV failed: %d\n", rv);
            return -1;
        }

        AES_set_key(&aesctx, aes_key, aesctx.iv, AES_MODE_128); //todo real key
        AES_cbc_encrypt(&aesctx, sig, sig, sizeof(sig));

        rv = send(sock, sig, sizeof(sig), 0);
        if (rv != sizeof(sig)) {
            UARTprintf("Sending SHA failed: %d\n", rv);
            return -1;
        }

        UARTprintf("sig ");
        for (i = 0; i < sizeof(sig); ++i) {
            UARTprintf("%x", sig[i]);
        }
        UARTprintf("\n");
    }
    memset(buffer, 0, buffer_size);

    //UARTprintf("Waiting for reply\n\r\n\r");
    rv = recv(sock, buffer, buffer_size, 0);
    if (rv <= 0) {
        UARTprintf("recv error %d\n\r\n\r", rv);
        return stop_connection();
    }
    UARTprintf("recv %d\n\r\n\r", rv);

    UARTprintf("Reply is:\n\r%s\n\r", buffer);
    {
		#define CL_HDR "Content-Length: "
		char * content = strstr(buffer, "\r\n\r\n") + 4;
		char * len_str = strstr(buffer, CL_HDR) + strlen(CL_HDR);
		int resp_ok = match("2..", buffer);
		int len;

		if (len_str != NULL) {
			len = atoi(len_str);
			if (resp_ok) {
				rx_data_pb((unsigned char*) content, len);
			} else {
				UARTprintf("Did not see http 2xx\n");
			}
		} else {
			UARTprintf("Failed to find length\n");
		}
	}

	UARTprintf("Send complete\n");

    //todo check for http response code 2xx

    //UARTprintfFaults();
    //close( sock ); //never close our precious socket

    return 0;
}
bool encode_pill_id(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
	periodic_data_pill_data_container * data = (periodic_data_pill_data_container*) arg;
    return pb_encode_tag(stream, PB_WT_STRING, field->tag) && pb_encode_string(stream, (uint8_t*) data->id, strlen(data->id));
}

#include "ble_cmd.h"
static bool _encode_encrypted_pilldata(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    const array_data* array_holder = (array_data*)(*arg);
    if(!array_holder)
    {
        return false;
    }

    if (!pb_encode_tag(stream, PB_WT_STRING, field->tag))
    {
        return false;
    }

    return pb_encode_string(stream, array_holder->buffer, array_holder->length);
}

bool encode_pill_data(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
	periodic_data_pill_data_container * data = *(periodic_data_pill_data_container**) arg;

	int i;
	if (xSemaphoreTake(pill_smphr, portMAX_DELAY)) {

		if( data->magic != PILL_MAGIC ) {
			return true; //nothing to encode
		}

		data->pill_data.deviceId.funcs.encode = encode_pill_id;
		data->pill_data.deviceId.arg = (void*) arg;

 		if (!pb_encode_tag(stream, PB_WT_STRING, field->tag))
        {
			return false;
        }

		{
			pb_ostream_t sizestream = { 0 };
			pb_encode(&sizestream, periodic_data_pill_data_fields,
					(const void*) &data->pill_data);

			if (!pb_encode_varint(stream, (uint64_t) sizestream.bytes_written))
				return false;
		}

		for (i = 0; data->magic == PILL_MAGIC && i < MAX_PILLS; ++i) {
            if(data->pill_data.motionDataEncrypted.arg && 
                NULL == data->pill_data.motionDataEncrypted.funcs.encode)
            {
                // Set the default encode function for encrypted motion data.
                data->pill_data.motionDataEncrypted.funcs.encode = _encode_encrypted_pilldata;
            }

			if (!pb_encode(stream, periodic_data_pill_data_fields,
					(const void*) &data->pill_data)) {
				return false;
			}
			++data;
		}
		xSemaphoreGive(pill_smphr);
	}
	return true;
}




bool encode_mac(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    unsigned char mac[6];
    unsigned char mac_len = 6;
#if 0
    mac[0] = 0xab;
    mac[1] = 0xcd;
    mac[2] = 0xab;
    mac[3] = 0xcd;
    mac[4] = 0xab;
    mac[5] = 0xcd;
#else
    sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &mac_len, mac);
#endif

    return pb_encode_tag(stream, PB_WT_STRING, field->tag) && pb_encode_string(stream, (uint8_t*) mac, mac_len);
}

bool encode_name(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    return pb_encode_tag(stream, PB_WT_STRING, field->tag)
            && pb_encode_string(stream, (uint8_t*) MORPH_NAME,
                    strlen(MORPH_NAME));
}

int send_periodic_data( data_t * data ) {
    char buffer[256];
    periodic_data msg;

    //build the message
    msg.firmware_version = 2;
    msg.dust = data->dust;
    msg.humidity = data->humid;
    msg.light = data->light;
    msg.light_variability = data->light_variability;
    msg.light_tonality = data->light_tonality;
    msg.temperature = data->temp;
    msg.unix_time = data->time;
    msg.name.funcs.encode = encode_name;
    msg.mac.funcs.encode = encode_mac;
    msg.pills.funcs.encode = encode_pill_data;
    msg.pills.arg = data->pill_list;

    int ret = send_data_pb(buffer, sizeof(buffer), periodic_data_fields, &msg);
    if(ret == 0)
    {
        // Release all the resource occupied by pill data, or
        // user can occupy the buffer forever by sending only one packet
        if (xSemaphoreTake(pill_smphr, portMAX_DELAY)) {
            int i;
            for (i = 0; i < MAX_PILLS; ++i) {
                if (data->pill_list[i].magic != PILL_MAGIC) {
                    // Slot already empty, skip.
                    continue;
                }

                if(data->pill_list[i].pill_data.motionDataEncrypted.arg)
                {
                    array_data* array_holder = data->pill_list[i].pill_data.motionDataEncrypted.arg;
                    if(array_holder->buffer)
                    {
                        vPortFree(array_holder->buffer);
                    }

                    vPortFree(array_holder);
                    data->pill_list[i].pill_data.motionDataEncrypted.arg = NULL;
                    data->pill_list[i].pill_data.motionDataEncrypted.funcs.encode = NULL;
                    data->pill_list[i].magic = 0;  // Release this slot.
                }
            }
            xSemaphoreGive(pill_smphr);
        }
    }

    return ret;
}


int Cmd_sl(int argc, char*argv[]) {

    unsigned char policyVal;

    //make sure we're in station mode
    if (sl_mode != ROLE_STA) {
        //Switch to STA Mode
        sl_WlanSetMode(ROLE_STA);
        sl_Stop(SL_STOP_TIMEOUT);
        sl_mode = sl_Start(NULL, NULL, NULL);
    }

    //sl_WlanProfileDel(WLAN_DEL_ALL_PROFILES);

    //set AUTO policy
    sl_WlanPolicySet( SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 0),
            &policyVal, 1 /*PolicyValLen*/);

    /* Start SmartConfig
     * This example uses the unsecured SmartConfig method
     */
    sl_WlanSmartConfigStart(0,                          //groupIdBitmask
            SMART_CONFIG_CIPHER_NONE,    //cipher
            0,                           //publicKeyLen
            0,                           //group1KeyLen
            0,                           //group2KeyLen
            NULL,                          //publicKey
            NULL,                          //group1Key
            NULL);                         //group2Key

    return 0;
}



#include "fatfs_cmd.h"

int audio_read_fn (char * buffer, int buffer_size) {
    memset(buffer, 0xabcd, buffer_size);

    return buffer_size;
}

int Cmd_audio_test(int argc, char *argv[]) {
    short audio[1024];
    send_audio_wifi( (char*)audio, sizeof(audio), audio_read_fn );
    return (0);
}

//radio test functions
#define FRAME_SIZE		1500
typedef enum
{
    LONG_PREAMBLE_MODE = 0,
    SHORT_PREAMBLE_MODE = 1,
    OFDM_PREAMBLE_MODE = 2,
    N_MIXED_MODE_PREAMBLE_MODE = 3,
    GREENFIELD_PREAMBLE_MODE = 4,
    MAX_NUM_PREAMBLE = 0xff
}RadioPreamble_e;

typedef enum
{
    RADIO_TX_CONTINUOUS = 1,
    RADIO_TX_PACKETIZED = 2,
    RADIO_TX_CW = 3,
    MAX_RADIO_TX_MODE = 0xff
}RadioTxMode_e;

typedef enum
{
    ALL_0_PATTERN = 0,
    ALL_1_PATTERN = 1,
    INCREMENTAL_PATTERN = 2,
    DECREMENTAL_PATTERN = 3,
    PN9_PATTERN = 4,
    PN15_PATTERN = 5,
    PN23_PATTERN = 6,
    MAX_NUM_PATTERN = 0xff
}RadioDataPattern_e;

int rawSocket;
uint8_t CurrentTxMode;

#define RADIO_CMD_BUFF_SIZE_MAX 100
#define DEV_VER_LENGTH			136
#define WLAN_DEL_ALL_PROFILES	255
#define CW_LOW_TONE			-25
#define CW_HIGH_TONE			25
#define CW_STOP					128

/**************************** Definitions for template frame ****************************/
#define RATE RATE_1M
#define FRAME_TYPE 0xC8											/* QOS data */
#define FRAME_CONTROL 0x01										/* TO DS*/
#define DURATION 0x00,0x00
#define RECEIVE_ADDR 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
#define TRANSMITTER_ADDR 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
#define DESTINATION_ADDR 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
#define FRAME_NUMBER   0x00, 0x00
#define QOS_CONTROL	0x00, 0x00

#define RA_OFFSET		4
#define TA_OFFSET		10
#define DA_OFFSET		16

uint8_t TemplateFrame[] = {
                   /*---- wlan header start -----*/
                   FRAME_TYPE,                          							/* version type and sub type */
                   FRAME_CONTROL,                       							/* Frame control flag*/
                   DURATION,                            							/* duration */
                   RECEIVE_ADDR,                        							/* Receiver Address */
                   TRANSMITTER_ADDR,                    						/* Transmitter Address */
                   DESTINATION_ADDR,                    						/* destination Address*/
                   FRAME_NUMBER,                        							/* frame number */
                   QOS_CONTROL											/* QoS control */
};

int Cmd_RadioStopRX(int argc, char*argv[])
{
	sl_WlanRxStatStop();

    vTaskDelay(30 / portTICK_RATE_MS);

	return sl_Close(rawSocket);
}

int RadioStartRX(int eChannel)
{
	struct SlTimeval_t timeval;
	uint8_t * DataFrame = (uint8_t*)pvPortMalloc( FRAME_SIZE );

	timeval.tv_sec =  0;             // Seconds
	timeval.tv_usec = 20000;             // Microseconds. 10000 microseconds resoultion

	sl_WlanRxStatStart();

	rawSocket = sl_Socket(SL_AF_RF, SL_SOCK_RAW, eChannel);

	if (rawSocket < 0)
	{
		return -1;
	}

	sl_SetSockOpt(rawSocket,SL_SOL_SOCKET,SL_SO_RCVTIMEO, &timeval, sizeof(timeval));    // Enable receive timeout

	sl_Recv(rawSocket, DataFrame, 1470, 0);

	vPortFree( DataFrame );
	return 0;
}

int Cmd_RadioStartRX(int argc, char*argv[])
{
	int channel;
	if(argc!=2) {
		UARTprintf("startrx <channel>\n");
	}
	channel = atoi(argv[1]);
	return RadioStartRX(channel);
}

int RadioGetStats(unsigned int *validPackets, unsigned int *fcsPackets,unsigned int *plcpPackets, int16_t *avgRssiMgmt, int16_t  *avgRssiOther, uint16_t * pRssiHistogram, uint16_t * pRateHistogram)
{

	SlGetRxStatResponse_t rxStatResp;

	sl_WlanRxStatGet(&rxStatResp, 0);

	*validPackets = rxStatResp.ReceivedValidPacketsNumber;
	*fcsPackets = rxStatResp.ReceivedFcsErrorPacketsNumber;
	*plcpPackets = rxStatResp.ReceivedPlcpErrorPacketsNumber;
	*avgRssiMgmt = rxStatResp.AvarageMgMntRssi;
	*avgRssiOther = rxStatResp.AvarageDataCtrlRssi;

	memcpy(pRssiHistogram, rxStatResp.RssiHistogram, sizeof(unsigned short) * SIZE_OF_RSSI_HISTOGRAM);

	memcpy(pRateHistogram, rxStatResp.RateHistogram, sizeof(unsigned short) * NUM_OF_RATE_INDEXES);

    return 0;

}

int Cmd_RadioGetStats(int argc, char*argv[])
{
	unsigned int valid_packets, fcs_packets, plcp_packets;
	int16_t avg_rssi_mgmt, avg_rssi_other;
	uint16_t * rssi_histogram;
	uint16_t * rate_histogram;
	int i;

	rssi_histogram = (uint16_t*)pvPortMalloc(sizeof(unsigned short) * SIZE_OF_RSSI_HISTOGRAM);
	rate_histogram = (uint16_t*)pvPortMalloc(sizeof(unsigned short) * NUM_OF_RATE_INDEXES);

	RadioGetStats(&valid_packets, &fcs_packets, &plcp_packets, &avg_rssi_mgmt, &avg_rssi_other, rssi_histogram, rate_histogram);

	UARTprintf( "valid_packets %d\n", valid_packets );
	UARTprintf( "fcs_packets %d\n", fcs_packets );
	UARTprintf( "plcp_packets %d\n", plcp_packets );
	UARTprintf( "avg_rssi_mgmt %d\n", avg_rssi_mgmt );
	UARTprintf( "acg_rssi_other %d\n", avg_rssi_other );

	UARTprintf("rssi histogram\n");
	for (i = 0; i < SIZE_OF_RSSI_HISTOGRAM; ++i) {
		UARTprintf("%d\n", rssi_histogram[i]);
	}
	UARTprintf("rate histogram\n");
	for (i = 0; i < NUM_OF_RATE_INDEXES; ++i) {
		UARTprintf("%d\n", rate_histogram[i]);
	}

	vPortFree(rssi_histogram);
	vPortFree(rate_histogram);
	return 0;
}

int RadioStopTX(RadioTxMode_e eTxMode)
{
	int retVal;

	if (RADIO_TX_PACKETIZED == eTxMode)
	{
		retVal = 0;
	}

	if (RADIO_TX_CW == eTxMode)
	{
		sl_Send(rawSocket, NULL, 0, CW_STOP);
		vTaskDelay(30 / portTICK_RATE_MS);
		retVal = sl_Close(rawSocket);
	}

	if (RADIO_TX_CONTINUOUS == eTxMode)
	{
		retVal = sl_Close(rawSocket);
	}

	return retVal;
//	radioSM = RADIO_IDLE;
//
//	return sl_Close(rawSocket);
}

/* Note: the followings are not yet supported:
1) Power level
2) Preamble type
3) CW	*/
//int32_t RadioStartTX(RadioTxMode_e eTxMode, RadioPowerLevel_e ePowerLevel, Channel_e eChannel, RateIndex_e eRate, RadioPreamble_e ePreamble, RadioDataPattern_e eDataPattern, uint16_t size, uint32_t delay, uint8_t * pDstMac)
int32_t RadioStartTX(RadioTxMode_e eTxMode, uint8_t powerLevel_Tone, int eChannel, SlRateIndex_e eRate, RadioPreamble_e ePreamble, RadioDataPattern_e eDataPattern, uint16_t size, uint32_t delay_amount, uint8_t overrideCCA, uint8_t * pDstMac)
{
	uint16_t loopIdx;
	int32_t length;
	uint32_t numberOfFrames = delay_amount;
	uint8_t pConfigLen = SL_BSSID_LENGTH;
	CurrentTxMode = (uint8_t) eTxMode;
	int32_t minDelay;
	uint8_t * DataFrame = (uint8_t*)pvPortMalloc( FRAME_SIZE );

	if ((RADIO_TX_PACKETIZED == eTxMode) || (RADIO_TX_CONTINUOUS == eTxMode))
	{
		/* build the frame */
		switch (eDataPattern)
		{
			case ALL_0_PATTERN:
				memset(DataFrame, 0, sizeof(DataFrame));

				break;
	    		case ALL_1_PATTERN:
				memset(DataFrame, 1, sizeof(DataFrame));

				break;
			case INCREMENTAL_PATTERN:
				for (loopIdx = 0; loopIdx < FRAME_SIZE; loopIdx++)
					DataFrame[loopIdx] = (uint8_t)loopIdx;

				break;
	    		case DECREMENTAL_PATTERN:
				for (loopIdx = 0; loopIdx < FRAME_SIZE; loopIdx++)
					DataFrame[loopIdx] = (uint8_t)(FRAME_SIZE - 1 - loopIdx);

				break;
			default:
				memset(DataFrame, 0, sizeof(DataFrame));
		}

		sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL,&pConfigLen, &TemplateFrame[TA_OFFSET]);
		memcpy(&TemplateFrame[RA_OFFSET], pDstMac, SL_BSSID_LENGTH);
		memcpy(&TemplateFrame[DA_OFFSET], pDstMac, SL_BSSID_LENGTH);

		memcpy(DataFrame, TemplateFrame, sizeof(TemplateFrame));

		/* open a RAW/DGRAM socket based on CCA override*/
		if (overrideCCA == 1)
			rawSocket = sl_Socket(SL_AF_RF, SL_SOCK_RAW, eChannel);
		else
			rawSocket = sl_Socket(SL_AF_RF, SL_SOCK_DGRAM, eChannel);

		if (rawSocket < 0)
		{
			vPortFree( DataFrame );
			return -1;
		}
	}



	if (RADIO_TX_PACKETIZED == eTxMode)
	{
			length = size;
			portTickType xDelay;


			while ((length == size))
			{
                            /* transmit the frame */
							minDelay = (delay_amount%50);
                            length = sl_Send(rawSocket, DataFrame, size, SL_RAW_RF_TX_PARAMS(eChannel, eRate, powerLevel_Tone, ePreamble));
                            //UtilsDelay((delay_amount*CPU_CYCLES_1MSEC)/12);
                            xDelay= minDelay / portTICK_RATE_MS;
                            vTaskDelay(xDelay);

                			minDelay = (delay_amount - minDelay);
                			while(minDelay > 0)
                			{
                				vTaskDelay(50/portTICK_RATE_MS);
                				minDelay -= 50;
                			}
			}

			if (length != size)
			{
				RadioStopTX((RadioTxMode_e)CurrentTxMode);

				vPortFree( DataFrame );
				return -1;
			}

			if(sl_Close(rawSocket) < 0)
			{
				vPortFree( DataFrame );
				return -1;
			}

			vPortFree( DataFrame );
			return 0;
		}

	if (RADIO_TX_CONTINUOUS == eTxMode)
	{
		sl_SetSockOpt(rawSocket, SL_SOL_PHY_OPT, SL_SO_PHY_NUM_FRAMES_TO_TX, &numberOfFrames, sizeof(uint32_t));

		sl_Send(rawSocket, DataFrame, size, SL_RAW_RF_TX_PARAMS(eChannel, eRate, powerLevel_Tone, ePreamble));

	}

	if (RADIO_TX_CW == eTxMode)
	{
		rawSocket = sl_Socket(SL_AF_RF,SL_SOCK_RAW,eChannel);
		if(rawSocket < 0)
		{
			vPortFree( DataFrame );
			return -1;
		}

		sl_Send(rawSocket, NULL, 0, powerLevel_Tone);
	}

	vPortFree( DataFrame );
	return 0;
}

int Cmd_RadioStartTX(int argc, char*argv[])
{
	RadioTxMode_e mode;
	uint8_t pwrlvl;
	int chnl;
	SlRateIndex_e rate;
	RadioPreamble_e preamble;
	RadioDataPattern_e datapattern;
	uint16_t size;
	uint32_t delay;
	uint8_t overridecca;
	uint8_t dest_mac[6];
	int i;

	if(argc!=16) {
		UARTprintf("startx <mode, RADIO_TX_PACKETIZED=2, RADIO_TX_CW=3, RADIO_TX_CONTINUOUS=1> "
				          "<power level 0-15, as dB offset from max power so 0 high>"
						  "<channel> <rate 1=1M, 2=2M, 3=5.5M, 4=11M, 6=6M, 7=9M, 8=12M, 9=18M, 10=24M, 11=36M, 12=48M, 13=54M, 14 to 21 = MCS_0 to 7"
						  "<preamble, 0=long, 1=short, 2=odfm, 3=mixed, 4=greenfield>"
				          "<data pattern 0=all 0, 1=all 1, 2=incremental, 3=decremental, 4=PN9, 5=PN15, 6=PN23>"
				          "<size> <delay amount> <override CCA> <destination mac address, given as six 8 bit hex values>"
						);
		return -1;
	}
	mode = (RadioTxMode_e)atoi(argv[1]);
	pwrlvl = atoi(argv[2]);
	chnl = atoi(argv[3]);
	rate = (SlRateIndex_e)atoi(argv[4]);
	preamble = (RadioPreamble_e)atoi(argv[5]);
	datapattern = (RadioDataPattern_e)atoi(argv[6]);
	size = atoi(argv[7]);
	delay = atoi(argv[8]);
	overridecca = atoi(argv[9]);

	for (i = 0; i < 6; ++i) {
		dest_mac[i] = strtol(argv[10+i], NULL, 16);
	}
	return RadioStartTX(mode, pwrlvl, chnl, rate, preamble, datapattern, size, delay, overridecca, dest_mac);
}

int Cmd_RadioStopTX(int argc, char*argv[])
{
	RadioTxMode_e mode;
	if(argc!=2) {
		UARTprintf("stoptx <mode, RADIO_TX_PACKETIZED=2, RADIO_TX_CW=3, RADIO_TX_CONTINUOUS=1>\n");
	}
	mode = (RadioTxMode_e)atoi(argv[1]);
	return RadioStopTX(mode);
}


//end radio test functions
