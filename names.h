typedef struct NameMessage NameMessage;

#pragma pack on
struct NameMessage {
	int pid;
	int reply;
	int status;
	u32int namelen;
	char name[1];
};
#pragma pack off

extern int nameserver(char*);
extern int msgregister(Mailbox*, int, char*);
extern int msgregisterpid(Mailbox*, int, char*, int);
extern int msglookupname(Mailbox*, int, char*);
