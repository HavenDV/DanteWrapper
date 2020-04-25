#include "dnssd_examples.h"

typedef struct runtime
{
	int running;
	int status;

	const char * service_type;
	const char * domain;

	DNSServiceRef sdRef;
	aud_socket_t fd;

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
browse_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t intf,
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
		if (kDNSServiceFlagsAdd & flags)
		{
			printf (
				"Discovered '%s' in %s%s\n"
				, name, regtype, domain
			);
		}
		else
		{
			printf (
				"Removed '%s' in %s%s\n"
				, name, regtype, domain
			);
		}
	}
	else
	{
		fprintf (stderr,
			"Browse failed: %d\n"
			, errCode
		);
		r->running = AUD_FALSE;
		r->status = 1;
	}
}


static aud_bool_t
start_browse (runtime_t * r)
{
	DNSServiceErrorType errcode;

	errcode =
		DNSServiceBrowse (
			& r->sdRef,
			0,	// no flags
			0,	// all interfaces
			r->service_type,
			r->domain,
			browse_reply,	// callback
			r	// context
		);
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to browse for %s: %d\n"
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
	g_progname = argv [0];
#else
	g_progname = basename (argv [0]);
#endif

	do
	{
		if (argc > 1)
		{
			r->service_type = argv[1];
		}

		if (argc > 2)
		{
			const char * domain = argv[2];
			if (domain[0])
			{
				r->domain = domain;
					// ensure that the domain isn't ""
			}
		}

		if (argc > 3)
		{
			fprintf (stderr, "Too many arguments\n");
			break;
		}

		return 0;
	} while (0);

	fprintf (stderr,
		"Usage: %s [service [domain]]\n"
		, g_progname
	);
	return 2;
}

int
main (int argc, char ** argv)
{
	int result;

	runtime_t r = {0};

	r.service_type = "_example._udp";
	r.domain = NULL;

	r.fd = AUD_SOCKET_INVALID;

	result = do_args (argc, argv, & r);
	if (result)
	{
		return result;
	}

	if (start_browse (& r))
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
