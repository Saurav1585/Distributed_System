#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define SIZE 's'
#define DATA 'd'
#define ACK 'a'
#define END 'e'

#define PORT 4333
using namespace std;

int frame_number=0;

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



int main()
{
	int server_id, new_socket, valread, client_id;
	struct sockaddr_in address;
	struct sockaddr_in client_addr;
	int addrlen = sizeof(address);
	char *buffer = (char*)malloc(512 * sizeof(char));
	char *temp = (char*)malloc(100*sizeof(char));
	strcpy(temp, "Requesting for file");

	if ((server_id = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(3333);

	if (bind(server_id, (struct sockaddr* )&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	socklen_t len = sizeof(client_addr);
	sendto(server_id, temp, 100, 0, (struct sockaddr*)&client_addr, len);
	recvfrom(server_id, buffer, 512, 0, (struct sockaddr*)&client_addr, &len);
	
	int size = 0;
	if (buffer[0] == SIZE)
	{
		int i = 1;
		while (buffer[i])
		{
			size = size * 10 + (int(buffer[i]) - 48);
			i++;
		}
	}
	
	else
	{
		//printf("Unknown size of the file\n");
		exit(EXIT_FAILURE);
	}
	printf("File is of size: %d\n", size);
	
	
	srand(time(NULL));
	FILE* out;
	out = fopen("out.c", "w");
	
	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		recvfrom(server_id, buffer, 512, 0, (struct sockaddr*)&client_addr, &len);
		bool checked = true;
		
		
		char recv[512];
		for (int i = 0; i < sizeof(buffer) / sizeof(char); i++) recv[i] = buffer[10+i];
		
		
		char carry_recv = buffer[1], carry = '0';
		char *check_sum, *calculated, *binary_value;
		
		calculated = (char*)malloc(9 * sizeof(char));
		binary_value = (char*)malloc(9 * sizeof(char));
		check_sum = (char*)malloc(9 * sizeof(char));
		for (int i = 0; i < 8; i++) {*(check_sum+i) = buffer[2+i];cout<<buffer[2+i];}
		strcpy(calculated, "00000000");
		cout<<endl;
		
		int i = 0;
		while (recv[i] != '\0')
		{
			memset(binary_value, 0, sizeof(binary_value));
			strcpy(binary_value,"00000000");
			bin(recv[i], binary_value);
			checksum(binary_value, calculated, &carry);
			i++;
		}
		
		for (int i = 0; i < 8; i++)
		{
			if (*(calculated+i) == '0') *(calculated+i) = '1';
			else *(calculated+i) = '0';
		}
		
		
		if (carry != carry_recv) checked = false;
		for (int i = 0; i < 8; i++)
		{
			cout<<*(calculated+i);
			if (*(check_sum+i) != *(calculated+i)) checked = false;
		}
		
		
		if (buffer[0] != END && !checked) cout << "Checksum Mismatch\n";
		
		if (buffer[0] == END) checked = true;
		
		if (rand() % 10 != 0 && checked && (buffer[0] == END || buffer[0] == frame_number))
		{
			frame_number++;
			printf("Received -- \"%s\"\n",recv);
			fputs(recv, out);
			memset(temp, 0, sizeof(temp));
			temp[0] = ACK;
			sendto(server_id, temp, 100, 0, (struct sockaddr*)&client_addr, len);
			
			if (buffer[0] == END) break;
		}
		
		//else cout << "\nMissed Ack\n";
	}
	
	memset(temp, 0, sizeof(temp));
	temp[0] = END;
	sendto(server_id, temp, 100, 0, (struct sockaddr*)&client_addr, len);
	printf("\n\t\t\tFile Received Sucessfully!\n");
	
	fclose(out);
	
	return 0;
	
}
