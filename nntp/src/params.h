#ifndef __PARAMS_H__
#define __PARAMS_H__

#define PORT 		7692   		// > Servers listening port
#define ADDRNOTFOUND 	0xffffffff	// > Addres for unfound host
#define BUFFERSIZE	1024		// > Max size for packets received

//Server and clientTCP
#define BUFFER_SIZE 	10		

//Server and clientUDP
#define RETRIES	5			// > Number of times to retry before givin up
#define TIMEOUT 6			// > Max time for getting a response


#endif
