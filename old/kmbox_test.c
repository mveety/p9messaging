#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	char buf[10];
	int i = 0;
	Message *msg;

	USED(argc);
	USED(argv);

	fprint(2, "pid %d waiting for messages\n", getpid());
	msgenable();
	read(0, &buf[0], sizeof(buf));

	while((msg = msgrecv(nil)) != nil){
		fprint(2, "%d: got msg type %d\n", i, msg->tag);
		freemsg(msg);
	}

	exits(nil);
}
