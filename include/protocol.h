#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include "dict.h"

void parse_and_execute(Dict *db, char *buffer, int client_fd);
char* find_crlf(ClientState *client);

#endif