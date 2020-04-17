#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include <unistd.h> 
#include<fstream>
using namespace std;

int read_count=0,write_count=0, var=0;
sem_t  rd,wrt;
string myText;

void *writer(void *i)
{

	sem_wait(&wrt);		//Lock the file for a writer
	
	var+=20;
	int x = *((int*)(&i));	//writer thread number
	
	cout<<"\nwriter"<<x<<" is writing";	
	ofstream file("file1.txt");
	file<<"value:"<<var<<endl;		//write in the file
	file.close();
	
	sem_post(&wrt);		//Release the file
}

void *reader(void *i)
{
	
	sem_wait(&rd);		//Lock the file for a Reader
	
	read_count++;
	if (read_count==1)
		sem_wait(&wrt);
	
	int x = *((int*)(&i));
	cout<<"\nReader"<< x <<" is reading";		
	
	sem_post(&rd);		//Release the file
	
	ifstream file("file1.txt");
	while(getline(file,myText))
	{
		cout<<endl<<myText;	// Read from the file
	}
	file.close();
	
	sem_wait(&rd);
	
	read_count--;
	if(read_count==0)
		sem_post(&wrt);
	
	sem_post(&rd);
}


int main()
{
	pthread_t read[10],write[10];
	sem_init(&rd,0,1);
	sem_init(&wrt,0,1);
	
	ofstream file;
	file.open("file1.txt");
	file <<"value:"<<0<<endl;
	file.close();
	
	for(int i=0;i<10;i++)
	{
		pthread_create(&write[i],NULL,writer,(void *)i);
	    	pthread_create(&read[i],NULL,reader,(void *)i);
	}
	
	for(int i=0;i<10;i++)
	{
		pthread_join(write[i],NULL);
	    	pthread_join(read[i],NULL);
	}
}
