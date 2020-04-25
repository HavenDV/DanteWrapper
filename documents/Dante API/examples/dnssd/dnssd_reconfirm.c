#include "dnssd_examples.h"
#if defined(WIN32)
#include "strcasecmp.h"
#endif // defined(WIN32)
#include <ctype.h>


typedef struct runtime
{
	int running;
	int status;

	const char * name;
	const char * service_type;
	const char * record_type;

	DNSServiceRef sdRef;
	aud_socket_t fd;

	fd_set rfd;
	int fd_max;

	unsigned int reconfirm_s;

	uint16_t rrtype;
	uint16_t rrclass;
	char fullname [kDNSServiceMaxDomainName];
	const char * rrtype_str;

	uint16_t rdlen;
	void * rdata;
	uint16_t rintf;
} runtime_t;

static char * g_progname;

static const struct record_type_mapping
{
	const char * name;
	uint16_t type;
} k_record_types [] =
{
	{ "TXT", kDNSServiceType_TXT },
	{ "SRV", kDNSServiceType_SRV },
	{ "PTR", kDNSServiceType_PTR },
	{ "CNAME", kDNSServiceType_CNAME },
	{ "A", kDNSServiceType_A },
	{ "AAAA", kDNSServiceType_AAAA },
	{ NULL, 0 }
};


// Prototypes (where needed)

static aud_bool_t
start_query (runtime_t * r);

static aud_bool_t
start_reconfirm (runtime_t * r);


static uint16_t
rrtype_from_str (const char * str)
{
	unsigned int i;
	for (i = 0; k_record_types[i].name; i++)
	{
		if (strcasecmp(k_record_types[i].name, str) == 0)
		{
			return k_record_types[i].type;
		}
	}

	return 0;
};

static const char *
str_from_rrtype (uint16_t type)
{
	unsigned int i;
	for (i = 0; k_record_types[i].name; i++)
	{
		if (k_record_types[i].type == type)
		{
			return k_record_types[i].name;
		}
	}

	return 0;
};


static int
run_loop (runtime_t * r)
{
	long int next_time = 0;
	aud_utime_t timeout, *tp = NULL;

	r->running = AUD_TRUE;
	r->status = 0;

	if (r->reconfirm_s)
	{
		aud_utime_get(&timeout);
		next_time = timeout.tv_sec + r->reconfirm_s;
		tp = & timeout;

		// set next timeout
		timeout.tv_sec = r->reconfirm_s;
		timeout.tv_usec = 0;
	}

	while (r->running)
	{
		fd_set r_curr;
		int sel_result;

		AUD_FD_COPY (& r->rfd, & r_curr);

		sel_result = select (r->fd_max, & r_curr, NULL, NULL, tp);
		if (sel_result > 0)
		{
			aud_socket_t fd = r->fd;
			if (FD_ISSET (fd, & r_curr))
			{
				DNSServiceProcessResult (r->sdRef);
			}
		}
		else if (sel_result < 0)
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
				break;
			}
		}

		if (next_time)
		{
			aud_utime_get(&timeout);
			if (timeout.tv_sec >= next_time)
			{
				start_reconfirm(r);
				next_time = timeout.tv_sec + r->reconfirm_s;

				// set next timeout
				timeout.tv_sec = r->reconfirm_s;
				timeout.tv_usec = 0;
			}
			else
			{
				timeout.tv_sec = next_time - timeout.tv_sec;
					// how long still to wait, rounded to seconds?
				timeout.tv_usec = 0;
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


static void
set_rdata(runtime_t * r, uint16_t rdlen, const void * rdata)
{
	if (rdata)
	{
		if (r->rdata)
		{
			if (rdlen == r->rdlen && memcmp(r->rdata,rdata,rdlen) == 0)
			{
				puts("Records match");
				return;
			}

			free(r->rdata);
			r->rdata = NULL;
			r->rdlen = 0;
		}

		r->rdata = malloc(rdlen);
		if (r->rdata)
		{
			memcpy(r->rdata, rdata, rdlen);
			r->rdlen = rdlen;
		}
		else
		{
			fputs("Memory failure\n", stderr);
			exit(_get_last_error());
		}
	}
	else if (r->rdata)
	{
		free(r->rdata);
		r->rdata = NULL;
		r->rdlen = 0;
	}
}

static void DNSSD_API
query_reply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t intf,
	DNSServiceErrorType errCode,
	const char * fullname,
	uint16_t rrtype,
	uint16_t rrclass,
	uint16_t rdlen,
	const void * rdata,
	uint32_t ttl,
	void * context
)
{
	runtime_t * r = context;
	if (errCode == kDNSServiceErr_NoError)
	{
		const char * rrtype_str = str_from_rrtype(rrtype);
		if (! rrtype_str)
		{
			rrtype_str = "<Unknown type>";
		}
		if (flags & kDNSServiceFlagsAdd)
		{
			printf (
				"Resolved %s record '%s': %u bytes, TTL %us\n"
				, rrtype_str, fullname, rdlen, ttl
			);

			set_rdata (r, rdlen, rdata);
			r->rintf = intf;
			r->rrtype_str = rrtype_str;
		}
		else
		{
			printf (
				"Removed %s record '%s'\n"
				, rrtype_str, fullname
			);

			set_rdata (r, 0, NULL);
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
start_query (runtime_t * r)
{
	DNSServiceErrorType errcode;

	errcode =
		DNSServiceQueryRecord (
			& r->sdRef,
			0,	// no flags
			0,	// all interfaces
			r->fullname,
			r->rrtype,
			r->rrclass,
			query_reply,	// callback
			r	// context
		);
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to query for %s: %d\n"
			, r->service_type, errcode
		);
		return AUD_FALSE;
	}

	add_fd (r);

	return AUD_TRUE;
}


static aud_bool_t
start_reconfirm (runtime_t * r)
{
	DNSServiceErrorType errcode;

	if (! r->rdata)
	{
		return AUD_FALSE;
			// nothing to confirm
	}

	errcode =
		DNSServiceReconfirmRecord (
			0,	// no flags
			r->rintf,
			r->fullname,
			r->rrtype,
			r->rrclass,
			r->rdlen,
			r->rdata
		);
	if (errcode != kDNSServiceErr_NoError)
	{
		fprintf (stderr,
			"Failed to initiate reconfirm for %s %s: %d\n"
			, r->fullname, r->rrtype_str, errcode
		);
		return AUD_FALSE;
	}
	else
	{
		printf(
			"Confirming %s %s is still current\n"
			, r->fullname, r->rrtype_str
		);
	}

	add_fd (r);

	return AUD_TRUE;
}


static int
do_args (int argc, char ** argv, runtime_t * r)
{
	int args_left = argc -1;
	char ** args = argv +1;

#ifdef WIN32
	g_progname = argv[0];
#else
	g_progname = basename (argv [0]);
#endif

	// options
	while (args_left && (args[0][0] == '-'))
	{
		const char * optstr = args[0]+1;
		args_left --;
		args ++;

		if (! optstr[0])
		{
			break;
				// done args
		}

		if (optstr[0] == 'r')
		{
			int reconfirm_s;
			if (optstr[1])
			{
				reconfirm_s = atoi(optstr+1);
			}
			else if (args_left && isdigit(args[0][0]))
			{
				reconfirm_s = atoi(args[0]);
				args_left --;
				args ++;
			}
			else
			{
				fputs("Missing or malformed value for reconfirm period\n", stderr);
				goto l__usage;
			}

			if (reconfirm_s >= 0)
			{
				r->reconfirm_s = reconfirm_s;
				continue;
			}
			else
			{
				fputs("Reconfirm delay must be >= 0\n", stderr);
			}
		}

		fprintf (stderr, "Unknown option: %s\n", optstr);
		goto l__usage;
	}

	if (! args_left)
	{
		fputs("Missing args\n", stderr);
		goto l__usage;
	}

	r->name = args[0];

	if (args_left >= 2)
	{
		r->service_type = args[1];

		if (args_left >= 3)
		{
			r->record_type = args[2];

			if (args_left > 3)
			{
				fprintf (stderr, "Too many arguments\n");
				goto l__usage;
			}

			r->rrtype = rrtype_from_str(r->record_type);
			if (! r->rrtype)
			{
				fprintf (stderr, "Unknown RR type '%s'\n", r->record_type);
				goto l__usage;
			}
		}
	}

	if (DNSServiceConstructFullName (r->fullname, r->name, r->service_type, "local"))
	{
		fprintf (stderr, "Invalid name or service\n");
		return 2;
	}

	return 0;

l__usage:
	fprintf (stderr,
		"Usage: %s [-r period_s] name [service [RR_TYPE] ]\n"
		, g_progname
	);
	return 2;
}

int
main (int argc, char ** argv)
{
	int result;

	runtime_t r = {0};

	r.rrtype = kDNSServiceType_TXT;
	r.rrclass = kDNSServiceClass_IN;
	r.service_type = "_example._udp";

	r.fd = AUD_SOCKET_INVALID;

	result = do_args (argc, argv, & r);
	if (result)
	{
		return result;
	}

	if (start_query (& r))
	{
		result = run_loop (& r);
	}

	if (r.fd != AUD_SOCKET_INVALID)
	{
		remove_fd (& r);
		DNSServiceRefDeallocate (r.sdRef);
	}

	set_rdata (&r, 0, NULL);

	return result;
}
