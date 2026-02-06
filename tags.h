/* Non-confidential */
/* No warranty in life supporting applications */

/*
By convention, the tags < 0 are special and have special message
types.
*/

enum {
	TagEcho = -1,
	TagExit = -2,

	TagResolvedName = -3,
	TagNameStatus = -4,
	TagRequestName = -5,
	TagRegisterName = -6,

	TagUnknown = -127,
// sys_monitor is not yet implemented on plan 9 and will
// have a very different structure than netbsd.
	TagMonitor = -128,
	TagAlive = -129,
};

typedef struct SystemMessage SystemMessage;
typedef struct ExitMessage ExitMessage;
typedef struct NameStatusMsg NameStatusMsg;
typedef struct ResolvedNameMsg ResolvedNameMsg;
typedef struct RegisterNameMsg RegisterNameMsg;
typedef struct NameRequestMsg NameRequestMsg;

struct ExitMessage {
	s32int *code;
};

struct NameStatusMsg {
	s32int *request_tag;
	s32int *request_status;
};

struct ResolvedNameMsg {
	s32int *pid;
	u32int *namelen;
	char *name;
};

struct RegisterNameMsg {
	u32int *namelen;
	char *name;
};

struct NameRequestMsg {
	u32int *namelen;
	char *name;
};

struct SystemMessage {
	Message *carrier;
	int *tag;
	int *pid;
	union {
		ExitMessage exitmsg;
		NameStatusMsg namestatus;
		ResolvedNameMsg resolvedname;
		RegisterNameMsg registername;
		NameRequestMsg namerequest;
	};
};

void free_systemmessage(SystemMessage*);
SystemMessage *parse_systemmsg(Message *);
SystemMessage *new_exitmessage(s32int);
SystemMessage *new_namestatus(s32int, s32int);
SystemMessage *new_resolvedname(s32int, char*, u32int);
SystemMessage *new_registername(char*, u32int);
SystemMessage *new_namerequest(char*, u32int);
