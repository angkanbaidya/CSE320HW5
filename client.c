#include "globals.h"
#include "user_registry.h"
#include "csapp.h"
#include "debug.h"
#include "client_registry.h"
#include "mailbox.h"

struct client{

 	int reference;
 	USER *user;
 	MAILBOX *box;
 	int filedescriptor;
 	pthread_mutex_t mutex;



 	};


static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t mutex;

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd){
	  debug("CLIENT.C 1");

	CLIENT *newclient = malloc(sizeof(CLIENT));
	if(newclient == NULL){
		return NULL;
	}
	pthread_mutex_init(&newclient->mutex,NULL);
	newclient->reference =1;
	newclient->box = NULL;
	newclient->user = NULL;
	newclient->filedescriptor = fd;
	 debug("CLIENT.C 1 CHECKING");

	return newclient;

}


void pthread_init(){
	debug("CLIENT.C 2");
	pthread_mutex_init(&mutex,NULL);
}


int client_login(CLIENT *client, char *handle){
	debug("CLIENT.C THIS IS THE HANDLE BEING INPUTTED %s",handle);
	pthread_once(&once,pthread_init);
	pthread_mutex_lock(&mutex);


	if(client->user != NULL){
		pthread_mutex_unlock(&mutex);
		debug("CLIENT.C 3 CHECKING 1");
		return -1;

	}
int i = 0;
	 CLIENT** array = creg_all_clients(client_registry);
	 CLIENT *current = array[i];

	 while(current->user != NULL){
	 	USER *currentuser = current->user;
	 	char *hdnl = user_get_handle(currentuser);
	 	debug("THIS IS THE HANDLE OF THE CURRENT USER BEING CHECKED %s",hdnl);
	 	if(strcmp(hdnl,handle) == 0){
	 		pthread_mutex_unlock(&mutex);
	 		free(array);
	 		return -1;
	 	}
	 	i++;
	 	current = array[i];
	 }
	 i = 0;
	 current = array[i];
	 debug("CLIENT.C 3 CHECKIN 4");

	 while(current->user != NULL){
	 	client_unref(current,"UNREF FOR CREG_ALL_CLIENTS");
	 	i++;
	 	current = array[i];
	 }
	 free(array);
	 debug("THIS IS THE HANDLE line 87 %s",handle);
	  USER *newuser = ureg_register(user_registry, handle);

	  client->user = newuser;
	  	MAILBOX *mailbox = mb_init(handle);
	  client->box = mailbox;
	   debug("THIS IS THE HANDLE OF THE MAILBOX BEING CALLED %s", mb_get_handle(client->box));

	  pthread_mutex_unlock(&mutex);
	  debug("CLIENT.C 3 COMPLETE");
	 return 0;
}

int client_logout(CLIENT *client){
	debug("CLIENT.C 4");
	pthread_mutex_lock(&client->mutex);
	if(client->user == NULL){
		pthread_mutex_unlock(&client->mutex);
		return -1;
	}
	mb_shutdown(client->box);
	mb_unref(client->box,"client logout");
	user_unref(client->user,"LOGOUT");
	client->box = NULL;
	client->user = NULL;
	debug("CLIENT.C 4 CHECKING");
pthread_mutex_unlock(&client->mutex);
    debug("CLIENT.C 4 CHECKING2");
	return 0;
}


CLIENT *client_ref(CLIENT *client, char *why){
	debug("CLICENT.C THIS IS THE REFERENCE BEFORE UPDATED %d",client->reference);
	pthread_mutex_lock(&client->mutex);
	client->reference = client->reference+1;
	debug("CLIENT.C this is the REFERENCE AFTER UPDATED OF THE CLIENT %d", client->reference);
	if(client->user != NULL)
		debug("THIS IS THE USER ASSOCIATED WITH THE CLIENT %s", user_get_handle(client->user));
	if(client->user == NULL)
		debug("NULL IN CLIENT REF");
	pthread_mutex_unlock(&client->mutex);

	return client;
}


void client_unref(CLIENT *client, char *why){
	debug("CLIENT.C 6");
		pthread_mutex_lock(&client->mutex);
	client->reference = client->reference-1;

	if(client->reference == 0){
		user_unref(client->user,"UNREF DUE TO CLIENT UNREF");
		debug("CALLING MB UNREF!!!!!!!!!!!");
		debug("THIS IS THE HANDLE OF THE MAILBOX BEING CALLED %s", mb_get_handle(client->box));
		free(client->box);
		free(client->user);
		free(client);
	}
	pthread_mutex_unlock(&client->mutex);
	pthread_mutex_destroy(&client->mutex);
	debug("CLIENT.C 6 CHECKING");
}




USER *client_get_user(CLIENT *client, int no_ref){
		debug("CLIENT.C 8");

	pthread_mutex_lock(&client->mutex);
	debug("CLIENT.C 8");
	debug("This is the handle of user attempting");

	if(client->user == NULL){
			pthread_mutex_unlock(&client->mutex);
			return NULL;
		}
	if(no_ref == 0){
		user_ref(client->user,"client_Get_user being called");
	}
	debug("CLIENT.C THIS IS THE RETURN USER HANDLE %s",user_get_handle(client->user));
	pthread_mutex_unlock(&client->mutex);
	return client->user;
}


MAILBOX *client_get_mailbox(CLIENT *client, int no_ref){
	pthread_mutex_lock(&client->mutex);

	debug("CLIENT.C 9");

		if(client->user == NULL){
			pthread_mutex_unlock(&client->mutex);
			return NULL;
		}
		if(no_ref == 0){
			debug("line 181 calling mb ref!!!!!!!");
			debug("THIS IS THE HANDLE OF THE MAILBOX BEING CALLED %s", mb_get_handle(client->box));
		mb_ref(client->box,"CLIENT GET MAILBOX IS CALLED");


	}
	debug("CLIENT.C 9 CHECKING");
	pthread_mutex_unlock(&client->mutex);
	return client->box;
}




int client_get_fd(CLIENT *client){
		debug("CLIENT.C 21");

	return client->filedescriptor;
}



int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data){
	debug("CLIENT.C 7");
	pthread_mutex_lock(&user->mutex);
	 int returnvalue =proto_send_packet(user->filedescriptor,  pkt,  data);
	 pthread_mutex_unlock(&user->mutex);
	 debug("CLIENT.C 7 checking") ;
	 return returnvalue;
}




int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen){
	debug("CLIENT.C 10");
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC,&start);
	CHLA_PACKET_HEADER *header = malloc(sizeof(CHLA_PACKET_HEADER));
	memset(header,0,sizeof(CHLA_PACKET_HEADER));
	header->type = CHLA_ACK_PKT;



	uint32_t payloadlength = datalen;
	uint32_t  sec = start.tv_sec;
	uint32_t nsec = start.tv_nsec;


	payloadlength = htonl(payloadlength);
	sec = htonl(sec);
	nsec = htonl(nsec);
	msgid = htonl(msgid);


	header->msgid = msgid;
	header->payload_length = payloadlength;
	header->timestamp_sec = sec;
	header->timestamp_nsec = nsec;
	int returnvalue = client_send_packet(client,header,data);
	free(header);
	debug("CLIENT.C 10 checking");
	return returnvalue;

}


int client_send_nack(CLIENT *client, uint32_t msgid){
	debug("CLIENT.C 11");
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC,&start);
	CHLA_PACKET_HEADER *header = malloc(sizeof(CHLA_PACKET_HEADER));
	memset(header,0,sizeof(CHLA_PACKET_HEADER));
	header->type = CHLA_NACK_PKT;



	uint32_t  sec = start.tv_sec;
	uint32_t nsec = start.tv_nsec;
	sec = htonl(sec);
	nsec = htonl(nsec);
	msgid = htonl(msgid);


	header->payload_length = 0;
	header->msgid = msgid;
	header->timestamp_sec = sec;
	header->timestamp_nsec = nsec;
	int returnvalue = client_send_packet(client,header,NULL);
	free(header);
	debug("CLIENT.C 11 CHECKING");
	debug("THIS IS THE RETURN VALUE OF SEND NACK%d",returnvalue);
	return returnvalue;

}





