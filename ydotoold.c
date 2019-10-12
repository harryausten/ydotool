/*
    This file is part of ydotool.
	Copyright (C) 2019 Harry Austen
    Copyright (C) 2018-2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* System includes */
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
/* Local includes */
#include "uinput.h"

void * client_handler(void * fdp) {
	struct uinput_raw_data buf;
    int fd = *(int *)fdp;

	for (;;) {
		int rc = recv(fd, &buf, sizeof(buf), MSG_WAITALL);

		if (rc == sizeof(buf)) {
			uinput_emit(buf.type, buf.code, buf.value);
		} else {
			break;
		}
	}

    pthread_exit(NULL);
}

int main() {
	const char path_socket[] = "/tmp/.ydotool_socket";
	unlink(path_socket);
	int fd_listener = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd_listener == -1) {
		fprintf(stderr, "ydotoold: failed to create socket: %s\n", strerror(errno));
		abort();
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path_socket, sizeof(addr.sun_path)-1);

	if (bind(fd_listener, (struct sockaddr *)&addr, sizeof(addr))) {
		fprintf(stderr, "ydotoold: failed to bind to socket [%s]: %s\n", path_socket, strerror(errno));
		abort();
	}

	if (listen(fd_listener, 16)) {
		fprintf(stderr, "ydotoold: failed to listen on socket [%s]: %s\n", path_socket, strerror(errno));
		abort();
	}

	chmod(path_socket, 0600);
	printf("ydotoold: listening on socket %s\n", path_socket);

    int fd_client = 0;
	while ((fd_client = accept(fd_listener, NULL, NULL))) {
		printf("ydotoold: accepted client\n");

        pthread_t thd;
        if (pthread_create(&thd, NULL, client_handler, (void *)&fd_client)) {
            fprintf(stderr, "ydotoold: Error creating thread!\n");
            abort();
        }

        if (pthread_detach(thd)) {
            fprintf(stderr, "ydotoold: Error detaching thread!\n");
            abort();
        }
	}

    uinput_destroy();
    return 0;
}