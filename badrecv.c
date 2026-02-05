#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	uvlong msgsz;
	void *msgbuf;

	fprint(2, "pid %d: waiting for message\n", getpid());

	msgenable();
	msgsz = sys_msgwait();
	if(msgsz == 0){
		fprint(2, "msgsz == 0 after msgwait: %r\n");
		exits("msgwait");
	}
	msgbuf = malloc(msgsz);
	if(sys_msgrecv(msgbuf, 2) == 0){
		fprint(2, "sent message too short\n");
		exits("msgrecv");
	}
	fprint(2, "failed successfully. errstr: %r\n");
	if(sys_msgrecv(msgbuf, msgsz) != 0){
		fprint(2, "actual fetch failed: %r\n");
		exits("msgrecv");
	}
	free(msgbuf);
	fprint(2, "msg was %llud bytes\n", msgsz);
	exits(nil);
}