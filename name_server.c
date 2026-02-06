/* Non-confidential */
/* No warranty in life supporting applications */

/*
note: Plan 9 does not yet have sys_monitor implemented so
the name server has no way to know if names are valid, living
processes. This will be fixed at a later date when sys_monitor
is added.
*/

#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"

typedef struct Name Name;

enum {
	NamesStartSize = 2,
};

struct Name {
	char *name;
	int pid;
};

Name **names;
uintptr namessz = 0;

int
grow_names(void)
{
	Name **old_names;
	uintptr old_namessz;
	Name **new_names;
	uintptr new_namessz;

	if(names == nil){
		names = mallocz(NamesStartSize*sizeof(Name*), 1);
		if(!names){
			fprint(2, "unable to alloc names: %r\n");
			abort();
		}
		namessz = NamesStartSize;
		return -1;
	}

	old_names = names;
	old_namessz = namessz;

	new_namessz = 2*old_namessz;
	new_names = mallocz(new_namessz*sizeof(Name*), 1);
	if(!new_names){
		fprint(2, "bad names resize: %r\n");
		abort();
	}
	memmove(new_names, old_names, old_namessz*sizeof(Name*));
	free(old_names);
	names = new_names;
	namessz = new_namessz;

	return 0;
}

int
add_name(char *name, int pid)
{
	uintptr i;
	Name *newname;

	for(i = 0; i < namessz; i++){
		if(names[i] == nil){
			newname = malloc(sizeof(Name));
			if(!newname){
				fprint(2, "unable to malloc new name: %r\n");
				abort();
			}
			newname->name = name;
			newname->pid = pid;
			names[i] = newname;
			return 0;
		}
	}

	grow_names();

	for(i = 0; i < namessz; i++){
		if(names[i] == nil){
			newname = malloc(sizeof(Name));
			if(!newname){
				fprint(2, "unable to malloc new name: %r\n");
				abort();
			}
			newname->name = name;
			newname->pid = pid;
			names[i] = newname;
			return 0;
		}
	}

	fprint(2, "unable to find an empty name!\n");
	abort();
}

int
remove_name_by_pid(int pid)
{
	uintptr i;

	for(i = 0; i < namessz; i++){
		if(names[i] == nil)
			continue;
		if(names[i]->pid == pid){
			free(names[i]->name);
			free(names[i]);
			names[i] = nil;
			return 0;
		}
	}

	return -1;
}

int
remove_name_by_name(char *name)
{
	uintptr i;

	for(i = 0; i < namessz; i++){
		if(names[i] == nil)
			continue;
		if(strcmp(name, names[i]->name) == 0){
			free(names[i]->name);
			free(names[i]);
			names[i] = nil;
			return 0;
		}
	}

	return -1;
}

Name*
find_by_pid(int pid)
{
	uintptr i;

	for(i = 0; i < namessz; i++){
		if(names[i] == nil)
			continue;
		if(names[i]->pid == pid)
			return names[i];
	}

	return nil;
}

Name*
find_by_name(char *name)
{
	uintptr i;

	for(i = 0; i < namessz; i++){
		if(names[i] == nil)
			continue;
		if(strcmp(name, names[i]->name) == 0)
			return names[i];
	}

	return nil;
}

_Noreturn void
response_allocate_error(void)
{
	fprint(2, "error: unable to allocate response: %r\n");
	abort();
}

void
send_response(SystemMessage *smsg, SystemMessage *resp)
{
	if(!resp)
		response_allocate_error();
	if(msgsend(*smsg->pid, resp->carrier) < 0)
		fprint(2, "warning: unable to send response to %d: %r\n", *smsg->pid);
}

void
name_server(void)
{
	Message *msg;
	SystemMessage *smsg, *resp = nil;
	char *namebuf;
	Name *lookup;
	int tags[] = {TagExit, TagRequestName, TagRegisterName, TagNameStatus};

	for(;;){
		msg = msgrecvfilter(nil, tags, nelem(tags));
		if(msg == nil){
			fprint(2, "warning: got empty message\n");
			continue;
		}

		smsg = parse_systemmsg(msg);
		if(smsg == nil){
			fprint(2, "warning: unable to parse system message from %d: %r\n", msg->pid);
			freemsg(msg);
			continue;
		}

		switch(*smsg->tag){
		case TagExit:
			fprint(2, "note: got exit message from %d. quitting...\n", *smsg->pid);
			exits(nil);
			break;
		case TagRequestName:
			if(*smsg->namerequest.namelen <= strlen(smsg->namerequest.name)){
				fprint(2, "warning: got invalid message from %d\n", *smsg->pid);
				free_systemmessage(smsg);
				continue;
			}
			lookup = find_by_name(smsg->namerequest.name);
			if(!lookup)
				resp = new_namestatus(TagRequestName, -1);
			else
				resp = new_resolvedname(lookup->pid, lookup->name, strlen(lookup->name)+1);

			break;
		case TagRegisterName:
			if(*smsg->registername.namelen <= strlen(smsg->registername.name)){
				fprint(2, "warning: got invalid message from %d\n", *smsg->pid);
				free_systemmessage(smsg);
				continue;
			}
			lookup = find_by_name(smsg->registername.name);
			if(!lookup){
				/*if(monitor(MT_Process|ME_Death, *smsg->pid) < 0){
					fprint(2, "warning: unable to monitor process %d: %r\n", *smsg->pid);
					resp = new_namestatus(TagRegisterName, 0);
				} else {*/
					namebuf = strdup(smsg->registername.name);
					add_name(namebuf, *smsg->pid);
					fprint(2, "note: registered %d as \"%s\"\n", *smsg->pid, namebuf);
					resp = new_namestatus(TagRegisterName, 0);
				//}
			} else
				resp = new_namestatus(TagRegisterName, -1);
			break;
		case TagNameStatus:
			if(*smsg->pid == getpid() && *smsg->namestatus.request_tag == TagRegisterName){
				if(*smsg->namestatus.request_status != 0) {
					fprint(2, "error: unable to register self! (request_status = %d)\n",
						*smsg->namestatus.request_status);
					abort();
				}
				fprint(2, "note: registered self as \"name_server\"\n");
				free_systemmessage(smsg);
				continue;
			}
			if(*smsg->namestatus.request_tag == TagAlive)
				resp = new_namestatus(TagAlive, 0);
			else
				resp = new_namestatus(TagUnknown, 0);
			break;
/*		case TagMonitor:
			namebuf = nil;
			if(!(*smsg->monitor.event & (MT_Process|ME_Death)){
				fprint(2, "warning: got spurious monitor message\n");
				free_systemmessage(smsg);
				continue;
			}
			lookup = find_by_pid(*smsg->monitor.oid);
			if(lookup)
				namebuf = strdup(lookup->name);
			if(remove_name_by_pid(*smsg->monitor.oid) < 0) {
				fprint(2, "warning: got spurious pid death for %d\n", *smsg->monitor.oid);
				free_systemmessage(smsg);
				if(namebuf)
					free(namebuf)
				continue;
			}
			unmonitor(*smsg->monitor.id);
			fprint(2, "removed \"%s\" (pid %d) from names\n", namebuf, *smsg->monitor.pid);
			free_systemmessage(smsg);
			free(namebuf);
			continue; */
		}

		send_response(smsg, resp);
		free_systemmessage(smsg);
		free_systemmessage(resp);
	}
}

char *argv0;

void
usage(void)
{
	fprint(2, "usage %s [-s]\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	enum { User, System } scope = User;
	SystemMessage *startmsg;
	char name[] = "name_server";

	argv0 = argv[0];
	ARGBEGIN{
	case 's':
		scope = System;
		break;
	default:
		usage();
		break;
	}ARGEND;

	switch(scope){
	case User:
		if(sys_msgctl(Mctlwrite, MSGENABLE) != MSGENABLE){
			fprint(2, "error: unable to msgctl: %r\n");
			exits("msgctl");
		}
		fprint(2, "note: name server starting as pid %d\n", getpid());
		break;
	case System:
		if(sys_msgctl(Mctlwrite, MSGENABLE|MSGALLUSERS) != (MSGENABLE|MSGALLUSERS)){
			fprint(2, "error: unable to msgctl: %r\n");
			exits("msgctl");
		}
		fprint(2, "note: system name server starting as pid %d\n", getpid());
		break;
	}

	startmsg = new_registername(name, sizeof(name));
	if(msgsend(getpid(), startmsg->carrier) < 0) {
		fprint(2, "error: unable to register self: %r\n");
		abort();
	}
	free_systemmessage(startmsg);

	name_server();

	// should be dead code here
	assert(0);
	return 0;
}


