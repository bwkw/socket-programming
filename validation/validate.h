#ifndef VALIDATE_H
#define VALIDATE_H

int is_valid_ip_address(const char* ipAddress);
int is_valid_port(const char* port_str);
void parse_command_line_arguments(int argc, char* argv[], char** server_ip, int* port);

#endif
