#include "protocol.h"
#include "csapp.h"
#include "debug.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){

	uint32_t old = ntohl(hdr->payload_length);





	if(rio_writen(fd,(void*)hdr,sizeof(CHLA_PACKET_HEADER)) < 0 ){
		return -1;
	}

	if(old != 0){

		if(rio_writen(fd,payload,old) <0){
			return -1;
		}


}

 return 0;
 }





int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
	if(rio_readn(fd,hdr,sizeof(CHLA_PACKET_HEADER)) <= 0){
		return -1;
	}

	uint32_t old = ntohl(hdr->payload_length);

	if(payload && old){

		char* temporary = Calloc(old,sizeof(char));


		if(rio_readn(fd,temporary,old) <= 0){
			Free(temporary);
			temporary = NULL;
			return -1;
		}
		*payload = temporary;

	}



	return 0;
}