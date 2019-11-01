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
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_returned_values(int *recv_length, char *buffer);
void print_fatal_msg(char *msg);
void send_command_print_resonse(int socket_server, char *command, char* buffer, int exit_upon_fail);
int char_strlen(const char *str);
void get_formatted_data_connection_info (const char *str, char *ip, int *port_one, int *port_two);
void fill_char_pointer_with_arr (char* pntr, int start, char* arr, int with_terminator);

int main(int argc, char *argv[]) {
	char *ip;
	int port;

	/*char ip_usr_str[17];
	char username_str[256];
	char password_str[1024];
	printf("Enter the IP for the FTP server:\n");
	scanf("%s", &ip_usr_str);
	printf("Enter the PORT for the FTP server:\n");
	scanf("%d", &port);
	printf("Enter the username:\n");
	scanf("%s", &username_str);
	printf("Enter the password:\n");
	scanf("%s", &password_str);*/

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		print_fatal_msg("server tcp socket creation has failed");
	}

	char *buffer = (char*)malloc(1024);
	memset(buffer, '\0', 1024);

	char *username_command = (char*)malloc(256);
	fill_char_pointer_with_arr (username_command, 0, "USER ", 0);

	char *pass_command = (char*)malloc(1024);
	fill_char_pointer_with_arr (pass_command, 0, "PASS ", 0);

	// argc contains 4 arguments
	// ip
	// port
	// username
	// password
	if (argc < 4) { // default intialization;
		ip = "127.0.0.1";
		port = 21;
		fill_char_pointer_with_arr (username_command, 5, "ftpuse", 1);
		fill_char_pointer_with_arr (pass_command, 5, "ssssss", 1);
	} else {
		ip = argv[0];
		port = strtoumax(argv[1], NULL, 10);
		fill_char_pointer_with_arr (username_command, 5, argv[2], 1);
		fill_char_pointer_with_arr (pass_command, 5, argv[3], 1);
	}

	struct sockaddr_in server_address;
	inet_aton(ip, &server_address.sin_addr);
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
	send_command_print_resonse(server_socket, command, buffer, 0);

	send_command_print_resonse(server_socket, username_command, buffer, 0);
	send_command_print_resonse(server_socket, pass_command, buffer, 1);

	command = "PASV\r\n";
	send_command_print_resonse(server_socket, command, buffer, 0);
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
	send_command_print_resonse(server_socket, command, buffer, 0);
	return 0;
}

void print_returned_values(int *recv_length, char *buffer) {
	printf("the received length is %d and the buffer is %s\n", *recv_length, buffer);
}

void print_fatal_msg(char *msg) {
    printf("FATAL ERROR: %s\n", msg);
    exit(1);
}

void send_command_print_resonse(int socket_server, char *command, char* buffer, int exit_upon_fail) {
	send(socket_server, command, char_strlen(command), 0);
	memset(buffer, '\0', 1024);
	int recv_length = recv(socket_server, buffer, 1024, 0);
	if(recv_length == -1) {
		printf("No returned value\n");
	} else {
		if (exit_upon_fail) {
			char* error = "530 Login incorrect.\r\n";
			char* returned_buffer = buffer;
			if (strcmp(error,returned_buffer) == 0) {
				print_fatal_msg(returned_buffer);
			}
		}

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

void fill_char_pointer_with_arr (char* pntr, int start, char* arr, int with_terminator) {
	int arr_size = char_strlen(arr);
	for(int index = 0; index < arr_size; index++) {
		pntr[start] = arr[index];
		start = start + 1;
	}

	if (with_terminator) {
		pntr[start++] = '\r';
		pntr[start++] = '\n';
	}
}
