#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	char msgbody[] = "Hello world!";
	char *msgin;
	uintptr msginsz;

	USED(argc);
	USED(argv);

	msgenable();

	if(sys_msgsend(getpid(), &msgbody[0], sizeof(msgbody)) < 0){
		fprint(2, "error: unable to send message to self: %r\n");
		exits("msgsend");
	}

	if((intptr)(msginsz = sys_msgwait()) < 0){
		fprint(2, "error: unable to wait for message: %r\n");
		exits("msgwait");
	}
	if(msginsz == 0){
		fprint(2, "error: zero length message: %r\n");
		exits("msgwait");
	}

	if(!(msgin = mallocz(msginsz, 1))){
		fprint(2, "error: malloc: %r");
		exits("malloc");
	}
	if(sys_msgrecv(msgin, msginsz) < 0){
		fprint(2, "error: msgrecv: %r\n");
		exits("msgrecv");
	}

	fprint(2, "sent (size = %d) = \"%s\"\n", sizeof(msgbody), &msgbody[0]);
	fprint(2, "recv (size = %d) = \"%s\"\n", msginsz, msgin);

	exits(nil);
}