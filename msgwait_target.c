#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"
#include "msgnames.h"

char *argv0;

void
usage(void){
	fprint(2, "usage: %s [-S namesrv]\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	char buffer[1024];
	uintptr msgsz;
	int failures = 0;
	char *namesrv = nil;
	int namepid;
	char c[5];

	msgenable();

	argv0 = argv[0];
	ARGBEGIN{
	case 'S':
		namesrv = strdup(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;

	namepid = name_server(namesrv);
	if(namepid < 0){
		fprint(2, "error: no name server found: %r\n");
		exits("no name server");
	}
	
	if(register_name(namepid, "msgtest_target") < 0){
		fprint(2, "error: unable to register name: %r\n");
		exits("register_name");
	}

	fprint(2, "pid %d: waiting for messages\n", getpid());
	fprint(2, "press enter to process messages...\n");
	read(0, &c[0], sizeof(c)-1);

	fprint(2, "doing msgwait+msgrecv now\n");
	for(;failures < 10;){
		msgsz = sys_msgwait();
		if((intptr)msgsz < 0) {
			failures++;
			fprint(2, "msgwait failure: %r\n");
			continue;
		}
		if(msgsz == 0) {
			failures++;
			fprint(2, "msgwait returned 0: %r\n");
			continue;
		}
		if(msgsz > sizeof(buffer)) {
			fprint(2, "message too big (msgsz > %d)\n", sizeof(buffer));
			exits("buffer too small");
		}
		if((vlong)sys_msgrecv(&buffer[0], msgsz) < 0) {
			failures++;
			fprint(2, "msgrecv failure: %r\n");
			continue;
		}
	}
	fprint(2, "too many failures\n");
	exits("too many failures");
}
