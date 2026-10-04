#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	char buffer[1024];
	uintptr msgsz;
	int failures = 0;

	USED(argc);
	USED(argv);

	msgenable();
	fprint(2, "pid %d: waiting for messages\n", getpid());

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
