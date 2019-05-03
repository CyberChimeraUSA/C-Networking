
#ifndef INCLUDE_ICMP_HPP_
#define INCLUDE_ICMP_HPP_

#include "CommonHeader.hpp"


class Protocol_ICMP
{
public:
	  int trace_route_base();

private:

	  // IPv4 header
	  typedef struct ip_hdr

	  {
	  	unsigned char  ip_verlen;        // 4-bit IPv4 version, 4-bit header length (in 32-bit words)
	  	unsigned char  ip_tos;           // IP type of service
	  	unsigned short ip_totallength;   // Total length
	  	unsigned short ip_id;            // Unique identifier
	  	unsigned short ip_offset;        // Fragment offset field
	  	unsigned char  ip_ttl;           // Time to live
	  	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	  	unsigned short ip_checksum;      // IP checksum
	  	unsigned int   ip_srcaddr;       // Source address
	  	unsigned int   ip_destaddr;      // Source address
	  } IPV4_HDR;//, *PIPV4_HDR, FAR * LPIPV4_HDR;


	  // IPv4 option header
	  typedef struct ipv4_option_hdr
	  {
	  	unsigned char   opt_code;           // option type
	  	unsigned char   opt_len;            // length of the option header
	  	unsigned char   opt_ptr;            // offset into options
	  	unsigned long   opt_addr[9];        // list of IPv4 addresses
	  } IPV4_OPTION_HDR;//, *PIPV4_OPTION_HDR, FAR *LPIPV4_OPTION_HDR;


	  // ICMP header
	  typedef struct icmp_hdr
	  {
	  	unsigned char   icmp_type;
	  	unsigned char   icmp_code;
	  	unsigned short  icmp_checksum;
	  	unsigned short  _icmp_id;
	  	unsigned short  icmp_sequence;
	  	unsigned int   icmp_timestamp;

	  } ICMP_HDR;//, *PICMP_HDR, FAR *LPICMP_HDR;



	  // IPv6 protocol header
	  typedef struct ipv6_hdr
	  {
	  	unsigned long   ipv6_vertcflow;        // 4-bit IPv6 version, 8-bit traffic class, 20-bit flow label
	  	unsigned short  ipv6_payloadlen;       // payload length
	  	unsigned char   ipv6_nexthdr;          // next header protocol value
	  	unsigned char   ipv6_hoplimit;         // TTL
	  	struct in6_addr ipv6_srcaddr;          // Source address
	  	struct in6_addr ipv6_destaddr;         // Destination address
	  } IPV6_HDR;//, *PIPV6_HDR, FAR * LPIPV6_HDR;



	  // IPv6 fragment header
	  typedef struct ipv6_fragment_hdr
	  {
	  	unsigned char   ipv6_frag_nexthdr;
	  	unsigned char   ipv6_frag_reserved;
	  	unsigned short  ipv6_frag_offset;
	  	unsigned long   ipv6_frag_id;
	  } IPV6_FRAGMENT_HDR;//, *PIPV6_FRAGMENT_HDR, FAR * LPIPV6_FRAGMENT_HDR;


	  // ICMPv6 header
	  typedef struct icmpv6_hdr {

	  	unsigned char   icmp6_type;
	  	unsigned char   icmp6_code;
	  	unsigned short  icmp6_checksum;

	  } ICMPV6_HDR;



	  // ICMPv6 echo request body
	  typedef struct icmpv6_echo_request
	  {
	  	unsigned short  icmp6_echo_id;
	  	unsigned short  icmp6_echo_sequence;
	  } ICMPV6_ECHO_REQUEST;


	  // Define the UDP header
	  typedef struct udp_hdr
	  {
	  	unsigned short src_portno;       // Source port no.
	  	unsigned short dst_portno;       // Dest. port no.
	  	unsigned short udp_length;       // Udp packet length
	  	unsigned short udp_checksum;     // Udp checksum (optional)

	  } UDP_HDR, *PUDP_HDR;




  //Trace Route
	  /****Variables****/
	  char gDestination[256] = { 0 };           // Destination
	  char var[1] = { };

	  int gAddressFamily = AF_UNSPEC;     // Address family to use
	  int gProtocol = IPPROTO_ICMP;        // Protocol value
	  int s;
	  int packetlen;

	  struct addrinfo *dest;
	  struct addrinfo *local;


	/*****ICMPv6 Variables******/
	unsigned char icmpv6_tpye [1] = {0};
	unsigned char icmpv6_code [1] = {0};
    unsigned short icmpv6_checksum[1] = {0};
	unsigned int   icmpv6_reserved[1] = {0};
	unsigned int   icmpv6_temp[1]={0};
	unsigned char icmpv6_version [1] = {0};
	unsigned short icmpv6_payload_length[1]={0};
	unsigned char  icmpv6_nextheader[1]={0};
	unsigned char  icmpv6_hoplimit[1]={0};
	unsigned char  icmpv6_ipaddress[16] = {0} ;
	unsigned char  icmpv6_ipaddress_dest[16] = {0} ;


	/******PCAP Variable *******/
	struct pcap_pkthdr header;
   const unsigned char *packet;
   char errbuf[PCAP_ERRBUF_SIZE];
   char *device;
   pcap_t *pcap_handle;
   int i;

      /****Functions****/

	  void InitIcmpHeader(char *buf, int datasize);
	  void SetIcmpSequence(char *buf);
	  void ComputeIcmpChecksum(int s, char *buf, int packetlen, struct addrinfo *dest);
	  void process_packet();
	  void process_packet_debug();
	  unsigned short checksum(unsigned short *buffer, int size);
	  unsigned short ComputeIcmp6PseudoHeaderChecksum(int s, char *icmppacket, int icmplen, struct addrinfo *dest);

	  int InitIcmp6Header(char *buf, int datasize);
	  int PrintAddress(struct addrinfo *sa);
	  int set_ICMP_protocol(struct addrinfo *sa);
	  int route_socket(void);
	  int SetTtl(int s, int ttl);

	  unsigned int route_endpoint(void);
	  unsigned int route_protocol(void);

	  struct addrinfo *resolve_address(char *addr, char *port, int af, int type, int proto);


	void pcap_fatal(const char *failed_in, const char *errbuf);
	void dump(const unsigned char *data_buffer, const unsigned int length);
	void dump_formatted(const unsigned char *data_buffer, const unsigned int length);
	void packet_analyze(const unsigned char *data_buffer, const unsigned int length);



};


#endif /* INCLUDE_ICMP_HPP_ */
