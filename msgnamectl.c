#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"
#include "msgnames.h"

char *argv0;

void
usage(void)
{
	fprint(2, "usage: %s [-r|-q] [-n nspid] [-p pid] name\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	enum {Usage, Query, Register} mode = Usage;
	int nspid = 0;
	char *name;
	int retval;

	argv0 = argv[0];

	ARGBEGIN{
	case 'r':
		mode = Register;
		break;
	case 'q':
		mode = Query;
		break;
	case 'n':
		nspid = atoi(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;

	if(mode == Usage || nspid == 0)
		usage();
	if(argc != 1)
		usage();

	name = strdup(argv[0]);
	sys_msgctl(Mctlwrite, MSGENABLE|MSGALLUSERS);

	switch(mode){
	case Query:
		retval = query_name(nspid, name);
		if(retval < 0){
			fprint(2, "error: query to %d failed (request_status = %d): %r\n",
				nspid, retval);
			exits("query");
		}
		fprint(2, "%s=%d\n", name, retval);
		break;
	case Register:
		retval = register_name(nspid, name);
		if(retval != 0){
			fprint(2, "error: register to %d failed (request_status = %d): %r\n",
				nspid, retval);
			exits("register");
		}
		break;
	}

	exits(nil);
}
