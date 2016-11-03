

#include <stdio.h>	
#include <unistd.h>  
#include <pthread.h>  
#include <ev.h>  

struct ev_loop *loop = NULL;  
struct ev_loop *loop2 =NULL;
static ev_async async_watcher;

int usr1 =0;
int term =0;


static void async_cb(EV_P_ ev_async *w, int revents)  
{  
		usr1 = 0;
		printf("async_cb() call, usr1 = %d\n", usr1);  
		if(term ==1)
			ev_break(loop2,EVBREAK_ALL);
}  
void signal_usr1_action(struct ev_loop *main_loop,ev_signal signal_w,int e){
		printf("get usr1 signal\n");
		ev_async_send(loop2,&async_watcher);
		usr1 =1;
}
void signal_term_action(struct ev_loop *main_loop,ev_signal signal_w,int e){
		printf("get term signal\n");		
		ev_async_send(loop2,&async_watcher);
		term =1;
		ev_break(loop,EVBREAK_ALL);
}

  
void *ev_create(void *p)  
{  
		printf("ev_create() call, start!\n");  
		loop2 = ev_loop_new(EVFLAG_AUTO);  
		ev_async_init(&async_watcher, async_cb);  
		ev_async_start(loop2, &async_watcher);		
		ev_run(loop2, 0); // 如果不在回调中调用stop或者break方法，ev_run后面的代码就永远不会被执行。  
		printf("ev_create() call, ev_run end!\n");
		pthread_exit((void*)1);
}  

int main()	
{  
		pthread_t tid;

		struct ev_signal sig_usr1;
		struct ev_signal sig_term;
		void* thread;


		loop = ev_default_loop (0);
		/******************signal register*******************************************/
		ev_signal_init(&sig_usr1,signal_usr1_action,SIGUSR1);
		ev_signal_start(loop,&sig_usr1);
		ev_signal_init(&sig_term,signal_term_action,SIGTERM);
		ev_signal_start(loop,&sig_term);
		/******************pthread*******************************************/
		pthread_create(&tid, NULL, ev_create, NULL); 
		sleep(1); // 要等子线程先注册完事件之后，才可以调用下面的ev_async_send函数，否则你懂的。 
		ev_run(loop,0);
		
		pthread_join(tid,&thread);
		return 0;  
}  

