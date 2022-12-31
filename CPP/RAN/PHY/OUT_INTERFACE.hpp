#pragma once 

#include "Headers.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 8090

class OUT_INTERFACE
{
	public:
	OUT_INTERFACE()
	{
	  //create a socket
	  // initial creation. When falls off should try to recreate the socket
	  this->openSocket();
  
	};
	~OUT_INTERFACE()
	{
	
	};
	
	int openSocket()
	{
 	m_socket_open = true;
  		if ((m_cliSockDes = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
  		{
    		// perror("socket creation error...\n");
    		// exit(-1);
    		m_socket_open = false;
    		return 1;
  		}
  
	m_serAddr.sin_family = AF_INET;
  	m_serAddr.sin_port = htons(PORT);
  	m_serAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  	
  	return 0;
	};

	
	int sendData(char* w_msg, size_t w_msg_length)
	{
   
	   if(  m_socket_open == true)
   		{
   		
   		/*
		serAddrLen = sizeof(serAddr);
  		readStatus = recvfrom(cliSockDes, buff, 1024, 0, (struct sockaddr*)&serAddr, &serAddrLen);
  		if (readStatus < 0) {
    			perror("reading error...\n");
    			close(cliSockDes);
    			exit(-1);
  		}
  		*/
     		if(w_msg_length>0)
     		{	
	  		if (sendto(m_cliSockDes, w_msg, w_msg_length, 0, (struct sockaddr*)&m_serAddr, sizeof(m_serAddr)) < 0) 
	  		{
		   		perror("sending error in UDP interface \n");
			   	close(m_cliSockDes);	
    				m_socket_open = false;
    				//exit(-1);
  			}
  		}
   	}	
   	else{
   	// attempt to open the socket;
   		this->openSocket();
   	}	
	 return 0;
	};
	public:
	
	int m_cliSockDes;
	bool m_socket_open = false;
	int m_readStatus;
  	struct sockaddr_in m_serAddr;
  	socklen_t m_serAddrLen;
};

