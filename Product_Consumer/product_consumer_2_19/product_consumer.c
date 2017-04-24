#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<assert.h>

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mycond = PTHREAD_COND_INITIALIZER;

typedef struct _node
{
	int data;
	struct _node* next;
}node,*pnode,**ppnode;

pnode alloc_node(int data)
{
	pnode _n = (pnode)malloc(sizeof(node));
	if( _n == NULL )
	{
		perror("malloc");
		return NULL;
	}
	_n->data = data;
	_n->next = NULL;
	return _n;
}

void InitList(ppnode _h)
{
	*_h = alloc_node(0);
}

void PushFront(pnode _h,int data)
{
	assert(_h);
	pnode ret = alloc_node(data);
	ret->next = _h->next;
	_h->next = ret;
}
void DelNode(pnode node)
{
	assert(node);
	free(node);
}
void PopFront(pnode _h,int* data)
{
	assert(_h);
	assert(data);
	if(_h->next == NULL)
	{
		printf("list is empty!\n");
		return ;
	}

	pnode del = _h->next;
	_h->next = del->next;
	*data = del->data;
	DelNode(del);
}

void destory(pnode _h)
{
	assert(_h);
	pnode start = _h;
//	pnode del = _h;
	int data = 0;
	while(start->next)
	{
		PopFront(start,&data);
//		del = start;
//		start = del->next;
//		free(del);
//		del = NULL;
  }
	DelNode(_h);
}

void show(pnode _h)
{
	assert(_h);
	pnode start = _h->next;
	while(start)
	{
		printf("%d ",start->data);
		start = start->next;
	}
	printf("\n");
}

void* thread_product(void* arg)
{
	pnode node = (pnode)arg;
	while(1)
	{
		usleep(123123);
		int data = rand()%10000;
		pthread_mutex_lock(&mylock);
		PushFront(node,data);
		pthread_mutex_unlock(&mylock);
		pthread_cond_signal(&mycond);
		printf("product:%d \n",data);
	}
}

void* thread_consumer(void* arg)
{
	pnode node = (pnode)arg;
	int data = 0;
	while(1)
	{
		usleep(1000);
		pthread_mutex_lock(&mylock);
		while(node->next == NULL)
		{
			pthread_cond_wait(&mycond,&mylock);
		}
		PopFront(node,&data);
		pthread_mutex_unlock(&mylock);
		printf("consumer:%d \n",data);
	}
}

void test_list(pnode head)
{
	int i = 0;
	for(; i<10; i++)
	{
		PushFront(head,i);
		show(head);
	}
	int data = 0;
	for(; i>=0; i--)
	{
		PopFront(head,&data);
		show(head);
	}
	destory(head);
	show(head);
}

int main()
{
	pnode head = NULL;
	InitList(&head);
//	test_list(head);

	pthread_t tid1;
	pthread_t tid2;

	pthread_create(&tid1,NULL,thread_product,(void*)head);
	pthread_create(&tid2,NULL,thread_consumer,(void*)head);

	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);

	pthread_mutex_destroy(&mylock);
	pthread_cond_destroy(&mycond);
	return 0;
}
