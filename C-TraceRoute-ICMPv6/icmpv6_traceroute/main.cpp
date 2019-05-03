
#include "ICMP.hpp"
#include <endian.h>
using namespace std;



int main (void)
{
    Protocol_ICMP trace_route_instance;
		trace_route_instance.trace_route_base();

}
// Function: trace_route_base
// Description:
//    Main Function for Trace Route Program
int Protocol_ICMP::trace_route_base()
{
	std::thread first (&Protocol_ICMP::process_packet,this);     // spawn new thread that calls foo()

	*var = 0;
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

	  PrintAddress(local);
  /***************************/
	/** Create the raw socket **/
	/*AF_INET = 2*/
	/*AF_INET6 = 23/
	 //
	 //IPPROTO_ICMP = 1*/
	/*IPPROTO_ICMP6 = 58*/
	/***************************/
		s = socket(gAddressFamily, SOCK_RAW, gProtocol);

		if (s == -1)
		{
			printf("socket() failed with\n");
			return -1;
		}

		/********************************************************/
		/** Figure out the size of the ICMP header and payload **/
		/********************************************************/
		if (gAddressFamily == AF_INET)
			packetlen += sizeof(ICMP_HDR);
		else if (gAddressFamily == AF_INET6)
			packetlen += sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

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

		/*********************************/
		/** Initialize the ICMP headers **/
		/*********************************/
		if (gAddressFamily == AF_INET)
		{
			InitIcmpHeader(icmpbuf, DEFAULT_DATA_SIZE);
		}
		else if (gAddressFamily == AF_INET6)
		{
			InitIcmp6Header(icmpbuf, DEFAULT_DATA_SIZE);
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

			rc = sendto(s, icmpbuf, packetlen, 0, dest->ai_addr, dest->ai_addrlen);

			if (rc == -1)
			{
				fprintf(stderr, "sendto() failed with error code \n");
				return -1;
			}
		//	else
			//	printf("sendto() is OK\n");
			
			ttl++;
			sleep(1);

		} while ((notdone) && (ttl < gTtl));
		first.join();
		free(icmpbuf);
	return 0;
}
/*****************************
*
*PCAP Functions
*
*
******************************/



void Protocol_ICMP::process_packet()
{
   device = pcap_lookupdev(errbuf);
   if(device == NULL)
      pcap_fatal("pcap_lookupdev", errbuf);
   //printf("Sniffing on device %s\n", device);

   pcap_handle = pcap_open_live(device, 4096, 1, 0, errbuf);
   if(pcap_handle == NULL)
     pcap_fatal("pcap_open_live", errbuf);

	   while(1)
   {
      packet = pcap_next(pcap_handle, &header);
      std::thread packet_analyze_thread(&Protocol_ICMP::packet_analyze,this,packet,header.len);
      packet_analyze_thread.join();
   }
   pcap_close(pcap_handle);

}

void Protocol_ICMP::packet_analyze(const unsigned char *data_buffer, const unsigned int length)
{
   if(*(data_buffer+0x14) == 0x3a && *(data_buffer+0x36) == 0x3 )
   {
      std::thread packet_dump(&Protocol_ICMP::dump_formatted,this,data_buffer,length);
     packet_dump.join();
   }
   return;
}
void Protocol_ICMP::dump_formatted(const unsigned char *data_buffer, const unsigned int length)
{
   int i;
   unsigned char buffer[6] = {0};
   unsigned short packet_data[1] = {0};
   unsigned char packet_data_byte[1] = {0};
   unsigned char  icmpv6_ipaddress[16] = {0} ;

   printf("Destination MAC Address: ");
   memcpy(buffer,data_buffer,6);
   for(i = 0; i < sizeof(buffer); i++)
   {
      printf("%x",buffer[i]);
   }
   printf("\n");
   printf("Source MAC Address: ");
   memcpy(buffer,data_buffer+0x6,6);
   for(i = 0; i < sizeof(buffer); i++)
   {
      printf("%x",buffer[i]);
   }
   printf("\n");

   memcpy(packet_data,data_buffer+0xC,2);
   printf("Type: 0x%x \n",be16toh(*packet_data));

   printf("Version: 0x%x \n",*(data_buffer + 0xE) >> 4);

   memcpy(packet_data,data_buffer+0x12,2);
   printf("Payload Length: 0x%x \n",be16toh(*(packet_data)));

   printf("Next Header: 0x%x \n",*(data_buffer + 0x14));

   printf("Hop Limit: 0x%x \n",*(data_buffer + 0x15));

	//Parsing - IP address
   memcpy ( icmpv6_ipaddress,data_buffer+0x16 ,16);
		 printf("Source IP: "); 
		unsigned char  local_ipaddress[1] = {0} ;
		 for (int i = 0; i < 16; i++)
		 {
			 	
				*local_ipaddress = *(icmpv6_ipaddress + i);
				if (*local_ipaddress < 10)
				{
					printf("0%x",*(local_ipaddress )); 

				}
				else
				{
			 	printf("%x",*(local_ipaddress )); 
				}
				if(i != 0  && i != 15 && i%2 != 0)
				{
			 		printf(":"); 
				}

		 }
		printf("\n"); 


   //Parsing - IP address
   memcpy ( icmpv6_ipaddress,data_buffer+ 0x26 ,16);
		 printf("Destination IP: "); 
		 for (int i = 0; i < 16; i++)
		 {
			 	
				*local_ipaddress = *(icmpv6_ipaddress + i);
				if (*local_ipaddress < 10)
				{
					printf("0%x",*(local_ipaddress )); 

				}
				else
				{
			 	printf("%x",*(local_ipaddress )); 
				}
				if(i != 0  && i != 15 && i%2 != 0)
				{
			 		printf(":"); 
				}

		 }
		printf("\n");

   printf("   (ICMPv6)Type: 0x%x \n",*(data_buffer + 0x36));

   //Parsing - IP address
   memcpy ( icmpv6_ipaddress,data_buffer+ 0x46 ,16);
		 printf("   (ICMPv6)Source IP: "); 
		 for (int i = 0; i < 16; i++)
		 {
			 	
				*local_ipaddress = *(icmpv6_ipaddress + i);
				if (*local_ipaddress < 10)
				{
					printf("0%x",*(local_ipaddress )); 

				}
				else
				{
			 	printf("%x",*(local_ipaddress )); 
				}
				if(i != 0  && i != 15 && i%2 != 0)
				{
			 		printf(":"); 
				}

		 }
		printf("\n");



   //Parsing - IP address
   memcpy ( icmpv6_ipaddress,data_buffer+ 0x56 ,16);
		 printf("   (ICMPv6)Destination IP: "); 
		 for (int i = 0; i < 16; i++)
		 {
			 	
				*local_ipaddress = *(icmpv6_ipaddress + i);
				if (*local_ipaddress < 10)
				{
					printf("0%x",*(local_ipaddress )); 

				}
				else
				{
			 	printf("%x",*(local_ipaddress )); 
				}
				if(i != 0  && i != 15 && i%2 != 0)
				{
			 		printf(":"); 
				}

		 }
		printf("\n");

   printf("   (ICMPv6)Type: 0x%x \n",*(data_buffer + 0x66));

}

void Protocol_ICMP::dump(const unsigned char *data_buffer, const unsigned int length) 
{
   unsigned char byte;
   unsigned int i, j;
   for(i=0; i < length; i++) {
      byte = data_buffer[i];
      printf("%02x ", data_buffer[i]);  // Display byte in hex.
      if(((i%16)==15) || (i==length-1)) {
         for(j=0; j < 15-(i%16); j++)
            printf("   ");
         printf("| ");
         for(j=(i-(i%16)); j <= i; j++) {  // Display printable bytes from line.
            byte = data_buffer[j];
            if((byte > 31) && (byte < 127)) // Outside printable char range
               printf("%c", byte);
            else
               printf(".");
         }
         printf("\n"); // End of the dump line (each line is 16 bytes)
      } // End if
   } // End for
}

void Protocol_ICMP::pcap_fatal(const char *failed_in, const char *errbuf) {
   printf("Fatal Error in %s: %s\n", failed_in, errbuf);
   return;
}


// Function: route_protocol
// Description:
//    Takes user input for IPv6
unsigned int Protocol_ICMP::route_protocol(void)
{

			gAddressFamily = AF_INET6;

	return 0;
}

 //Function: route_endpoint
 // Description:
 //   Takes user input and returns endpoint location
 //   ex: www.google.com
 //
unsigned int Protocol_ICMP::route_endpoint()
{

	cout << "Enter Trace Destination: " << endl;
	//TODO-Add Error Handler for name
	cin >> gDestination;
	cout << "Trace Destination: "  << gDestination << endl << endl;
	return 0;
}

// Function: set_ICMP_protocol
// Description:
//    Decides on IPPROTO_ICMP or IPPROTO_ICMP6
int Protocol_ICMP::set_ICMP_protocol(struct addrinfo *sa)
{
	/**********************/
	/**Set ICMP Protocols**/
	/**********************/
	gAddressFamily = sa->ai_family;
	if (gAddressFamily == AF_INET)
		gProtocol = IPPROTO_ICMP;
	else if (gAddressFamily == AF_INET6)
		gProtocol = IPPROTO_ICMP6;
	return gProtocol;
}

// Function: PrintAddress
// Description:
//    This routine prints Debug Information
int Protocol_ICMP::PrintAddress(struct addrinfo *sa)
{
	char    host[NI_MAXHOST], serv[NI_MAXSERV];
	int     hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc;

	rc = getnameinfo(sa->ai_addr, sa->ai_addrlen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc != 0)
	{
		fprintf(stderr, "%s: getnameinfo() failed with error code %d\n", __FILE__, rc);
		return rc;
	}
	//else
	//	printf("PrintAddress(): getnameinfo() is OK!\n");

	// If the port is zero then don't print it
	if (strcmp(serv, "0") != 0)
	{
		if (sa->ai_addr->sa_family == AF_INET)
			printf("[%s]:%s", host, serv);
		else
			printf("%s:%s", host, serv);
	}
	//else
	//	printf("%s", host);

	return 0;

}

// Function: resolve_address
// Description:
//    This routine resolves the specified address and returns a list of addrinfo
//    structure containing SOCKADDR structures representing the resolved addresses.
//    Note that if 'addr' is non-NULL, then getaddrinfo will resolve it whether
//    it is a string literal address or a hostname.
struct addrinfo *Protocol_ICMP::resolve_address(char *addr, char *port, int af, int type, int proto)
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
	//else
	//	printf("ResolveAddress(): getnameinfo() is OK!\n");
	return res;
}

// Function: InitIcmpHeader
// Description:
//    Initialize the ICMP header as well as the echo request header.
void Protocol_ICMP::InitIcmpHeader(char *buf, int datasize)

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

// Function: InitIcmp6Header
// Description:
//    Initialize the ICMP6 header as well as the echo request header.
int Protocol_ICMP::InitIcmp6Header(char *buf, int datasize)
{
	ICMPV6_HDR          *icmp6_hdr = NULL;
	ICMPV6_ECHO_REQUEST *icmp6_req = NULL;
	char                *datapart = NULL;
	// Initialize the ICMP6 header fields

	icmp6_hdr = (ICMPV6_HDR *)buf;
	icmp6_hdr->icmp6_type = ICMPV6_ECHO_REQUEST_TYPE;
	icmp6_hdr->icmp6_code = ICMPV6_ECHO_REQUEST_CODE;
	icmp6_hdr->icmp6_checksum = 0;

	// Initialize the echo request fields
	icmp6_req = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));
	icmp6_req->icmp6_echo_id = (unsigned short)getpid();
	icmp6_req->icmp6_echo_sequence = 0;
	datapart = (char *)buf + sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

	memset(datapart, '$', datasize);
	return (sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST));
}

// Function: SetTtl
// Description:
//    Sets the TTL on the socket.
int Protocol_ICMP::SetTtl(int s, int ttl)
{
	int     optlevel, option, rc;
	rc = 0;


	if (gAddressFamily == AF_INET)
	{
		optlevel = IPPROTO_IP;
		option = IP_TTL;
	}
	else if (gAddressFamily == AF_INET6)
	{
		optlevel = IPPROTO_IPV6;
		option = IPV6_UNICAST_HOPS;
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
		//printf("SetTtl(): setsockopt() should be fine!\n");

	return rc;
}

// Function: SetIcmpSequence
// Description:
//    This routine sets the sequence number of the ICMP request packet.
void Protocol_ICMP::SetIcmpSequence(char *buf)
{
	unsigned long    sequence = 0;
	sequence = (unsigned long)time(NULL);

	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR    *icmpv4 = NULL;
		icmpv4 = (ICMP_HDR *)buf;
		icmpv4->icmp_sequence = (unsigned short)sequence;
	}
	else if (gAddressFamily == AF_INET6)
	{
		ICMPV6_HDR          *icmpv6 = NULL;
		ICMPV6_ECHO_REQUEST *req6 = NULL;
		icmpv6 = (ICMPV6_HDR *)buf;
		req6 = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));
		req6->icmp6_echo_sequence = (unsigned short)sequence;
	}
}

// Function: checksum
// Description:
//    This function calculates the 16-bit one's complement sum
//    of the supplied buffer (ICMP) header.

unsigned short Protocol_ICMP::checksum(unsigned short *buffer, int size)
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


// Function: ComputeIcmp6PseudoHeaderChecksum
// Description:
//    This routine computes the ICMP6 checksum which includes the pseudo
//    header of the IPv6 header (see RFC2460 and RFC2463). The one difficulty
//    here is we have to know the source and destination IPv6 addresses which
//    will be contained in the IPv6 header in order to compute the checksum.
//    To do this we call the SIO_ROUTING_INTERFACE_QUERY ioctl to find which
//    local interface for the outgoing packet.
#if 1
unsigned short Protocol_ICMP::ComputeIcmp6PseudoHeaderChecksum(int s, char *icmppacket, int icmplen, struct addrinfo *dest)
{
	sockaddr_storage  localif;
	//DWORD            bytes;
	char             tmp[65535], *ptr = NULL, proto = 0;
	int              total, length, i;

	// We use a temporary buffer to calculate the pseudo header.
	ptr = tmp;
	total = 0;

	// Copy source address
    uint8_t  ip_local[16] = {0};      /* IPv6 address */
		inet_pton(AF_INET6,(char *)dest,ip_local);

	//printf("sizeof ip local %d \n",sizeof(ip_local));
	memcpy(ptr,ip_local,16);
	//memcpy(ptr, &((sockaddr_in6  *)&localif)->sin6_addr, sizeof(struct in6_addr));
	ptr += sizeof(struct in6_addr);
	total += sizeof(struct in6_addr);

	// Copy destination address
	memcpy(ptr, &((sockaddr_in6  *)dest->ai_addr)->sin6_addr, sizeof(struct in6_addr));
	ptr += sizeof(struct in6_addr);
	total += sizeof(struct in6_addr);
	// Copy ICMP packet length

	length = htonl(icmplen);
	memcpy(ptr, &length, sizeof(length));
	ptr += sizeof(length);
	total += sizeof(length);

	// Zero the 3 bytes
	memset(ptr, 0, 3);
	ptr += 3;
	total += 3;

	// Copy next hop header
	proto = IPPROTO_ICMP6;
	memcpy(ptr, &proto, sizeof(proto));
	ptr += sizeof(proto);
	total += sizeof(proto);

	// Copy the ICMP header and payload
	memcpy(ptr, icmppacket, icmplen);
	ptr += icmplen;
	total += icmplen;
	for (i = 0; i < icmplen % 2; i++)
	{
		*ptr = 0;
		ptr++;
		total++;
	}
	return checksum((unsigned short *)tmp, total);
}
#endif

// Function: ComputeIcmpChecksum
// Description:
//    This routine computes the checksum for the ICMP request. For IPv4 its
//    easy, just compute the checksum for the ICMP packet and data. For IPv6,
//    its more complicated. The pseudo checksum has to be computed for IPv6
//    which includes the ICMP6 packet and data plus portions of the IPv6
//    header which is difficult since we aren't building our own IPv6 header.

void Protocol_ICMP::ComputeIcmpChecksum(int s, char *buf, int packetlen, struct addrinfo *dest)
{
	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR    *icmpv4 = NULL;
		icmpv4 = (ICMP_HDR *)buf;
		icmpv4->icmp_checksum = 0;
		icmpv4->icmp_checksum = checksum((unsigned short *)buf, packetlen);
	}
	else if (gAddressFamily == AF_INET6)
	{
		ICMPV6_HDR  *icmpv6 = NULL;
		icmpv6 = (ICMPV6_HDR *)buf;
		icmpv6->icmp6_checksum = 0;
		icmpv6->icmp6_checksum = ComputeIcmp6PseudoHeaderChecksum(s, buf, packetlen, dest);
	}
}

