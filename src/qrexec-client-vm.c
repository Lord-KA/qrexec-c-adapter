/*
 * The Qubes OS Project, http://www.qubes-os.org
 *
 * Copyright (C) 2010  Rafal Wojtczuk  <rafal@invisiblethingslab.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

int IN_FD;
int OUT_FD;

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include "libqrexec-utils.h"
#include "qrexec.h"
#include "qrexec-agent.h"

const bool qrexec_is_fork_server = false;

void handle_vchan_error(const char *op)
{
    LOG(ERROR, "Error while vchan %s, exiting", op);
    exit(1);
}

void do_exec(const char *cmd __attribute__((unused)), char const* user __attribute__((__unused__))) {
    LOG(ERROR, "BUG: do_exec function shouldn't be called!");
    abort();
}

static int connect_unix_socket(const char *path)
{
    int s;
    size_t len;
    struct sockaddr_un remote;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        PERROR("socket");
        return -1;
    }

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, path,
            sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *) &remote, (socklen_t)len) == -1) {
        PERROR("connect");
        exit(1);
    }
    return s;
}

/* Target specification with keyword have changed from $... to @... . Convert
 * the argument appropriately, to avoid breaking user tools.
 */
static void convert_target_name_keyword(char *target)
{
    size_t len = strlen(target);

    for (size_t i = 0; i < len; i++)
        if (target[i] == '$')
            target[i] = '@';
}

enum {
    opt_no_filter_stdout = 't'+128,
    opt_no_filter_stderr = 'T'+128,
};

int exec_connector(const char *target_vmname, const char *service_name)
{
    int trigger_fd;
    struct msg_header hdr;
    struct trigger_service_params3 params;
    struct exec_params exec_params;
    size_t service_name_len;
    ssize_t ret;
    pid_t child_pid = 0;
    int inpipe[2], outpipe[2];
    int buffer_size = 0;
    const char *agent_trigger_path = QREXEC_AGENT_TRIGGER_PATH;

    setup_logging("qrexec-client-vm");

    // TODO: this should be in process_io
    signal(SIGPIPE, SIG_IGN);

    replace_chars_stdout = 0;
    replace_chars_stderr = 0;

    service_name_len = strlen(service_name) + 1;

    trigger_fd = connect_unix_socket(agent_trigger_path);

    hdr.type = MSG_TRIGGER_SERVICE3;
    hdr.len = sizeof(params) + service_name_len;

    memset(&params, 0, sizeof(params));

    strncpy(params.target_domain, target_vmname,
            sizeof(params.target_domain) - 1);
    convert_target_name_keyword(params.target_domain);
    fprintf(stderr, "target_vm name: %s\n", params.target_domain);

    snprintf(params.request_id.ident,
            sizeof(params.request_id.ident), "SOCKET");

    if (!write_all(trigger_fd, &hdr, sizeof(hdr))) {
        PERROR("write(hdr) to agent");
        exit(1);
    }
    if (!write_all(trigger_fd, &params, sizeof(params))) {
        PERROR("write(params) to agent");
        exit(1);
    }
    if (!write_all(trigger_fd, service_name, service_name_len)) {
        PERROR("write(command) to agent");
        exit(1);
    }
    ret = read(trigger_fd, &exec_params, sizeof(exec_params));
    if (ret == 0) {
        fprintf(stderr, "Request refused\n");
        exit(126);
    }
    if (ret < 0 || ret != sizeof(exec_params)) {
        PERROR("read");
        exit(1);
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, inpipe) ||
            socketpair(AF_UNIX, SOCK_STREAM, 0, outpipe)) {
        PERROR("socketpair");
        exit(1);
    }
    prepare_child_env();

    IN_FD  = inpipe[0];
    OUT_FD = outpipe[1];

    switch (child_pid = fork()) {
        case -1:
            PERROR("fork");
            exit(-1);
        case 0:
            close(trigger_fd);

    	    ret = handle_data_client(MSG_SERVICE_CONNECT,
            	exec_params.connect_domain, exec_params.connect_port,
            	inpipe[1], outpipe[0], -1, buffer_size, child_pid);

	    exit(0);
	    return 0;
    }
    close(inpipe[1]);
    close(outpipe[0]);
    close(trigger_fd);
    return 0;
}

#ifdef __cplusplus
}
#endif
