#include "dnssd_examples.h"

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

/*
	This program resolves a service advertised over DNS-SD and then sends
	anything entered via stdin to it.

	First, we create a DNS-SD resolve query.
	When that returns successfully, we do a GetAddrInfo on the result.
	Finally, we open a socket and can send text.
 */

enum
{
	HOSTNAME_LEN = 255,

	PACKET_BUF = 1500,
	INPUT_BUF = 1024
};

typedef char packet_buf_t [PACKET_BUF + 1];
typedef char input_buf_t [INPUT_BUF];

enum
{
	SD_RESOLVE = 0,
	SD_ADDR, // get addr info

	SD_MAX
};

typedef struct runtime
{
	// Runtime
	int running;
	int status;

	fd_set rfd;
	int fd_max;

	// Service discovery
	DNSServiceRef sdRef [SD_MAX];
	aud_socket_t fd_sd [SD_MAX];

	const char * service_type;
	const char * name;
	const char * domain;

	unsigned int port;
	char target [HOSTNAME_LEN + 1];

	// Packet socket
	union
	{
		struct sockaddr sa;
		struct sockaddr_in in;
	} sa;
	unsigned int sa_len;
	aud_socket_t fd_pkt;

} runtime_t;

static char * g_progname;

// Prototypes

static aud_bool_t
handle_packet (runtime_t * r);

static aud_bool_t
handle_input (runtime_t * r);


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
		if (sel_result >= 0)
		{
			aud_socket_t fd;
			unsigned int i;
			for (i = 0; i < SD_MAX; i++)
			{
				fd = r->fd_sd[i];
				if (fd != AUD_SOCKET_INVALID && FD_ISSET (fd, & r_curr))
				{
					DNSServiceProcessResult (r->sdRef[i]);
				}
			}

			fd = r->fd_pkt;
			if (fd != AUD_SOCKET_INVALID && FD_ISSET (fd, & r_curr))
			{
				handle_packet (r);
			}

			if (FD_ISSET (0, & r_curr))
			{
				handle_input (r);
			}
		}
		else
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
		r->fd_max = fd+1;
	}
}

static void
remove_fd (runtime_t * r, aud_socket_t fd)
{
	FD_CLR (fd, & r->rfd);
}


static void
add_sd (runtime_t * r, unsigned int i)
{
	int fd = DNSServiceRefSockFD(r->sdRef[i]);
	if (fd != AUD_SOCKET_INVALID)
	{
		r->fd_sd[i] = fd;
		add_fd(r, fd);
	}
}

static void
close_sd (runtime_t * r, unsigned int i)
{
	if (r->fd_sd[i] != AUD_SOCKET_INVALID)
	{
		remove_fd (r, r->fd_sd[i]);
		DNSServiceRefDeallocate (r->sdRef[i]);
		r->fd_sd[i] = AUD_SOCKET_INVALID;
	}
}

static void
close_pkt (runtime_t * r)
{
	if (r->fd_pkt != AUD_SOCKET_INVALID)
	{
		remove_fd (r, r->fd_pkt);
		r->fd_pkt = AUD_SOCKET_INVALID;
	}
}


static void DNSSD_API
hostname_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t intf,
	DNSServiceErrorType errCode,
	const char * fullname,
	const struct sockaddr * addr,
	uint32_t ttl,
	void * context
)
{
	runtime_t * r = context;
	if (errCode == kDNSServiceErr_NoError)
	{
#if defined(WIN32)
		SOCKET result;
#else
		int result;
#endif // defined(WIN32)
		unsigned family = addr->sa_family;
			// Family is host byte order, but might be uint8_t or uint16_t depending on platform

		printf (
			"Resolved '%s' to "
			, fullname
		);

		if (family == AF_INET)
		{
			// cast through (void *) to stop compiler warnings about alignment
			// which can't hurt us in this particular case.
			const struct sockaddr_in * sa_in = (const void *) addr;

			printf (
				"%s",
				inet_ntoa (sa_in->sin_addr)
			);

			if (r->fd_pkt != AUD_SOCKET_INVALID)
			{
				puts (": discarding redundant address");
				return;
			}

			printf (
				", connecting to port %u\n"
				, r->port
			);

			r->sa_len = 16;
			r->sa.in = * sa_in;

			r->sa.in.sin_port = htons (r->port);

			result = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (result == AUD_SOCKET_INVALID)
			{
				int err = _get_last_error();
				fprintf (stderr,
					"Failed to create socket: %s (%d)\n"
					, strerror (err), err
				);
				goto l__finish;
			}

			r->fd_pkt = result;
			add_fd (r, (int)result);
				// Handle incoming replies
			add_fd (r, 0);
				// Begin reading stdin for input

			// Release DNS-SD query
			close_sd(r, SD_ADDR);
		}
		else
		{
			printf (": unknown family %u\n", family);
		}

		return;
	}
	else
	{
		fprintf (stderr,
			"Resolve failed: %d\n"
			, errCode
		);
	}

l__finish:
	r->running = AUD_FALSE;
	r->status = 1;
}


static aud_bool_t
resolve_hostname_using_get_addr_info (runtime_t * r)
{
	DNSServiceErrorType errcode;

	errcode =
		DNSServiceGetAddrInfo (
			r->sdRef + SD_ADDR,
			0,	// flags
			0,	// all interfaces
			kDNSServiceProtocol_IPv4,
			r->target,
			hostname_reply,	// callback
			r	// context
		);
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to create query for '%s': %d\n"
			, r->target, errcode
		);
		return AUD_FALSE;
	}

	add_sd (r, SD_ADDR);

	return AUD_TRUE;
}


static void DNSSD_API
resolve_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t intf,
	DNSServiceErrorType errCode,
	const char * fullname,
	const char * target,
	uint16_t in_port,
	uint16_t txt_len,
	const uint8_t * txt,
	void * context
)
{
	runtime_t * r = context;
	if (errCode == kDNSServiceErr_NoError)
	{
		unsigned int port = ntohs (in_port);

		printf (
			"Resolved '%s' at %s, port %u\n"
			, fullname, target, port
		);
		strncpy (r->target, target, HOSTNAME_LEN);
		r->target [HOSTNAME_LEN] = 0;
		r->port = port;

		close_sd(r, SD_RESOLVE);

		if (resolve_hostname_using_get_addr_info (r))
		{
			return;
		}
	}
	else
	{
		fprintf (stderr,
			"Resolve failed: %d\n"
			, errCode
		);
	}

	r->running = AUD_FALSE;
	r->status = 1;
}

// There is a slight discrepancy between
// Windows & Posix for mDNS callbacks.
// Disable warning on windows to avoid unwanted warnings
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4028)
#endif
static aud_bool_t
start_resolve (runtime_t * r)
{
	DNSServiceErrorType errcode;

	errcode =
		DNSServiceResolve (
			r->sdRef + SD_RESOLVE,
			0,	// flags
			0,	// all interfaces
			r->name,
			r->service_type,
			r->domain,
			resolve_reply,	// callback
			r	// context
		);
#ifdef WIN32
#pragma warning(pop)
#endif
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to create query for '%s' in %s: %d\n"
			, r->name, r->service_type, errcode
		);
		return AUD_FALSE;
	}

	add_sd (r, SD_RESOLVE);

	return AUD_TRUE;
}


static aud_bool_t
handle_packet (runtime_t * r)
{
	struct sockaddr_storage sa_s;
	struct sockaddr * sa = (struct sockaddr *) & sa_s;
	socklen_t sa_len = sizeof (sa_s);

	packet_buf_t p;
#ifdef WIN32
	int len;
#else
	ssize_t len;
#endif

	len = recvfrom (r->fd_pkt, p, PACKET_BUF, 0, sa, &sa_len);
	if (len < 0)
	{
		int err = _get_last_error();
		fprintf (stderr,
			"Recv fail: %s (%d)\n"
			, strerror (err), err
		);
		return AUD_FALSE;
	}

	p [len] = 0;
		// Just in case

	fputs (p, stdout);

	return AUD_TRUE;
}


static aud_bool_t
handle_input (runtime_t * r)
{
	input_buf_t buf;

	if (fgets (buf, INPUT_BUF, stdin))
	{
#ifdef WIN32
		int len;
#else
		ssize_t len;
#endif
		size_t bytes = strlen (buf) + 1;

		len = sendto (r->fd_pkt, buf, (int)bytes, 0, & r->sa.sa, r->sa_len);
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
	else if (feof (stdin))
	{
		r->running = AUD_FALSE;
	}
	else
	{
		fprintf (stderr,
			"Failed to read input\n"
		);
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

	for(i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-s") && i < argc-1)
		{
			i++;
			r->service_type = argv[i];
		}
		else if (!strcmp(argv[i], "-d") && i < argc-1)
		{
			i++;
			r->domain = argv[i];
		}
		else if (!strcmp(argv[i], "-?"))
		{
			goto l__usage;
		}
		else if (strlen(argv[i]) > 0 && argv[i][0] == '-')
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
				"%s: Too many arguments"
				, g_progname
			);
			goto l__usage;
		}
		else
		{
			r->name = argv[i];
		}
	}

	if (r->name)
	{
		return 0;
	}

	fprintf (stderr,
		"%s: Missing args\n"
		, g_progname
	);
l__usage:
	fprintf (stderr,
		"Usage: %s [-s service] [-d domain] name\n"
		, g_progname
	);
	return 2;
}

int
main (int argc, char ** argv)
{
	int result;
	unsigned int i;

	runtime_t r = {0};

	r.service_type = "_echo._udp";
	r.domain = "local";

	for (i = 0; i < SD_MAX; i++)
	{
		r.fd_sd[i] = AUD_SOCKET_INVALID;
	}
	r.fd_pkt = AUD_SOCKET_INVALID;

	result = do_args (argc, argv, & r);
	if (result)
	{
		return result;
	}

	if (start_resolve (& r))
	{
		result = run_loop (& r);
	}

	remove_fd (& r, 0);
	close_pkt (& r);

	return result;
}

#ifdef WIN32
#pragma warning(pop)
#endif
