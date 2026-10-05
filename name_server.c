#include <u.h>
#include <libc.h>
#include <avl.h>
#include <msg.h>
#include "names.h"

typedef struct Name Name;
typedef struct Pid Pid;
typedef struct ExitMessage ExitMessage;

enum {
	NamesStartSize = 2,
};

struct Name {
	Avl;
	char *name;
	int pid;
	Pid *ppid;
};

struct Pid {
	Avl;
	int pid;
	Name *name;
};

#pragma pack on
struct ExitMessage {
	u64int cookie;
	s64int sender;
};
#pragma pack off

Avltree *names;
Avltree *pids;
int srvpid;
u64int cookie;

int
namestrcmp(Avl *la, Avl *lb)
{
	Name *na = (Name*)la;
	Name *nb = (Name*)lb;

	return strcmp(na->name, nb->name);
}

int
namepidcmp(Avl *la, Avl *lb)
{
	Pid *pa = (Pid*)la;
	Pid *pb = (Pid*)lb;

	if(pa->pid == pb->pid)
		return 0;
	if(pa->pid < pb->pid)
		return -1;
	return 1;
}

int
add_name(char *name, int pid)
{
	Name *newname;
	Pid *newpid;

	newname = mallocz(sizeof(Name), 1);
	newpid = mallocz(sizeof(Pid), 1);
	if(!newname || !newpid){
		fprint(2, "error: bad malloc: %r\n");
		abort();
	}
	newname->name = name;
	newname->pid = pid;
	newname->ppid = newpid;
	newpid->pid = pid;
	newpid->name = newname;
	avlinsert(names, newname);
	avlinsert(pids, newpid);
	return 0;
}

int
remove_name_by_pid(int pid)
{
	Pid *p;
	Pid key;
	Name *name;

	key.pid = pid;
	p = (Pid*)avllookup(pids, &key, 0);
	if(!p)
		return -1;

	name = p->name;
	avldelete(names, p->name);
	avldelete(pids, p);
	free(name->name);
	free(name);
	free(p);
	return 0;
}

int
remove_name_by_name(char *str)
{
	Name *name;
	Name namekey;
	Pid *p;

	namekey.name = str;
	name = (Name*)avllookup(names, &namekey, 0);
	if(!name)
		return -1;
	p = name->ppid;
	avldelete(names, name);
	avldelete(pids, p);
	free(name->name);
	free(name);
	free(p);
	return 0;
}

Name*
find_by_pid(int pid)
{
	Pid pidkey;
	Pid *p;

	pidkey.pid = pid;
	p = (Pid*)avllookup(pids, &pidkey, 0);
	if(!p)
		return nil;
	return p->name;
}

Name*
find_by_name(char *str)
{
	Name *name;
	Name namekey;

	namekey.name = str;
	name = (Name*)avllookup(names, &namekey, 0);
	if(!name)
		return nil;
	return name;
}

_Noreturn void
response_allocate_error(void)
{
	fprint(2, "error: unable to allocate response: %r\n");
	abort();
}

void
name_server(void)
{
	Message *msg;
	NameMessage *nmsg;
	MonitorMsg *mmsg;
	Message *resp;
	NameMessage *respnmsg;
	Name *lookup;
	char *namebuf;
	s64int *respint;
	ExitMessage *emsg;
	int pid;
	int replypid;

	int tags[] = {TagDefault, TagMonitor, TagRequestName, TagRegisterName};

	for(;;){
		msg = msgrecvfilter(nil, tags, nelem(tags));
		if(msg == nil){
			fprint(2, "warning: got empty message\n");
			continue;
		}

		switch(msg->tag){
		case TagDefault:
			if(msg->len < sizeof(ExitMessage))
				continue;
			emsg = msg->data;
			if(emsg->cookie == cookie && emsg->sender == srvpid){
				fprint(2, "notice: exiting...\n");
				exits(nil);
			} else {
				fprint(2, "bad exit message\n");
				fprint(2, "cookie %llud (got %llud)\n", cookie, emsg->cookie);
				fprint(2, "srvpid %d (got %lld)\n", srvpid, emsg->sender);
			}
			break;
		case TagMonitor:
			mmsg = msg->data;
			if(mmsg->event & (MT_Process|ME_Death)){
				lookup = find_by_pid(mmsg->object);
				if(remove_name_by_pid(mmsg->object) < 0)
					fprint(2, "warning: monitor tried to remove untrack proc %d\n", mmsg->object);
				else
					fprint(2, "notice: removed dead process %d (%s)\n", mmsg->object, lookup->name);
			}
			break;
		case TagRegisterName:
			nmsg = msg->data;
			namebuf = mallocz(nmsg->namelen+3, 1);
			if(!namebuf){
				fprint(2, "error: bad malloc: %r\n");
				exits("malloc");
			}
			strncpy(namebuf, nmsg->name, nmsg->namelen);
			pid = nmsg->pid;
			replypid = nmsg->reply;
			fprint(2, "note: trying to register pid %d as %s\n", pid, namebuf);
			lookup = find_by_name(namebuf);
			resp = message(TagNameError, nil, sizeof(u64int));
			if(!resp){
				fprint(2, "error: bad malloc: %r\n");
				exits("malloc");
			}
			respint = msg->data;
			if(lookup == nil) {
				monitor(pid, MT_Process|ME_Death);
				add_name(namebuf, pid);
				*respint = 0;
				fprint(2, "notice: registered name %s -> %d\n", namebuf, pid);
			} else {
				fprint(2, "notice: %s already exists as %d\n", lookup->name, lookup->pid);
				*respint = -1;
			}
			msgsend(replypid, resp);
			freemessage(resp);
			break;
		case TagRequestName:
			nmsg = msg->data;
			if(nmsg->pid != 0){
				nmsg->status = -1;
				resp = message(TagNameResponse, nmsg, msg->len);
			} else {
				lookup = find_by_name(nmsg->name);
				if(lookup == nil){
					nmsg->status = -1;
					resp = message(TagNameResponse, nmsg, msg->len);
				} else {
					resp = message(TagNameResponse, nil, sizeof(NameMessage)+strlen(lookup->name)+2);
					respnmsg = resp->data;
					respnmsg->pid = lookup->pid;
					respnmsg->reply = getpid();
					respnmsg->status = 0;
					strcpy(respnmsg->name, lookup->name);
					respnmsg->namelen = strlen(lookup->name);
				}
			}
			msgsend(nmsg->reply, resp);
			freemessage(resp);
			break;
		}
		freemessage(msg);
	}
}

void
usage(void)
{
	fprint(2, "usage %s [-s] [-S srvname]\n", argv0);
	exits("usage");
}

void
srvproc(char *srvfile, int srvfd, int pid)
{
	char buffer[32];
	vlong sz;
	Message *msg;
	ExitMessage *emsg;

	msgenable();
	fprint(2, "note: registering self (%d) as name_server\n", pid);
	msgregisterpid(nil, pid, "name_server", pid);
	msgdisable();
	for(;;) {
		memset(&buffer[0], 0, sizeof(buffer));
		sz = read(srvfd, &buffer[0], sizeof(buffer)-1);
		if(sz < 0){
			fprint(2, "error: srv: %r\n");
			exits("read");
		}
		if(strcmp(buffer, "pid") == 0){
			if(fprint(srvfd, "%10 d", pid) < 0){
				fprint(2, "error: srv: %r\n");
				exits("write");
			}
		} else if(strcmp(buffer, "exit") == 0){
			fprint(srvfd, "ok");
			msg = message(TagDefault, nil, sizeof(ExitMessage));
			emsg = msg->data;
			emsg->cookie = cookie;
			emsg->sender = srvpid;
			close(srvfd);
			if(remove(srvfile) < 0)
				fprint(2, "error: unable to remove %s: %r\n", srvfile);
			msgsend(pid, msg);
			exits(nil);
		} else {
			fprint(srvfd, "invalid");
			fprint(2, "srv: got invalid message: \"%s\"\n", buffer);
		}
	}
}

int
main(int argc, char *argv[])
{
	enum { User, System } scope = User;
	char *srvname = nil;
	char *srv;
	int srvfd, srvpipe[2];
	int parentpid;

	argv0 = argv[0];
	cookie = truerand();
	ARGBEGIN{
	case 's':
		scope = System;
		break;
	case 'S':
		srvname = strdup(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;

	names = avlcreate(namestrcmp);
	pids = avlcreate(namepidcmp);

	if(srvname == nil)
		srvname = "name_server";

	srv = smprint("/srv/%s", srvname);
	pipe(srvpipe);
	srvfd = create(srv, OWRITE, scope == System ? 0666 : 0600);
	if(srvfd < 0){
		fprint(2, "error unable to create %s: %r\n", srv);
		exits("create");
	}
	fprint(srvfd, "%d", srvpipe[0]);
	close(srvfd);
	close(srvpipe[0]);
	parentpid = getpid();

	switch(scope){
	case User:
		if(sys_msgctl(Mctlwrite, MSGENABLE|MSGMONITOR|MSGPROCS) != (MSGENABLE|MSGMONITOR|MSGPROCS)){
			fprint(2, "error: unable to msgctl: %r\n");
			exits("msgctl");
		}
		fprint(2, "note: name server starting as pid %d\n", getpid());
		break;
	case System:
		if(sys_msgctl(Mctlwrite, MSGENABLE|MSGMONITOR|MSGALLUSERS|MSGPROCS) != (MSGENABLE|MSGMONITOR|MSGALLUSERS|MSGPROCS)){
			fprint(2, "error: unable to msgctl: %r\n");
			exits("msgctl");
		}
		fprint(2, "note: system name server starting as pid %d\n", getpid());
		break;
	}

	switch((srvpid = rfork(RFPROC|RFMEM|RFNOWAIT))){
	case 0:
		srvproc(srv, srvpipe[1], parentpid);
		exits(nil);
		break;
	case -1:
		fprint(2, "error: unable to rfork: %r\n");
		exits("rfork");
		break;
	default:
		break;
	}

	name_server();

	// should be dead code here
	assert(0);
	return 0;
}