/*
 * CommonHeader.hpp
 *
 *  Created on: Feb 3, 2019
 *      Author: isaac
 */

#ifndef INCLUDE_COMMONHEADER_HPP_
#define INCLUDE_COMMONHEADER_HPP_

#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <limits>
#include <istream>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <vector>


#include <sys/stat.h>
#include <sys/stat.h>

#include <curl/curl.h>


#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>

#include <pcap.h>


#define IP_RECORD_ROUTE     0x7
// ICMP6 protocol value
#define IPPROTO_ICMP6       58
// ICMP types and codes
#define ICMPV4_ECHO_REQUEST_TYPE   8
#define ICMPV4_ECHO_REQUEST_CODE   0
#define ICMPV4_ECHO_REPLY_TYPE     0
#define ICMPV4_ECHO_REPLY_CODE     0

#define ICMPV4_DESTUNREACH    3
#define ICMPV4_SRCQUENCH      4
#define ICMPV4_REDIRECT       5
#define ICMPV4_ECHO           8
#define ICMPV4_TIMEOUT       11
#define ICMPV4_PARMERR       12

// ICMP6 types and codes
#define ICMPV6_ECHO_REQUEST_TYPE   128
#define ICMPV6_ECHO_REQUEST_CODE   0
#define ICMPV6_ECHO_REPLY_TYPE     129
#define ICMPV6_ECHO_REPLY_CODE     0
#define ICMPV6_TIME_EXCEEDED_TYPE  3
#define ICMPV6_TIME_EXCEEDED_CODE  0

#define DEFAULT_DATA_SIZE      32       // default data size
#define DEFAULT_RECV_TIMEOUT   6000     // six second
#define DEFAULT_TTL            30       // default timeout


#endif /* INCLUDE_COMMONHEADER_HPP_ */
