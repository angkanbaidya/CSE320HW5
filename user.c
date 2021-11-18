#include "user.h"
#include "csapp.h"
#include "debug.h"




 struct user {

	char* handle;
	int reference;

	pthread_mutex_t mutex;
};



USER *user_create(char *handle){
	debug("user.c 1");
	 USER *newuser = malloc(sizeof(USER));
	 if(newuser == NULL){
	 	return NULL;
	 }
	 if(pthread_mutex_init(&newuser->mutex,NULL) < 0){

	 	return NULL;
	 }

	 newuser->handle = malloc(strlen(handle));
	 strncpy(newuser->handle,handle,strlen(handle));
	 newuser->reference = 1;
	 return newuser;

}


USER *user_ref(USER *user, char *why){
		debug("user.c 2");

	pthread_mutex_lock(&user->mutex);
	user->reference = user->reference +1;
	pthread_mutex_unlock(&user->mutex);

	return user;


}


void user_unref(USER *user, char *why){
		debug("user.c 3");

	pthread_mutex_lock(&user->mutex);
	user->reference = user->reference -1;
	if(user->reference == 0){
		Free(user->handle);
		Free(user);
	}
		pthread_mutex_unlock(&user->mutex);
		pthread_mutex_destroy(&user->mutex);



}

char *user_get_handle(USER *user){
		debug("user.c 4");

	return user->handle;
}


