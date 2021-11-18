#include "globals.h"
#include "user_registry.h"
#include "csapp.h"
#include "debug.h"




typedef struct node{
	USER *user;
	struct node *next;
}node;






struct user_registry{

	node *head;
	pthread_mutex_t mutex;
};






USER_REGISTRY *ureg_init(void){
	debug("USER_REGISTRY 1");

user_registry = malloc(sizeof(USER_REGISTRY));
pthread_mutex_init(&user_registry->mutex,NULL);
user_registry->head = NULL;
debug("USER_REGISTRY.C: finishing REGISTRY");

return user_registry;
}




void ureg_fini(USER_REGISTRY *ureg){
		debug("USER_REGISTRY 2");

	pthread_mutex_lock(&ureg->mutex);
	node* temp;
	USER* temp2;
	while(ureg->head != NULL){
		temp = ureg->head;
		temp2 = temp->user;
		ureg->head = ureg->head->next;
		user_unref(temp2,"UREG_FINI");
		free(temp);

	}
	pthread_mutex_unlock(&ureg->mutex);
	pthread_mutex_destroy(&ureg->mutex);
	free(ureg);

}



USER *ureg_register(USER_REGISTRY *ureg, char *handle){

	debug("USER_REGISTRY.C: starting UREG_REGISTRY");

	pthread_mutex_lock(&ureg->mutex);


	USER *newuser = user_create(handle);
	debug("USER_REGISTRY.C: starting UREG_REGISTRY2");
	node *toinsert = malloc(sizeof(node));
	debug("USER_REGISTRY.C: starting UREG_REGISTRY3");
	user_ref(newuser,"INCREASE DUE TO REGISTER");
	debug("USER_REGISTRY.C: starting UREG_REGISTRY4");
	toinsert->user = newuser;
	debug("USER_REGISTRY.C: starting UREG_REGISTRY5");
	toinsert->next = NULL;
 if(ureg->head == NULL){ //no users
 	ureg->head = toinsert;
 }

 else{

 	node* p = ureg->head;
	 while(p->next != NULL){ //while the next of the head is not null
	 	if(strcmp(user_get_handle(p->user),handle) ==0){
	 		pthread_mutex_unlock(&ureg->mutex);
	 		return p->user;
	 	}
	 	p = p->next;

	 }
	 p->next = toinsert;


	}
	pthread_mutex_unlock(&ureg->mutex);
	debug("USER_REGISTRY.C: finishing UREG_REGISTRY");

	return newuser;
}



void ureg_unregister(USER_REGISTRY *ureg, char *handle){
		debug("USER_REGISTRY 4");

	pthread_mutex_lock(&ureg->mutex);
	node* nodetoremove = ureg->head;
	node *previous;

if(nodetoremove != NULL && (strcmp(user_get_handle(nodetoremove->user),handle) ==0)){ //if the head is the handle
	ureg->head = nodetoremove->next; //remove from registry
	user_unref(nodetoremove->user,"user is unregistered."); // unregister to user
	free(nodetoremove);
}

while(nodetoremove != NULL && (strcmp(user_get_handle(nodetoremove->user),handle) !=0)){
	previous = nodetoremove; //
	nodetoremove = nodetoremove->next;

}


user_unref(nodetoremove->user,"user is unregistered");
previous->next = nodetoremove->next;
pthread_mutex_unlock(&ureg->mutex);
free(nodetoremove);


}