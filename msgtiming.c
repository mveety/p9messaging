#include <u.h>
#include <libc.h>

int
main(int argc, char *argv[])
{
	char *argv1;
	char *buffer;
	int msgsize;
	int nmsgs;
	int target;
	int randfd;
	vlong start_time;
	vlong end_time;
	vlong total_time;
	vlong seconds;
	vlong milliseconds;

	if(argc != 4) {
		fprint(2, "usage: %s target nmsgs msgsize\n", argv[0]);
		exits("usage");
	}

	if((randfd = open("/dev/random", OREAD)) < 0 ){
		fprint(2, "unable to open /dev/random: %r\n");
		exits("open");
	}

	target = atoi(argv[1]);
	nmsgs = atoi(argv[2]);
	msgsize = atoi(argv[3]);

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
