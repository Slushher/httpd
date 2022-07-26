#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

bool setupSocket(unsigned short port);