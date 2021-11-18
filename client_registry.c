#include "client_registry.h"
#include "globals.h"
#include "user_registry.h"
#include "csapp.h"
#include "debug.h"
#include "client.h"




typedef struct node{
	CLIENT *client;
	struct node *next;
}node;



struct client_registry {
	pthread_mutex_t mutex;
	node *head;
	int totalclients;

} ;

static sem_t semaphore;




CLIENT_REGISTRY *creg_init(){
	debug("CLIENT_REGISTRY.C: Succesfully starting creg init");

	client_registry= malloc(sizeof(CLIENT_REGISTRY));
	if(client_registry == NULL){
		return NULL;
	}
	debug("CLIENT_REGISTRY.C: Succesfully created client registry, and malloced.");
	if(pthread_mutex_init(&client_registry->mutex,NULL) < 0 || sem_init(&semaphore,0,0) < 0){
		return NULL;
	}
	client_registry->head = NULL;
	client_registry->totalclients = 0;
	debug("CLIENT_REGISTRY.C: Succesfully created the head node, and set it equal to null");
	return client_registry;

}



void creg_fini(CLIENT_REGISTRY *cr){
		debug("CLIENT_REGISTRY.C: Succesfully started ran creg_fini");



	debug("past while loop");

	pthread_mutex_destroy(&cr->mutex);
	sem_destroy(&semaphore);
	free(cr);

	cr=NULL;
	debug("CLIENT_REGISTRY.C: Succesfully ran creg_fini");


}





CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd){

	pthread_mutex_lock(&cr->mutex);

		debug("CLIENT_REGISTRY.C: Start registering the new client");

	CLIENT* newclient = client_create(cr,fd);
	node *toinsert = malloc(sizeof(node));
	client_ref(newclient,"INCREASE DUE TO REGISTER");
	toinsert->client = newclient;
	toinsert->next = NULL;
	debug("CLIENT_REGISTRY.C: Succesfully created a node to insert, and a new client");

	if(cr->head == NULL){ //no users
		cr->head = toinsert;
		cr->totalclients = cr->totalclients +1;
		debug("CLIENT_REGISTRY.C: There are currently no clients so this is the first client");
		debug("CLIENT_REGISTRY.C: Total clients %d",cr->totalclients);
	}

	else{
		debug("CLIENT_REGISTRY.C: Current number of clients before updated %d",cr->totalclients);
		node* p = cr->head;
		while(p->next != NULL){
			p= p->next;
		}
		p->next = toinsert;

		int updatedlcients = cr->totalclients +1;
		cr->totalclients = updatedlcients;
		debug("CLIENT_REGISTRY.C: Added new client to list");
		debug("CLIENT_REGISTRY.C: Total clients %d",cr->totalclients);
	}
	pthread_mutex_unlock(&cr->mutex);
	debug("CLIENT_REGISTRY.C: Completed registering the new client");
	return newclient;
}




int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client){
	pthread_mutex_lock(&cr->mutex);

	debug("CLIENT_REGISTRY.C: Started unregistering the client");
	int clientfound = -1;
	node* nodetoremove = cr->head;
	node* previous = NULL;;

	while(nodetoremove != NULL){
		if(nodetoremove->client == client){
			clientfound = 1;
			client_unref(nodetoremove->client,"unref due to client is unregistered");
			debug("CLIENT_REGISTRY.C: Found the client to remove. ");
			break;
		}
	 	previous = nodetoremove; //
	 	nodetoremove = nodetoremove->next;

	 }

	 if(clientfound == -1){
	 	pthread_mutex_unlock(&cr->mutex);
	 	debug("CLIENT_REGISTRY.C: Client not found");
	 	return -1;
	 }




	previous->next = nodetoremove->next; //set the next of the previous linked to the next to the node to remove.
	 cr->totalclients = cr->totalclients -1;
	debug("CLIENT_REGISTRY.C: Total clients (Inside unregister) %d",cr->totalclients);

	 if(cr->totalclients == 0){
	 	pthread_mutex_unlock(&cr->mutex);
	 	V(&semaphore); //IF THE LINKED LIST OF CLIENTS IS EMPTY THEN YOU STOP THE SEMAPHROE THREAD
	 	free(nodetoremove);
	 	debug("CLIENT_REGISTRY.C: Current list of clients are at 0., we are V'ing the semaphore");
	 	return 0;
	 }
	pthread_mutex_unlock(&cr->mutex);
	free(nodetoremove);
	debug("CLIENT_REGISTRY.C: Completed unregistering the client");
	return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr){
	pthread_mutex_lock(&cr->mutex);

	debug("CLIENT_REGISTRY.C: Started creg_allclients %d",cr->totalclients);
	CLIENT ** array = malloc(sizeof(CLIENT*) * cr->totalclients + 2);
	node **nodearray = &cr->head;
	node *nextnode = *nodearray;
	int i =0;
	while(nextnode != NULL){
		CLIENT * current = nextnode->client;
		client_ref(current,"REF DUE TO CREG ALL CLIENTS");
		debug("CLIENT_REGISTRY.C: client and the current ref of it ");
		array[i] = current;
		i++;
		nextnode = nextnode->next;

	}

		 array[i] = NULL; //puts null terminator
		 debug("CLIENT_REGISTRY.C: Inserted null terminator");
		 pthread_mutex_unlock(&cr->mutex);
		 debug("CLIENT_REGISTRY.C: Finished creg_allclients");
		 return array;

		}


void creg_shutdown_all(CLIENT_REGISTRY *cr){
	debug("CLIENT_REGISTRY.C: Started creg_shutdown_all");
	node* temporary = cr->head;
	while(temporary != NULL){
		shutdown(client_get_fd(temporary->client),2);
		temporary = temporary->next;
	}

	P(&semaphore);
	V(&semaphore);
	debug("CLIENT_REGISTRY.C: Finished creg_shutdown_all");

	}






