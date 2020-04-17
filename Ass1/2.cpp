#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include <unistd.h>
#include<fstream> 

using namespace std;

int read_cnt=0,write_cnt=0,var=0;
sem_t  rd,wrt,readTry,resource;
string myText;

void *reader(void *i){
	sem_wait(&readTry);
	sem_wait(&rd);
	read_cnt++;
	if (read_cnt==1)
		sem_wait(&resource);
	sem_post(&rd);
	sem_post(&readTry);
	
	//critical section begin
	int x = *((int*)(&i));	
	cout<<"\nReader"<< x <<" is reading";
	ifstream file("file2.txt");
	while(getline(file,myText)){
		cout<<endl;
		cout<<myText;
	}
	file.close();
	
	sem_wait(&rd);
	read_cnt--;
	if(read_cnt==0)
		sem_post(&resource);
	sem_post(&rd);
}

void *writer(void *i){
	
	
	sem_wait(&wrt);
	write_cnt++;
	if (write_cnt==1)
		sem_wait(&readTry);
	sem_post(&wrt);	
	
	sem_wait(&resource);//critcical section begin
	int x = *((int*)(&i));
	cout<<"\nwriter"<<x<<" is writing";
	var+=5;
	ofstream file("file2.txt");
	file<<"variable:"<<var<<endl;
	file.close();
	sem_post(&resource);
	
	sem_wait(&wrt);
	write_cnt--;
	if (write_cnt==0)
		sem_post(&readTry);
	sem_post(&wrt);
}

int main(){

	pthread_t r[10],w[10];
	sem_init(&rd,0,1);
	sem_init(&wrt,0,1);
	sem_init(&readTry,0,1);
	sem_init(&resource,0,1);
	ofstream file;
	file.open("file2.txt");
	file <<"variable:"<<0<<endl;
	file.close();
	for(int i=0;i<10;i++)
	  {
	    pthread_create(&w[i],NULL,writer,(void *)i);
	    pthread_create(&r[i],NULL,reader,(void *)i);
	  }
	for(int i=0;i<10;i++)
	  {
	    pthread_join(w[i],NULL);
	    pthread_join(r[i],NULL);
	  }
}

