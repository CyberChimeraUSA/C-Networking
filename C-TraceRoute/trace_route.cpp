#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <endian.h>
#include <iostream>
#include <thread>
#include <unistd.h>


// ICMP types and codes
#define ICMPV4_ECHO_REQUEST_TYPE   8
#define ICMPV4_ECHO_REQUEST_CODE   0
#define ICMPV4_ECHO           8
#define DEFAULT_DATA_SIZE      32       // default data size
#define DEFAULT_TTL            30       // default timeout

using namespace std;

// ICMP header
typedef struct icmp_hdr
{
	unsigned char   icmp_type;
	unsigned char   icmp_code;
 	unsigned short  icmp_checksum;
    unsigned short  _icmp_id;
	unsigned short  icmp_sequence;
	unsigned int   icmp_timestamp;
} ICMP_HDR;


//Trace Route
/****Variables****/
char gDestination[256] = { 0 };      // Destination
char var[1] = { };
int gAddressFamily = AF_UNSPEC;     // Address family to use
int gProtocol = IPPROTO_ICMP;       // Protocol value
int s;
int packetlen;
struct addrinfo *dest;
struct addrinfo *local;


/****Functions****/
void InitIcmpHeader(char *buf, int datasize);
void SetIcmpSequence(char *buf);
void ComputeIcmpChecksum(int s, char *buf, int packetlen, struct addrinfo *dest);
void process_packet();
unsigned short checksum(unsigned short *buffer, int size);
int PrintAddress(struct addrinfo *sa);
int set_ICMP_protocol(struct addrinfo *sa);
int SetTtl(int s, int ttl);
unsigned int route_endpoint(void);
unsigned int route_protocol(void);
struct addrinfo *resolve_address(char *addr, char *port, int af, int type, int proto);

// Description:
//    Main Function for Trace Route Program
int main (void)
{
	*var = 0;
//    /int s;
    int rc,ttl,notdone;
    int gTtl = DEFAULT_TTL;              // Default TTL value

	packetlen = 0;

	char *icmpbuf = NULL;

	cout << "Trace Route " << endl;

	/*************************************/
	/** Resolve the destination address **/
	/*************************************/
	route_endpoint();
	route_protocol();

	dest = resolve_address(gDestination,var,gAddressFamily,0,0);
	  if (dest == NULL)
	  {
		 printf("Bad name %s\n", gDestination);
		 return 1;
	  }
	  else
		printf("ResolveAddress() is OK!\n");
	/***************************/
	/**Print Debug Information**/
	/***************************/

	PrintAddress(dest);

	/**********************/
	/**Set ICMP Protocols**/
	/**********************/
	set_ICMP_protocol(dest);

    	/*************************/
	/** Get the bind address**/
	/*************************/
	local = resolve_address(NULL, var, gAddressFamily, 0, 0);
	  if (local == NULL)
	  {
		 printf("Unable to obtain the bind address!\n");
		 return 1;
	  }
	  else
		printf("ResolveAddress() is fine!\n");
	  PrintAddress(local);

    /***************************/
	/** Create the raw socket **/
		//AF_INET = 2*/
	  //IPPROTO_ICMP = 1*/
	/***************************/
    s = socket(gAddressFamily, SOCK_RAW, gProtocol);

	if (s == -1)
	{
		printf("socket() failed with\n");
		return -1;
	}
	else
		printf("socket() is OK!, fd is %d \n",s);

/********************************************************/
/** Figure out the size of the ICMP header and payload **/
/********************************************************/
	if (gAddressFamily == AF_INET)
		packetlen += sizeof(ICMP_HDR);

	/** Add in the data size **/
		packetlen += DEFAULT_DATA_SIZE;

/************************************************************/
/** Allocate the buffer that will contain the ICMP request **/
/************************************************************/
	icmpbuf = (char *)malloc(packetlen);
	if (icmpbuf == NULL)
	{
		fprintf(stderr, "Malloc for ICMP buffer failed with error code\n");
		return -1;
	}
	else
		printf("Malloc() for ICMP buffer is OK!\n");

/*********************************/
/** Initialize the ICMP headers **/
/*********************************/
	if (gAddressFamily == AF_INET)
	{
		InitIcmpHeader(icmpbuf, DEFAULT_DATA_SIZE);
	}

/**********************************************************************/
/** Bind the socket -- need to do this since we post a receive first **/
/**********************************************************************/
	rc = bind(s, local->ai_addr, local->ai_addrlen);
	if (rc == -1)
	{
		fprintf(stderr, "bind() failed with error code \n");
		return 1;
	}
	else
		printf("bind() is OK!: Local Address Bound\n");
		//PrintAddress(local->ai_addr, local->ai_addrlen);
	    ttl = 1;  //Increase and watch on Wireshark

		/**********************************/
		/* Start sending the ICMP requests*/
		/**********************************/
		do
		{
			notdone = 1;
			SetTtl(s, ttl);

			// Set the sequence number and compute the checksum
			SetIcmpSequence(icmpbuf);
			ComputeIcmpChecksum(s, icmpbuf, packetlen, dest);

			// Send the ICMP echo request
			std::thread first (&process_packet);     // spawn new thread that calls foo()
			rc = sendto(s, icmpbuf, packetlen, 0, dest->ai_addr, dest->ai_addrlen);
			//send(s,icmpbuf,packetlen,0);
			if (rc == -1)
			{
				fprintf(stderr, "sendto() failed with error code \n");
				return -1;
			}
			else
				printf("sendto() is OK\n");
			first.join();


			ttl++;
			sleep(1);

		} while ((notdone) && (ttl < gTtl));

		free(icmpbuf);



    return 0;
}














struct addrinfo *resolve_address(char *addr, char *port, int af, int type, int proto)
{

	struct addrinfo hints, *res = NULL;

	int  rc;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;

	rc = getaddrinfo(addr, port, &hints, &res);
	if (rc != 0)
	{
		printf("Invalid address %s, getaddrinfo() failed with error code %d\n", addr, rc);
		return NULL;
	}
	else
		printf("ResolveAddress(): getnameinfo() is OK!\n");
	return res;
}

// Function: route_protocol
// Description:
//    Takes user input for IPv4
unsigned int route_protocol(void)
{
	unsigned int input_family;
	cout << "Enter Protocol IPv4 (4) or IPv6 (Not Implemented yet) (6): " << endl;
	cin >> input_family;
	cout << "Protocol IPv"  << input_family << endl;

	if (input_family == 4 )
	{
		if (input_family == 4)
		{
			gAddressFamily = AF_INET;
		}

	}
	else
	{
		cout << "Not IPv4 " << endl;
		return 1;
	}
	return 0;
}

 //Function: route_endpoint
 // Description:
 //   Takes user input and returns endpoint location
 //   ex: www.google.com
 //
unsigned int route_endpoint()
{

	cout << "Enter Trace Destination: " << endl;
	//TODO-Add Error Handler for name
	cin >> gDestination;
	cout << "Trace Destination: "  << gDestination << endl;
	return 0;
}



int SetTtl(int s, int ttl)
{
	int     optlevel, option, rc;
	rc = 0;


	if (gAddressFamily == AF_INET)
	{
		optlevel = IPPROTO_IP;
		option = IP_TTL;
	}
	else
	{
		rc = 0;
	}


	if (rc == 0)
	{
		rc = setsockopt(s, optlevel, option, (char *)&ttl, sizeof(ttl));
	}
	else if (rc == -1)
	{
		fprintf(stderr, "SetTtl(): setsockopt() failed with error code\n");
	}
	else
		printf("SetTtl(): setsockopt() should be fine!\n");

	return rc;
}


// Function: set_ICMP_protocol
// Description:
//    Decides on IPPROTO_ICMP or IPPROTO_ICMP6
int set_ICMP_protocol(struct addrinfo *sa)
{
	/**********************/
	/**Set ICMP Protocols**/
	/**********************/
	gAddressFamily = sa->ai_family;
	if (gAddressFamily == AF_INET)
		gProtocol = IPPROTO_ICMP;
	return gProtocol;
}


// Function: PrintAddress
// Description:
//    This routine prints Debug Information
int PrintAddress(struct addrinfo *sa)
{
	char    host[NI_MAXHOST], serv[NI_MAXSERV];
	int     hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc;



	rc = getnameinfo(sa->ai_addr, sa->ai_addrlen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc != 0)
	{
		fprintf(stderr, "%s: getnameinfo() failed with error code %d\n", __FILE__, rc);
		return rc;
	}
	else
		printf("PrintAddress(): getnameinfo() is OK!\n");

	// If the port is zero then don't print it
	if (strcmp(serv, "0") != 0)
	{
		if (sa->ai_addr->sa_family == AF_INET)
			printf("[%s]:%s", host, serv);
		else
			printf("%s:%s", host, serv);
	}
	else
		printf("%s", host);

	return 0;

}


unsigned short checksum(unsigned short *buffer, int size)
{
	unsigned long cksum = 0;
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}
	if (size)
	{
		cksum += *(unsigned char*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (unsigned short)(~cksum);
}

// Function: InitIcmpHeader
// Description:
//    Initialize the ICMP header as well as the echo request header.
void InitIcmpHeader(char *buf, int datasize)

{
	ICMP_HDR   *icmp_hdr = NULL;
	char       *datapart = NULL;
	icmp_hdr = (ICMP_HDR *)buf;
	icmp_hdr->icmp_type = ICMPV4_ECHO_REQUEST_TYPE;        // request an ICMP echo
	icmp_hdr->icmp_code = ICMPV4_ECHO_REQUEST_CODE;
	icmp_hdr->_icmp_id = (unsigned short)getpid();
	icmp_hdr->icmp_checksum = 0;
	icmp_hdr->icmp_sequence = 0;
	icmp_hdr->icmp_timestamp =  (unsigned int)time(NULL);
	datapart = buf + sizeof(ICMP_HDR);
	// Place some junk in the buffer.
	memset(datapart, 'E', datasize);
}

// Function: SetIcmpSequence
// Description:
//    This routine sets the sequence number of the ICMP request packet.
void SetIcmpSequence(char *buf)
{
	unsigned long    sequence = 0;
	sequence = (unsigned long)time(NULL);

	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR    *icmpv4 = NULL;
		icmpv4 = (ICMP_HDR *)buf;
		icmpv4->icmp_sequence = (unsigned short)sequence;
	}
}

void ComputeIcmpChecksum(int s, char *buf, int packetlen, struct addrinfo *dest)
{
	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR    *icmpv4 = NULL;
		icmpv4 = (ICMP_HDR *)buf;
		icmpv4->icmp_checksum = 0;
		icmpv4->icmp_checksum = checksum((unsigned short *)buf, packetlen);
	}
}

void process_packet()
{
	unsigned char recvbuf[65000] = {0};
	unsigned char bit_parsing_buffer[1] = {0};
	unsigned char header_version[1] = {0};
	unsigned char header_length[1] = {0};
	unsigned char diff_serv_codepoint[1] = {0};
	unsigned char diff_serv_explicit_congection[1] = {0};
  unsigned short total_length[1] = {0};
  unsigned short identification [1] = {0};
	unsigned short flags [1] = {0};
	unsigned char time_to_live[1] = {0};
	unsigned char protocol_icmp[1]={0};
	unsigned short header_checksum[1] = {0};
	unsigned int   source_ip[1] = {0};
	unsigned int   destination_ip[1] = {0};
	unsigned char  ipv4_type[1] = {0};
	unsigned char  ipv4_code[1] = {0};
	unsigned short ipv4_checksum[1] = {0};
	unsigned char  ipv4_version[1] = {0};
	unsigned char  ipv4_length[1] = {0};
	unsigned char  ipv4_diff_serv_field[1] = {0}; //Not breaking down to bit level
	unsigned short ipv4_total_length[1] = {0};
	unsigned short ipv4_identification [1] = {0};
	unsigned short ipv4_flags [1] = {0};
	unsigned char  ipv4_time_to_live[1] = {0};
	unsigned char  ipv4_protocol_icmp[1] = {0};
	unsigned short ipv4_header_checksum[1] = {0};
	unsigned int   ipv4_source_ip[1] = {0};
	unsigned int   ipv4_destination_ip[1] = {0};
	unsigned char  icmp_type[1] = {0};
	unsigned char  icmp_code[1] = {0};
	unsigned short icmp_checksum[1] = {0};
	unsigned short icmp_identifier[1] = {0};
	unsigned short icmp_sequence_number[1] = {0};



	int status;
	
	printf("Debug- PacketLength %d \n",packetlen);
	//status = recv(s, recvbuf, packetlen,0);
	status = recv(s, recvbuf, 56,0);
	printf("recv from value %d \n",status);
	for (int x = 0; x < packetlen; x++)
	{
		printf("%x",recvbuf[x]);
	}
		printf("\n");
		//Parsing- Version
        *header_version = *recvbuf >> 4;  //Extract Header Version from recvbuf
		printf("  Version: %x\n",*header_version); 
	  
		//Parsing- Header Length
	    *bit_parsing_buffer = *recvbuf; //Assign recvbuf to bit_parsing_buf, this contain 1 byte
		*header_length = *bit_parsing_buffer << 4; //Truncate half of the byte so only 4 bit length remains
		*header_length = *header_length >> 4;  //Shift buffer back the orignal offset
		 printf("  Header Length: %x\n",*header_length);
		
		//Parsing- Differentianted Service Field 
		printf("  Differentianted Service Field : 0x%x\n",*(recvbuf + 1));
        *bit_parsing_buffer = *(recvbuf + 1);
        *diff_serv_codepoint = *bit_parsing_buffer >> 2;
		printf("     Differentianted Service Codepoint : 0x%x\n",*diff_serv_codepoint);
         *diff_serv_explicit_congection = *bit_parsing_buffer << 6;
         *diff_serv_explicit_congection = *diff_serv_explicit_congection >> 6;
		printf("     Differentianted Service Explicit Congection : 0x%x\n",*diff_serv_explicit_congection);
		
		//Parsing- Total Length
		//*total_length = *(recvbuf + 4);
    	memcpy ( total_length,recvbuf+2 ,2);
		printf("  Total Length: 0x%x (%d)\n",be16toh(*total_length),be16toh(*total_length));

		//Parsing- Identification 
		memcpy ( identification,recvbuf+4 ,2);
        printf("  Identification: 0x%x (%d)\n",be16toh(*identification),be16toh(*identification));

		//Parsing- Flags 
		memcpy ( flags,recvbuf+6 ,2);
     	printf("  Flags: 0x%x (%d)\n",be16toh(*flags),be16toh(*flags));

			//Parsing- TTL
		*time_to_live = *(recvbuf+8);
		//memcpy ( time_to_live,recvbuf+8 ,1);
    	printf("  TTL: 0x%x (%d)\n",*time_to_live,*time_to_live);

		//Parsing- Protocol
		*protocol_icmp = *(recvbuf+9);
     	printf("  Protocol: 0x%x (%d)\n",*protocol_icmp,*protocol_icmp);

     //Parsing- Header Checksum
    	memcpy ( header_checksum,recvbuf+10 ,2);
		printf("  Header Checksum: 0x%x (%d)\n",be16toh(*header_checksum),be16toh(*header_checksum));

    //Parsing- Source IP Address
	    struct sockaddr_in source_ip_structure;
		char *source_addr;
    	memcpy(source_ip,recvbuf+12 ,4);
		source_ip_structure.sin_addr.s_addr = *source_ip;
		source_addr = inet_ntoa(source_ip_structure.sin_addr); 
		printf("  Source IP: %s (0x%x)\n",source_addr,be32toh(*source_ip));

    //Parsing- Destination IP Address
		struct sockaddr_in dest_ip_structure;
		char *dest_addr;
    	memcpy (destination_ip,recvbuf+16 ,4);
		dest_ip_structure.sin_addr.s_addr = *destination_ip;
		dest_addr = inet_ntoa(dest_ip_structure.sin_addr); 
		printf("  Destination IP: %s (0x%x)\n",dest_addr,be32toh(*destination_ip));

		//Parsing- IPV4 Type
		*ipv4_type = *(recvbuf+20);
		printf("    IPV4 Type: 0x%x (%d)\n",*ipv4_type,*ipv4_type);

		//Parsing- IPV4 Code
		*ipv4_code = *(recvbuf+21);
		printf("    IPV4 Code: 0x%x (%d)\n",*ipv4_code,*ipv4_code);

	    //Parsing- IPV4 Checksum
        memcpy ( ipv4_checksum,recvbuf+22 ,2);
		printf("    IPV4 Checksum: 0x%x (%d)\n",be16toh(*ipv4_checksum),be16toh(*ipv4_checksum));
	
	    //Parsing- IPV4 Version
        *ipv4_version = *(recvbuf + 28) >> 4;  //Extract Header Version from recvbuf
		printf("    Version: %x\n",*ipv4_version); 

		//Parsing- IPV4 Length
	     *bit_parsing_buffer = *(recvbuf + 28); //Assign recvbuf to bit_parsing_buf, this contain 1 byte
		*ipv4_length = *bit_parsing_buffer << 4; //Truncate half of the byte so only 4 bit length remains
		*ipv4_length = *ipv4_length >> 4;  //Shift buffer back the orignal offset
		 printf("    Length: %x\n",*ipv4_length);

			//Parsing- IPV4 Differential Dervices
		*ipv4_diff_serv_field = *(recvbuf+29);
		printf("    Differentianted Service Field: 0x%x (%d)\n",*ipv4_diff_serv_field,*ipv4_diff_serv_field);

		//Parsing- IPV4 Total Length
		//*total_length = *(recvbuf + 4);
    	memcpy ( ipv4_total_length,recvbuf+30 ,2);
		printf("    IPV4 Total Length: 0x%x (%d)\n",be16toh(*ipv4_total_length),be16toh(*ipv4_total_length));

		//Parsing- IPV4 Identification
		memcpy ( ipv4_identification,recvbuf+32 ,2);
	     printf("    IPV4 Identification: 0x%x (%d)\n",be16toh(*ipv4_identification),be16toh(*ipv4_identification));

		//Parsing- IPV4 Flags
		memcpy ( ipv4_flags,recvbuf+34 ,2);
	      printf("    IPV4 Flags: 0x%x (%d)\n",be16toh(*ipv4_flags),be16toh(*ipv4_flags));

		//Parsing- IPV4 TTL
		*ipv4_time_to_live = *(recvbuf+36);
		//memcpy ( time_to_live,recvbuf+8 ,1);
     	printf("    IPV4-TTL: 0x%x (%d)\n",*ipv4_time_to_live,*ipv4_time_to_live);

		//Parsing- IPV4 Protocol
		*ipv4_protocol_icmp = *(recvbuf+37);
    	printf("    IPV4-Protocol: 0x%x (%d)\n",*ipv4_protocol_icmp,*ipv4_protocol_icmp);

	    //Parsing- IPV4 Checksum
    	memcpy ( ipv4_header_checksum,recvbuf+38 ,2);
		printf("    IPV4-Checksum: 0x%x (%d)\n",be16toh(*ipv4_header_checksum),be16toh(*ipv4_header_checksum));
	
    //Parsing- IPV4 Source IP Address
		struct sockaddr_in icmp_source_ip_structure;
		char *icmp_source_addr;
    	memcpy(ipv4_source_ip,recvbuf+40 ,4);
		icmp_source_ip_structure.sin_addr.s_addr = *ipv4_source_ip;
		icmp_source_addr = inet_ntoa(icmp_source_ip_structure.sin_addr); 
		printf("    ICMP-Source IP: %s (0x%x)\n",icmp_source_addr,be32toh(*ipv4_source_ip));

    //Parsing- IPV4 Destination IP Address
		struct sockaddr_in icmp_dest_ip_structure;
		char *icmp_dest_addr;
    	memcpy (ipv4_destination_ip,recvbuf+44 ,4);
		icmp_dest_ip_structure.sin_addr.s_addr = *ipv4_destination_ip;
		icmp_dest_addr = inet_ntoa(icmp_dest_ip_structure.sin_addr); 
		printf("    ICMP- Destination IP: %s (0x%x)\n",icmp_dest_addr,be32toh(*ipv4_destination_ip));


	//Parsing- ICMP Type
		*icmp_type = *(recvbuf+48);
    	printf("      ICMP-Type: 0x%x (%d)\n",*icmp_type,*icmp_type);

	//Parsing- ICMP Code
		*icmp_code = *(recvbuf+49);
    	printf("      ICMP-Code: 0x%x (%d)\n",*icmp_code,*icmp_code);

   //Parsing- ICMP Checksum
  	    memcpy ( icmp_checksum,recvbuf+50 ,2);
		printf("      ICMP-Checksum: 0x%x (%d)\n",be16toh(*icmp_checksum),be16toh(*icmp_checksum));

    //Parsing- ICMP Identifier
    	memcpy ( icmp_identifier,recvbuf+52 ,2);
		printf("      ICMP-Identifier: 0x%x (%d)\n",be16toh(*icmp_identifier),be16toh(*icmp_identifier));

		  //Parsing- ICMP Sequence Number
    	memcpy ( icmp_sequence_number,recvbuf+54 ,2);
		printf("      ICMP-Sequence: 0x%x (%d)\n",be16toh(*icmp_sequence_number),be16toh(*icmp_sequence_number));

}