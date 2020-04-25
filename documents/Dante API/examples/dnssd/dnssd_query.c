#include "dnssd_examples.h"
#if defined(WIN32)
#include "strcasecmp.h"
#endif // defined(WIN32)


typedef struct runtime
{
	int running;
	int status;

	const char * name;
	const char * service_type;
	const char * record_type;
	const char * domain;

	aud_bool_t long_lived_query;

	DNSServiceRef sdRef;
	aud_socket_t fd;

	fd_set rfd;
	int fd_max;

	uint16_t rrtype;
	uint16_t rrclass;
	char fullname [kDNSServiceMaxDomainName];
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
		}
		else
		{
			printf (
				"Removed %s record '%s'\n"
				, rrtype_str, fullname
			);
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

	unsigned int flags = 0;
	if (r->long_lived_query)
		flags |= kDNSServiceFlagsLongLivedQuery;

	errcode =
		DNSServiceQueryRecord (
			& r->sdRef,
			flags,	// no flags
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


// Argument and option handling

typedef struct args
{
	unsigned argc;
	char ** argv;
} args_t;

AUD_INLINE char *
use_arg(args_t * args)
{
	char * result = args->argv[0];

	args->argc--;
	args->argv++;

	return result;
}

AUD_INLINE aud_bool_t
is_opt(const args_t * args)
{
	return args->argc && args->argv[0][0] == '-';
}

AUD_INLINE aud_bool_t
is_arg(const args_t * args)
{
	return args->argc && args->argv[0][0] != '-';
}

static int
parse_opts(args_t * args, runtime_t * r)
{
	while(is_opt(args))
	{
		char * opt = use_arg(args)+1;

		if (strcmp(opt, "-") == 0)
			break;

		if (strcmp(opt, "?") == 0 ||
			strcmp(opt, "h") == 0 || strcmp(opt, "-help") == 0
		)
		{
			return 1;
		}

		if (strcmp(opt, "-domain") == 0)
		{
			if (args->argc)
			{
				r->domain = use_arg(args);
				continue;
			}
			else
				goto l__missing_arg;
		}

		if (strcmp(opt, "l") == 0 || strcmp(opt, "-llq") == 0)
		{
			if (args->argc)
			{
				r->long_lived_query = AUD_TRUE;
				r->domain = use_arg(args);
				continue;
			}
			else
				goto l__missing_arg;
		}

		fprintf(stderr, "Uknown option: -%s\n", opt);
		return 2;

	l__missing_arg:
		fprintf(stderr, "Missing argument to option -%s\n", opt);
		return 2;
	}

	return 0;
}


static int
parse_args(args_t * args, runtime_t * r)
{
	if (! is_arg(args))
	{
		fprintf (stderr, "Missing args\n");
		return 2;
	}

	r->name = use_arg(args);

	if (is_arg(args))
	{
		r->service_type = use_arg(args);

		if (is_arg(args))
		{
			r->record_type = use_arg(args);

			r->rrtype = rrtype_from_str(r->record_type);
			if (! r->rrtype)
			{
				fprintf (stderr, "Unknown RR type '%s'\n", r->record_type);
				return 2;
			}

			if (is_arg(args))
			{
				fprintf (stderr, "Too many arguments\n");
				return 2;
			}
		}
	}

	return 0;
}

static int
do_args (int argc, char ** argv, runtime_t * r)
{
	int result;

	args_t args;
	args.argc = argc -1;
	args.argv = argv +1;

#ifdef WIN32
	g_progname = argv[0];
#else
	g_progname = basename(argv[0]);
#endif

	result = parse_opts(&args, r);
	if (result != 0)
		goto l__usage;

	result = parse_args(&args, r);
	if (result != 0)
		goto l__usage;

	result = parse_opts(&args, r);
	if (result != 0)
		goto l__usage;

	if (DNSServiceConstructFullName (r->fullname, r->name, r->service_type, r->domain))
	{
		fprintf (stderr, "Invalid name or service\n");
		result = 2;
		goto l__usage;
	}

	return 0;

l__usage:
	fprintf (stderr,
		"Usage: %s name [service [RR_TYPE] ] [--domain|--llq domain]\n"
		, g_progname
	);
	return result;
}


// Main

int
main (int argc, char ** argv)
{
	int result;

	runtime_t r = {0};

	r.rrtype = kDNSServiceType_TXT;
	r.rrclass = kDNSServiceClass_IN;
	r.service_type = "_example._udp";
	r.domain = "local";

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

	return result;
}
