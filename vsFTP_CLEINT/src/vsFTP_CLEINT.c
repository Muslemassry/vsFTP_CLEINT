/*
 ============================================================================
 Name        : FTP_CLEINT_TOOL.c
 Author      : Amaragy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_returned_values(int *recv_length, char *buffer);
void print_fatal_msg(char *msg);
void send_command_print_resonse(int socket_server, char *command, char* buffer);
int char_strlen(const char *str);
void get_formatted_data_connection_info (const char *str, char *ip, int *port_one, int *port_two);

int main(void) {
//	char *ip = "172.0.0.1";
	int port = 21;
//	char *username = "USER ftpuser";
//	char *password = "PASS ssssss";
	char *buffer = (char*)malloc(1024);
	memset(buffer, '\0', 1024);

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		print_fatal_msg("server tcp socket creation has failed");
	}

	struct sockaddr_in server_address;
//	inet_pton(AF_INET, ip, &(server_address.sin_addr));
	server_address.sin_addr.s_addr = 0;
	server_address.sin_port = htons(port);
	server_address.sin_family = AF_INET;
	memset(&(server_address.sin_zero), '\0', 8);

	// connect
	if (connect(server_socket,(struct sockaddr*)&server_address, sizeof(server_address))) {
		print_fatal_msg("Could not open connection to server");
	} else {
		int recv_length = recv(server_socket, buffer, 1024, 0);
		if(recv_length == -1) {
			printf("No returned value\n");
		} else {
			printf("the received length is %d and the buffer is %s\n", recv_length, buffer);
		}
	}

	char *command = "ftp localhost 21\r\n";
	send_command_print_resonse(server_socket, command, buffer);
	command = "USER ftpuser\r\n";
	send_command_print_resonse(server_socket, command, buffer);
	command = "PASS ssssss\r\n";
	send_command_print_resonse(server_socket, command, buffer);

	command = "PASV\r\n";
	send_command_print_resonse(server_socket, command, buffer);
	printf("the buffer is: %s", buffer);
	char *ip_str = (char*)malloc(17);
	memset(ip_str, '\0', 17);
	int port_one, port_two;
	get_formatted_data_connection_info (buffer, ip_str, &port_one, &port_two);

	printf("=========>> ip_str: %s\n", ip_str);
	printf("=========>> port_one: %d\n", port_one);
	printf("=========>> port_two: %d\n", port_two);
	int port_number_for_listen = (port_one * 256) + port_two;

	int server_data_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_data_address;
//	inet_aton(ip_str, &loca_data_address.sin_addr);
	server_data_address.sin_addr.s_addr = 0;
	server_data_address.sin_port = htons(port_number_for_listen);
	server_data_address.sin_family = AF_INET;
	memset(&(server_data_address.sin_zero), '\0', 8);

	command = "LIST\r\n";
	send(server_socket, command, char_strlen(command), 0);

	if (connect(server_data_socket,(struct sockaddr*)&server_data_address, sizeof(server_data_address))) {
		print_fatal_msg("Could not open connection to server");
	} else {
		memset(buffer, '\0', 1024);
		int recv_length = recv(server_data_socket, buffer, 1024, 0);
		if(recv_length == -1) {
			printf("No returned value\n");
		} else {
			printf("the received length is %d and the buffer is %s\n", recv_length, buffer);
		}
	}

	command = "QUIT\r\n";
	send_command_print_resonse(server_socket, command, buffer);
	return 0;
}

void print_returned_values(int *recv_length, char *buffer) {
	printf("the received length is %d and the buffer is %s\n", *recv_length, buffer);
}

void print_fatal_msg(char *msg) {
    printf("FATAL ERROR: %s\n", msg);
    exit(1);
}

void send_command_print_resonse(int socket_server, char *command, char* buffer) {
	send(socket_server, command, char_strlen(command), 0);
	memset(buffer, '\0', 1024);
	int recv_length = recv(socket_server, buffer, 1024, 0);
	if(recv_length == -1) {
		printf("No returned value\n");
	} else {
		printf("the received length is %d and the buffer is %s\n", recv_length, buffer);
	}

	return;
}

int char_strlen(const char *str)
{
	int i;
	for (i = 0; str[i]; i++);
	return i;
}

void get_formatted_data_connection_info (const char *str, char *ip, int *port_one, int *port_two) {
	char *port_one_str = (char*)malloc(6);
	char *port_two_str = (char*)malloc(6);
	memset(port_one_str, '\0', 6);
	memset(port_two_str, '\0', 6);

    int i;
    int str_length = char_strlen(str);
    int to_start = 0;
    int comma_number = 0;
    int ip_index = 0;
    int port_one_index = 0;
    int port_two_index = 0;
    for (i = 0; i < str_length; i++) {
    	if (str[i] =='(') {
    		to_start = 1;
    	}

    	if (to_start) {
			if (str[i] == ')') {
				break;
			}

			if (str[i] == ',') {
				comma_number++;
			}

			if (comma_number < 4 && (str[i] != '(')) {
				if (str[i] == ',') {
					ip[ip_index] = '.';
				} else {
					ip[ip_index] = str[i];
				}

				ip_index++;
			}

			if (comma_number == 4 && (str[i] != ',')) {
				port_one_str[port_one_index] = str[i];
				port_one_index++;
			}

			if (comma_number == 5 && (str[i] != ',')) {
				port_two_str[port_two_index] = str[i];
				port_two_index++;
			}
		}
    }

    *port_one = strtoumax(port_one_str, NULL, 10);
    *port_two = strtoumax(port_two_str, NULL, 10);
}
