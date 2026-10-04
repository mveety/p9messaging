#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	Mailbox *mbox;
	Message *m;

	USED(argc);
	USED(argv);

	fprint(2, "pid %d: waiting 10 for messages\n", getpid());
	
	msgenable();
	mbox = mailbox();
	while(mailboxsz(mbox) < 10){
		m = msgrecv(mbox);
		if(m == nil){
			fprint(2, "error: %r\n");
			exits("msgrecv");
		}
		fprint(2, "got msgtype = %d from pid %d (mboxsize = %llud, mbox->i = %llud)\n",
				m->tag, m->pid, mailboxsz(mbox), mbox->i);
	}
	flushmailbox(mbox);
	fprint(2, "mboxsize after flush = %llud\n", mailboxsz(mbox));

	exits(nil);
}
