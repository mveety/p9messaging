#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"
#include "msgnames.h"

char *argv0;

void
usage(void)
{
	fprint(2, "usage: %s [-S name_server] [-n nmsgs] [-s msgsize]\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	char *buffer;
	int msgsize = 24;
	int nmsgs = 100000;
	int target;
	int randfd;
	int namepid;
	char *namesrv = nil;

	vlong start_time;
	vlong end_time;
	vlong total_time;
	vlong seconds;
	vlong milliseconds;

	argv0 = argv[0];

	sys_msgctl(Mctlwrite, MSGENABLE|MSGALLUSERS);

	ARGBEGIN{
	case 'S':
		namesrv = strdup(EARGF(usage()));
		break;
	case 'n':
		nmsgs = atoi(EARGF(usage()));
		break;
	case 's':
		msgsize = atoi(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;

	namepid = name_server(namesrv);
	if(namepid < 1){
		fprint(2, "error: unable to get name_server: %r\n");
		exits("name_server");
	}

	target = query_name(namepid, "msgtest_target");
	if(target < 0){
		fprint(2, "error: unable to find msgtest_target: %r\n");
		exits("no_target");
	}

	if((randfd = open("/dev/random", OREAD)) < 0 ){
		fprint(2, "unable to open /dev/random: %r\n");
		exits("open");
	}

	if(target <= 0 || nmsgs <= 0 || msgsize <= 0){
		fprint(2, "invalid arguments\n");
		exits("usage");
	}

	if(!(buffer = mallocz(msgsize, 1))){
		fprint(2, "unable to malloc: %r\n");
		exits("malloc");
	}

	if(readn(randfd, &buffer[0], msgsize) < 0){
		fprint(2, "read error: %r\n");
		exits("readn");
	}

	fprint(2, "test start: sending %d %d byte message to %d\n",
			nmsgs, msgsize, target);

	start_time = nsec();
	for(int i = 0; i < nmsgs; i++){
		if(sys_msgsend(target, &buffer[0], msgsize) < 0){
			fprint(2, "failed to send message: %r\n");
			exits("msgsend");
		}
	}
	end_time = nsec();

	total_time = end_time - start_time;
	seconds = total_time/1000000000;
	milliseconds = (total_time-(seconds*1000000000))/1000000;

	fprint(2, "took %lld.%03lld seconds to send %d messages\n",
		seconds, milliseconds, nmsgs);
	exits(nil);
}
