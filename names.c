#include <u.h>
#include <libc.h>
#include <msg.h>
#include "names.h"

int
nameserver(char *srvname)
{
	int srvfd;
	int nspid;
	char buffer[32];

	if(srvname == nil)
		srvname = "/srv/name_server";

	srvfd = open(srvname, ORDWR);
	if(srvfd < 0)
		return -1;

	memset(&buffer[0], 0, sizeof(buffer));

	fprint(srvfd, "pid");
	if(read(srvfd, &buffer[0], sizeof(buffer)) < 10){
		close(srvfd);
		fprint(2, "error: invalid srv message: %r\n");
		return -1;
	}
	close(srvfd);

	nspid = atoi(buffer);
	if(nspid == 0){
		fprint(2, "error: unable to get name_server\n");
		return -1;
	}

	return nspid;
}

int
msgregisterpid(Mailbox* mbox, int nspid, char *name, int pid)
{
	Message *msg;
	NameMessage *nmsg;
	Message *resp;
	s64int *respint;
	int status;
	int tags[] = {TagNameError};

	msg = message(TagRegisterName, nil, sizeof(NameMessage)+strlen(name)+2);
	if(!msg)
		return -1;
	nmsg = msg->data;
	
	nmsg->pid = pid;
	nmsg->reply = getpid();
	nmsg->status = 0;
	nmsg->namelen = strlen(name);
	strcpy(nmsg->name, name);

	msgsend(nspid, msg);
	resp = msgrecvfilter(mbox, tags, nelem(tags));
	if(resp == nil){
		freemessage(msg);
		return -1;
	}
	if(mbox != nil)
		selectmsg(mbox, resp);

	respint = resp->data;
	status = *respint;

	freemessage(msg);
	freemessage(resp);

	return status;
}

int
msgregister(Mailbox* mbox, int nspid, char *name)
{
	return msgregisterpid(mbox, nspid, name, getpid());
}

int
msglookupname(Mailbox *mbox, int nspid, char *name)
{
	Message *msg;
	NameMessage *nmsg;
	Message *resp;
	NameMessage *respnmsg;
	int status;
	int tags[] = {TagNameResponse};

	msg = message(TagRequestName, nil, sizeof(NameMessage)+strlen(name)+2);
	if(!msg)
		return -1;
	nmsg = msg->data;

	nmsg->pid = 0;
	nmsg->reply = getpid();
	nmsg->status = 0;
	nmsg->namelen = strlen(name);
	strcpy(nmsg->name, name);

	msgsend(nspid, msg);
	resp = msgrecvfilter(mbox, tags, nelem(tags));
	if(resp == nil){
		freemessage(msg);
		return -1;
	}
	if(mbox != nil)
		selectmsg(mbox, resp);

	respnmsg = resp->data;

	if(respnmsg->status < 0)
		status = respnmsg->status;
	else
		status = respnmsg->pid;

	freemessage(msg);
	freemessage(resp);

	return status;
}
