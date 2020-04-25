#include "dnssd_examples.h"

typedef struct runtime
{
	int running;
	int status;

	unsigned int port;
	const char * service_type;
	const char * domain;

	DNSServiceRef sdRef;
	aud_socket_t fd;

	const char * name;

	fd_set rfd;
	int fd_max;
} runtime_t;

static char * g_progname;

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
			aud_socket_t fd = r->fd;
			if (FD_ISSET (fd, & r_curr))
			{
				DNSServiceProcessResult (r->sdRef);
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
add_fd (runtime_t * r)
{
	int fd = DNSServiceRefSockFD (r->sdRef);
	r->fd = fd;

	FD_SET (fd, & r->rfd);
	if (fd + 1 > r->fd_max)
	{
		r->fd_max = fd+1;
	}
}

static void
remove_fd (runtime_t * r)
{
	aud_socket_t fd = r->fd;
	FD_CLR (fd, & r->rfd);
	r->fd = AUD_SOCKET_INVALID;
}


static void DNSSD_API
resolve_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t intf,
	DNSServiceErrorType errCode,
	const char * fullname,
	const char * target,
	uint16_t port,
	uint16_t txt_len,
	const uint8_t * txt,
	void * context
)
{
	runtime_t * r = context;
	if (errCode == kDNSServiceErr_NoError)
	{
		printf (
			"Resolved '%s' at %s, port %u\n"
			, fullname, target, port
		);
		if (! (flags & kDNSServiceFlagsMoreComing))
		{
			r->running = AUD_FALSE;
		}
	}
	else
	{
		fprintf (stderr,
			"Resolve failed: %d\n"
			, errCode
		);
		r->running = AUD_FALSE;
		r->status = 1;
	}
}


static aud_bool_t
start_resolve (runtime_t * r)
{
	DNSServiceErrorType errcode;

// There is a slight discrepancy between
// Windows & Posix for mDNS callbacks.
// Disable warning on windows to avoid unwanted warnings
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4028)
#endif
	errcode =
		DNSServiceResolve (
			& r->sdRef,
			0,	// no flags
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
			"Failed to start resolve for %s: %d\n"
			, r->service_type, errcode
		);
		return AUD_FALSE;
	}

	add_fd (r);

	return AUD_TRUE;
}


static int
do_args (int argc, char ** argv, runtime_t * r)
{
#ifdef WIN32
	g_progname = argv[0];
#else
	g_progname = basename (argv [0]);
#endif

	do
	{
		if (argc == 1)
		{
			fprintf (stderr, "Missing args\n");
			break;
		}

		if (argv[1][0] == '-')
		{
			fprintf (stderr, "Unknown option: %s\n", argv [1]);
			break;
		}

		if (argc > 2)
		{
			r->service_type = argv [2];
		}

		if (argc > 3)
		{
			const char * domain = argv[3];
			if (domain[0])
			{
				r->domain = domain;
					// ensure that the domain isn't ""
			}
		}

		if (argc > 4)
		{
			fprintf (stderr, "Too many arguments\n");
			break;
		}

		r->name = argv [1];
		return 0;
	} while (0);

	fprintf (stderr,
		"Usage: %s name [service [domain]]\n"
		, g_progname
	);
	return 2;
}

int
main (int argc, char ** argv)
{
	int result;

	runtime_t r = {0};

	r.port = 6789;
	r.service_type = "_example._udp";
	r.domain = "local";

	r.fd = AUD_SOCKET_INVALID;

	result = do_args (argc, argv, & r);
	if (result)
	{
		return result;
	}

	if (start_resolve (& r))
	{
		result = run_loop (& r);
	}

	if (r.fd != AUD_SOCKET_INVALID)
	{
		remove_fd (& r);
		DNSServiceRefDeallocate (r.sdRef);
	}

	return result;
}
