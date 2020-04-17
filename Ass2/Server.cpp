#include <iostream>
#include <stdio.h>
#include <cstring>
#include <thread>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#define SIZE 's'
#define DATA 'd'
#define ACK 'a'
#define END 'e'

#define PORT 3333
using namespace std;

int server_id, new_socket, client_id;
struct sockaddr_in address, client_addr;
int addrlen = sizeof(address);
socklen_t len = sizeof(client_id);
int frame_number=0;

void receive_ack(bool *ack)
{
	char recv[512];
	while (true)
	{
		recvfrom(server_id, recv, 512, 0, (struct sockaddr*)&client_addr, &len);
		if (recv[0] == ACK)	
		{
			*ack = true;
		}
		else if (recv[0] == END)
		{
			*ack = true;
			break;
		}
	}
}


void bin(char c, char *binary_value)
{
	int a = c, i = 7;
	while (a > 0)
	{
		*(binary_value+i) = (char)(a % 2 + 48);
		a /= 2;
		i--;
	}
}


void checksum(char *c, char *check_sum, char *carry)
{
	for (int i = 7; i >= 0; i--)
	{
		if (*(c+i) == '0' && *(check_sum+i) == '0' && *carry == '0') 
		{
			*(check_sum+i) = '0';
			*carry = '0';
		}
		if ((*(c+i) == '0' && *(check_sum+i) == '0' && *carry == '1') || (((*(c+i) == '1' && *(check_sum+i) == '0') || *(c+i) == '0' && *(check_sum+i) == '1') && *carry == '0'))
		{
			*(check_sum+i) = '1';
			*carry = '0';
		}
		if ((*(c+i) == '1' && *(check_sum+i) == '1' && *carry == '0') || (((*(c+i) == '1' && *(check_sum+i) == '0') || *(c+i) == '0' && *(check_sum+i) == '1') && *carry == '1'))
		{
			*(check_sum+i) = '0';
			*carry = '1';
		}
		if (*(c+i) == '1' && *(check_sum+i) == '1' && *carry == '1') 
		{
			*(check_sum+i) = '1';
			*carry = '1';
		}
	}
	
}

long int findSize(char file_name[]) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
}

int main()
{
	// to calculate the size of the file
	FILE *send;
	//send = fopen("send", "r");
	
	int size_of_send = 4;
	char ch[2];
	char buffer[512];
	char recv[512];
	char frame[512];
	int digits = 0;
	bool ack = false;
	
	char file_name[] = { "send" }; 
    	long int size = findSize(file_name);
    	int n =size;
    	while (n != 0) { 
        	n = n / 10; 
        	++digits; 
    	} 
    	
    	
	
	if ((server_id = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	if (bind(server_id, (struct sockaddr* )&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	// socket created
	printf("Socket Created\n");

	
	srand(time(NULL));
	
	// to send the size of the file to be transfered
	while (1)
	{
		printf("\n\nWaiting for a Client\n");
		recvfrom(server_id, recv, 512, 0, (struct sockaddr*)&client_addr, &len);
		
		//printf("File Requested\n");
		//printf("Size of the file is: %d\n", size);
		memset(buffer, 0, sizeof(buffer));
		
		buffer[0] = SIZE;
		while (digits)
		{
			buffer[digits--] = char(size % 10 + 48);
			size /= 10;
		}
		
		sendto(server_id, buffer, 512, 0, (struct sockaddr*)&client_addr, len);
		

		
		// file transfer  using stop and wait
		
		send = fopen("send", "r");
		
		thread receiving(&receive_ack, &ack);
		while (fgets(buffer, size_of_send, send))
		{
			ack = false;
			int retries = 0;
			
			// For Checksum
			char *binary_value, *check_sum, carry = '0';
			binary_value = (char*)malloc(9 * sizeof(char));
			check_sum = (char*)malloc(9 * sizeof(check_sum));
			strcpy(check_sum, "00000000");
			int i = 0;
			while (buffer[i] != '\0')
			{
				memset(binary_value, 0, sizeof(binary_value));
				strcpy(binary_value,"00000000");
				bin(buffer[i], binary_value);
				checksum(binary_value, check_sum, &carry);
				i++;
			}
			
			for (int i = 0; i < 8; i++)
			{
				if (*(check_sum+i) == '0') *(check_sum+i) = '1';
				else *(check_sum+i) = '0';
			}
			
			// cout << check_sum << endl;
			// Checksum Created
			
			// Data Frame
			frame[0] = frame_number++;
			frame[1] = carry;
			for (int i = 0; i < 8; i++) frame[2+i] = *(check_sum+i);
			strcat(frame, buffer);
			// cout << "Current Frame: " << frame << endl;
			// Data Frame Created
			
			while (!ack)
			{
				char *sending_frame;
				sending_frame = (char*)malloc(512 * sizeof(sending_frame));
				memset(sending_frame, 0, sizeof(sending_frame));
				strcpy(sending_frame, frame);
				//srand(time(0));
				int x=rand()%10;
				printf ("random : %d\n", x);
				if (x == 0)
				{
					if (sending_frame[rand() % 9 + 1] == '0') sending_frame[rand() % 9 + 1] = '1';
					else sending_frame[rand() % 9 + 1] = '0';
				}
				if (retries == 0) printf ("Data Send  \"%s\"\n", buffer);
				else printf ("Resending .............\"\n");
				
				sendto(server_id, sending_frame, 512, 0, (struct sockaddr*)&client_addr, len);
				timeval init_clock;
				timeval curr_time;
				
				gettimeofday(&init_clock, NULL);
				gettimeofday(&curr_time, NULL);
				while (curr_time.tv_usec < init_clock.tv_usec + 1000) 
				{
					gettimeofday(&curr_time, NULL);
					if (ack)
					{
						printf("\nAcknowledgement Received after %d tries!\n", retries+1);
						break;
					}
				}
				retries++;
			}
			memset(frame, 0, sizeof(frame));
			memset(buffer, 0, sizeof(buffer));
		}
		
		ack = false;
		memset(frame, 0, sizeof(frame));
		frame[0] = END;
		
		while (!ack)
		{
			sendto(server_id, frame, 512, 0, (struct sockaddr*)&client_addr, len);
			timeval init_clock;
			timeval curr_time;
			
			gettimeofday(&init_clock, NULL);
			gettimeofday(&curr_time, NULL);
			while (curr_time.tv_usec < init_clock.tv_usec + 1000) 
			{
				gettimeofday(&curr_time, NULL);
				if (ack) break;
			}	
		}
		
		printf("\n\t\t\tFile Successfully Transfered!\n");
		
		receiving.join();
		fclose(send);
	}
	
	exit(0);
}
