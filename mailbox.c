#include "client_registry.h"
#include "globals.h"
#include "user_registry.h"
#include "csapp.h"
#include "debug.h"
#include "client.h"
#include "mailbox.h"


typedef struct node{
	MAILBOX_ENTRY *entry;
	struct node *next;
}node;




 struct mailbox{
 	node *head;
 	pthread_mutex_t mutex;
 	char* handle;
 	int reference;
 	int totalamount;
 	MAILBOX_DISCARD_HOOK *hook;
 	int isdefunct;
 	sem_t semophore;
 };


/*
 * Create a new mailbox for a given handle.  A private copy of the
 * handle is made.  The mailbox is returned with a reference count of 1.
 */

MAILBOX *mb_init(char *handle){
	debug("MAILBOX.C: Initializing mailbox");
	MAILBOX *mailbox = malloc(sizeof(MAILBOX));
	mailbox->reference = 1;
	mailbox->totalamount = 0;
	mailbox->isdefunct = 0;
	mailbox->head = NULL;
	mailbox->handle = malloc(strlen(handle));
	strncpy(mailbox->handle,handle,strlen(handle));
	debug("MAILBOX.C: THIS IS THE HANDLE OF THE MAILBOX %s",mailbox->handle);
	if(pthread_mutex_init(&mailbox->mutex,NULL) < 0 || sem_init(&mailbox->semophore,0,0) < 0){
		return NULL;
	}
	debug("MAILBOX.C: Completed mailbox");
	return mailbox;
}


void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *hook){
	debug("MAILBOX.C: Starting hook");
	mb->hook =hook;
	debug(" MAILBOX.C: Completed hook");

}

void mb_ref(MAILBOX *mb, char *why){
	debug("MAILBOX.C: Starting Reference");

	debug("Mailbox.c Reference before %d:",mb->reference);

	pthread_mutex_lock(&mb->mutex);

	debug("Mailbox.c Reference before %d:",mb->reference);
	debug("Mailbox.c HANDLE before %s:",mb->handle);

	mb->reference = mb->reference +1;
	pthread_mutex_unlock(&mb->mutex);

	debug("MAILBOX.C: Finishing Reference");
}

void mb_unref(MAILBOX *mb, char *why){
	debug("MAILBOX.C: Starting UnReference");
	pthread_mutex_lock(&mb->mutex);
	debug("Mailbox.c: Handle of user being dereferenced mailbox %s",mb->handle);
	debug("Mailbox.c Reference before %d:",mb->reference);
	mb->reference = mb->reference -1;
	debug("Mailbox.c Reference after %d:",mb->reference);



	if(mb->reference ==0){

		node* temp;

	while(mb->head != NULL){
		temp = mb->head;
		mb->head =mb->head->next;
 		free(temp);
 	}



		free(mb->handle);
		free(mb);

	}



	pthread_mutex_unlock(&mb->mutex);
	pthread_mutex_destroy(&mb->mutex);
	sem_destroy(&mb->semophore);


	debug("MAILBOX.C: Finishing UnReference");

}



void mb_shutdown(MAILBOX *mb){
	debug("MAILBOX.C: Starting shutdown");
	mb->isdefunct = 1;
	V(&mb->semophore);
	debug("MAILBOX.C: Ending shutdown");
}

char *mb_get_handle(MAILBOX *mb){
		debug("MAILBOX.C: Getting handle:%s",mb->handle);

	return mb->handle;
}



void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length){
debug("MAILBOX.C: Starting add message");
pthread_mutex_lock(&mb->mutex);


MAILBOX_ENTRY *mailboxentry = malloc(sizeof(MAILBOX_ENTRY));

mailboxentry->content.message.length = length;
mailboxentry->type  = MESSAGE_ENTRY_TYPE;
mailboxentry->content.message.msgid =msgid;
mailboxentry->content.message.from = from;


mailboxentry->content.message.body = calloc(length,sizeof(char));
memcpy(mailboxentry->content.message.body,body,length);
if(mb != from){
	mb_ref(from,"UNREF IN MB_ADD_MESSAGE");
}

node *current;
node *node = malloc(sizeof(node));
node->entry = mailboxentry;




if(mb->head == NULL){
	mb->head = node;

}
else{
	current =  mb->head;
	while(current != NULL){
		current = current ->next;

	}
	current->next = node;
}


	mb->totalamount = mb->totalamount +1;
	pthread_mutex_unlock(&mb->mutex);
	V(&mb->semophore);


}


void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid){
	debug("MAILBOX.C: Starting add notice");

	pthread_mutex_lock(&mb->mutex);

	MAILBOX_ENTRY *mbentry = malloc(sizeof(MAILBOX_ENTRY));
	mbentry->type = NOTICE_ENTRY_TYPE;
	mbentry->content.notice.msgid = msgid;
	mbentry->content.notice.type = ntype;

	node *current;
	node *node = malloc(sizeof(node));
	node->entry = mbentry;

	if(mb->head == NULL){
	mb->head = node;

}
else{
	current =  mb->head;
	while(current != NULL){
		current = current ->next;

	}
	current->next = node;
}


	mb->totalamount = mb->totalamount +1;
	pthread_mutex_unlock(&mb->mutex);
	V(&mb->semophore);


}



MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb){
		debug("MAILBOX.C: Starting next entry");

	P(&mb->semophore);
			debug("MAILBOX.C: ENTRY 1");

	if(mb->isdefunct == 1){
		return NULL;
	}

	pthread_mutex_lock(&mb->mutex);
	debug("MAILBOX.C: ENTRY 2");

	if(mb->head == NULL){
		pthread_mutex_unlock(&mb->mutex);
		return NULL;
	}
					debug("MAILBOX.C: ENTRY 3");

	node *current = mb->head;
	MAILBOX_ENTRY *firstentry= current->entry; //first

	if(firstentry->type == MESSAGE_ENTRY_TYPE){
		mb_unref(firstentry->content.message.from,"UNREF DUE TO MB_NEXT_ENTRY");

	}

	if(current->next != NULL){
		free(mb->head);
		mb->head = current->next;

	}

	else{
		free(mb->head);
		mb->head = NULL;


	}

	mb->totalamount = mb->totalamount -1;

	pthread_mutex_unlock(&mb->mutex);

	return firstentry;


}
