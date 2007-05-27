/*
 * Copyright (c) 2005 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * <LICENSEHERE>
 */

#include <sys/param.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <netinet/in.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <event.h>
#include <evdns.h>

#include "util.h"
#include "smtp.h"

/* globals */

extern FILE *flog_email;	/* log the email transactions somewhere */
extern const char *log_datadir;	/* log the email transactions somewhere */

int debug;

static void
usage(char *progname)
{
	fprintf(stderr, "%s [-p port] [-l logfile]\n"
	    "\t -p port    - specifies port to bind to\n"
	    "\t -l logfile - logs SMTP transaction to specified file\n",
	    progname);
	exit(1);
}

int
main(int argc, char **argv)
{
	struct event bind_ev;
	char *progname = argv[0];
	char *logfile = NULL;
	int ch;
	u_short port = 2525;

	while ((ch = getopt(argc, argv, "vp:l:d:")) != -1) {
		switch (ch) {
		case 'v':
			debug++;
			break;
		case 'p':
			port = atoi(optarg);
			if (!port)
				err(1, "Bad port number: %s", optarg);
			break;
		case 'd': {
			if (smtp_set_datadir(optarg) == -1)
				errx(1, "Bad directory specification: %s", 
				    log_datadir);
			break;
		}
		case 'l':
			logfile = optarg;
			break;
		default:
			usage(progname);
		}
	}

	if (logfile != NULL) {
		flog_email = fopen(logfile, "a");
		if (flog_email == NULL)
			err(1, "%s: fopen(%s)", __func__, logfile);
		fprintf(stderr, "Logging to %s\n", logfile);
	}

	event_init();

	evdns_init();

	smtp_bind_socket(&bind_ev, port);
	
	event_dispatch();

	exit(0);
}
