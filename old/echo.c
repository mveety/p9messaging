#include <u.h>
#include <libc.h>
#include <msg.h>

char *argv0;

int
msgsendfree(int pid, Message *msg)
{
	if(msgsend(pid, msg) < 0)
		return -1;
	freemsg(msg);
	return 0;
}

int
sendecho(int pid)
{
	char *payload = "hello world";
	Message *resp;
	int tags[] = {-1};

	fprint(2, "sending \"hello world\" to %d\n", pid);

	if(msgsendfree(pid, message(-1, payload, strlen(payload)+1)) < 0){
		fprint(2, "unable to send to %d: %r\n", pid);
		exits("msgsend");
	}

	resp = msgrecvfilter(nil, tags, nelem(tags));
	if(resp == nil){
		fprint(2, "got a nil message: %r\n");
		exits("msgrecv");
	}
	fprint(2, "got response from %d: \"%s\"\n", resp->pid, resp->data);
	exits(nil);
}

int
recvecho(void)
{
	int tags[] = {-1, -2};
	Message *req;
	int sender;

	fprint(2, "send a -1 tag to %d for an echo and a -2 tag to exit\n", getpid());
	for(;;) {
		req = msgrecvfilter(nil, tags, nelem(tags));
		if(req == nil){
			fprint(2, "got a nil message: %r\n");
			exits("msgrecv");
		}
		switch(req->tag){
		default:
			assert(0);
			break;
		case -1:
			fprint(2, "got a message from %d: \"%s\"\n", req->pid, req->data);
			sender = req->pid;
			req->pid = getpid();
			if(msgsend(sender, req) < 0)
				fprint(2, "unable to send to %d: %r\n", sender);
			freemsg(req);
			break;
		case -2:
			fprint(2, "got a -2 tag from %d: exitting...\n", req->pid);
			exits(nil);
			break;
		}
	}
}

void
usage(void)
{
	fprint(2, "usage: send: %s -s pid\n", argv0);
	fprint(2, "    receive: %s -r\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	enum {Usage, Send, Recv} task = Usage;
	int pid = 0;

	argv0 = strdup(argv[0]);
	ARGBEGIN{
	case 's':
		pid = atoi(EARGF(usage()));
		if(pid > 0)
			task = Send;
		break;
	case 'r':
		task = Recv;
		break;
	default:
		usage();
	}ARGEND;

	if(pid == 0)
		usage();

	if(sys_msgctl(Mctlwrite, MSGENABLE|MSGALLUSERS) != (MSGENABLE|MSGALLUSERS)){
		fprint(2, "unable to set msgctl: %r\n");
		exits("msgctl");
	}
	switch(task){
	default:
		assert(0);
		break;
	case Usage:
		usage();
		break;
	case Send:
		sendecho(pid);
		break;
	case Recv:
		recvecho();
		break;
	}

	exits(nil);
}