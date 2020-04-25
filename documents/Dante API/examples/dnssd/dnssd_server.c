#include "dnssd_examples.h"

/*
	This program advertises a service over DNS-SD and then echoes any data
	sent to it tp both stdout and the sender.
 */

enum
{
	PACKET_BUF = 1500,
	INPUT_BUF = 1024
};

typedef char packet_buf_t [PACKET_BUF + 1];
typedef char input_buf_t [INPUT_BUF];

typedef struct runtime
{
	// Runtime
	int running;
	int status;

	fd_set rfd;
	int fd_max;

	// Service discovery
	DNSServiceRef sdRef;
	aud_socket_t fd_sd;

	const char * service_type;
	const char * name;
	const char * domain;

	unsigned int port;

	// Packet socket
	aud_socket_t fd_pkt;

} runtime_t;

static char * g_progname;

// Prototypes

static aud_bool_t
handle_packet (runtime_t * r);


// Functions

static int
run_loop (runtime_t * r)
{
	r->running = AUD_TRUE;
	r->status = 0;

	while (r->running)
	{
		fd_set r_curr;
		int sel_result;

		AUD_FD_COPY (& r->rfd, & r_curr);

		sel_result = select (r->fd_max, & r_curr, NULL, NULL, NULL);
		if (sel_result > 0)
		{
			aud_socket_t fd;

			fd = r->fd_sd;
			if (fd != AUD_SOCKET_INVALID && FD_ISSET (fd, & r_curr))
			{
				DNSServiceProcessResult (r->sdRef);
			}

			fd = r->fd_pkt;
			if (fd != AUD_SOCKET_INVALID && FD_ISSET (fd, & r_curr))
			{
				handle_packet (r);
			}
		}
		else if(sel_result < 0)
		{
			int err = _get_last_error();
			if (err != EINTR)
			{
				fprintf (stderr,
					"Select error: %s (%d)\n"
					, strerror (err), err
				);
				r->running = AUD_FALSE;
				r->status = err;
			}
		}
	}

	return r->status;
}


static void
add_fd (runtime_t * r, int fd)
{
	FD_SET (fd, & r->rfd);
	if (fd + 1 > r->fd_max)
	{
		r->fd_max = (int)fd+1;
	}
}

static void
remove_fd (runtime_t * r, int fd)
{
	FD_CLR (fd, & r->rfd);
}

static void
close_sd (runtime_t * r)
{
	if (r->fd_sd != AUD_SOCKET_INVALID)
	{
		remove_fd (r, (int)r->fd_sd);
		DNSServiceRefDeallocate (r->sdRef);
		r->fd_sd = AUD_SOCKET_INVALID;
	}
}

static void
close_pkt (runtime_t * r)
{
	if (r->fd_pkt != AUD_SOCKET_INVALID)
	{
		remove_fd (r, (int)r->fd_pkt);
		r->fd_pkt = AUD_SOCKET_INVALID;
	}
}


static void DNSSD_API
register_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	DNSServiceErrorType errCode,
	const char * name,
	const char * regtype,
	const char * domain,
	void * context
)
{
	runtime_t * r = context;
	if (errCode == kDNSServiceErr_NoError)
	{
		printf (
			"Registered %s.%s%s\n"
			, name, regtype, domain
		);
	}
	else
	{
		fprintf (stderr,
			"Registeration failed: %d\n"
			, errCode
		);
		r->running = AUD_FALSE;
		r->status = AUD_FALSE;
	}
}


static aud_bool_t
register_service (runtime_t * r)
{
	DNSServiceErrorType errcode;

	errcode =
		DNSServiceRegister (
			& r->sdRef,
			0,	// no flags
			0,	// all interfaces
			r->name,	// hostname
			r->service_type,
			r->domain,
			NULL,	// default host
			htons (r->port),
			0,	// no TXT record
			NULL,	// no TXT record
			register_reply,	// callback
			r	// context
		);
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to register %s on port %u: %d\n"
			, r->service_type, r->port, errcode
		);
		return AUD_FALSE;
	}

	r->fd_sd = DNSServiceRefSockFD (r->sdRef);
	add_fd (r, (int)r->fd_sd);

	return AUD_TRUE;
}


static aud_bool_t
create_socket (runtime_t * r)
{
#if defined(WIN32)
	SOCKET result;
#else
	int result;
#endif // defined(WIN32)
	struct sockaddr_in sa_in = {0};
	struct sockaddr * sa = (struct sockaddr *) & sa_in;

	result = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (result < 0)
	{
		int err = _get_last_error();
		fprintf (stderr,
			"Failed to create socket: %s (%d)\n"
			, strerror (err), err
		);
		goto l__finish;
	}

	r->fd_pkt = result;

	// sa_len nastiness
	#if defined(BSD)
		// This covers BSD platforms.  Possibly need SA_LEN on others too.
	sa_in.sin_len = 16;
	#endif
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons (r->port);

	result = bind (r->fd_pkt, sa, 16);
	if (result < 0)
	{
		int err = _get_last_error();
		fprintf (stderr,
			"Failed to bind socket to port %u: %s (%d)\n"
			, r->port, strerror (err), err
		);
		goto l__finish;
	}

	add_fd (r, (int)r->fd_pkt);

	return AUD_TRUE;

l__finish:
	close_pkt (r);

	return AUD_FALSE;
}


static aud_bool_t
handle_packet (runtime_t * r)
{
	packet_buf_t p;
#ifdef WIN32
	int len;
#else
	ssize_t len;
#endif
	struct sockaddr_storage sa_s;
	struct sockaddr * sa = (struct sockaddr *) & sa_s;
	socklen_t sa_len = sizeof (sa_s);

	len = recvfrom (r->fd_pkt, p, PACKET_BUF, 0, sa, & sa_len);
	if (len < 0)
	{
		int err = _get_last_error();
		fprintf (stderr,
			"Read fail: %s (%d)\n"
			, strerror (err), err
		);
		return AUD_FALSE;
	}

	p [len] = 0;
		// Just in case

	fputs (p, stdout);

	{
		size_t bytes = len;

		len = sendto (r->fd_pkt, p, (int)bytes, 0, sa, sa_len);
		if (len < 0)
		{
			int err = _get_last_error();
			fprintf (stderr,
				"Send fail: %s (%d)\n"
				, strerror (err), err
			);
		}
		else if (len < (int) bytes)
		{
			fprintf (stderr,
				"Send error: sent %d of %u bytes"
				, (int) len, (unsigned int) bytes
			);
		}
		else
		{
			return AUD_TRUE;
		}
	}

	return AUD_FALSE;
}


static int
do_args (int argc, char ** argv, runtime_t * r)
{
	int i;

#ifdef WIN32
	g_progname = argv [0];
#else
	g_progname = basename (argv [0]);
#endif

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-s") && (i < argc-1))
		{
			i++;
			r->service_type = argv[i];
		}
		else if (!strcmp(argv[i], "-d") && (i < argc-1))
		{
			i++;
			r->domain = argv[i];
		}
		else if (!strcmp(argv[i], "-p") && (i < argc-1))
		{
			i++;
			r->port = atoi (argv[i]);
		}
		else if (!strcmp(argv[i], "-?"))
		{
			goto l__usage;
		}
		else if (strlen(argv[i]) > 0 && (argv[i][0] == '-'))
		{
			fprintf (stderr,
				"%s: Unknown option: %s\n"
				, g_progname, argv[i]
			);
			goto l__usage;
		}
		else if (i != argc-1)
		{
			fprintf (stderr,
				"%s: Too many arguments\n"
				, g_progname
			);
			goto l__usage;
		}
		else
		{
			r->name = argv[i];
		}
	}
	return 0;

l__usage:
	fprintf (stderr,
		"Usage: %s [-s service] [-d domain] [-p port] [name]\n"
		, g_progname
	);
	return 2;
}

int
main (int argc, char ** argv)
{
	int result;

	runtime_t r = {0};
#ifdef WIN32
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	result = WSAStartup(version, &data);
	if (result)
	{
		fprintf(stderr, "System error\n");
		return result;
	}
#endif

	r.service_type = "_echo._udp";
	r.port = 6789;
	// default domain is 'NULL'

	r.fd_sd = AUD_SOCKET_INVALID;
	r.fd_pkt = AUD_SOCKET_INVALID;

	result = do_args (argc, argv, & r);
	if (result)
	{
		return result;
	}

	if (create_socket (& r) && register_service (& r))
	{
		result = run_loop (& r);
	}

	close_sd (& r);
	close_pkt (& r);

	return result;
}
