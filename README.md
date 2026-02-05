# Plan 9 Message Passing Examples

Examples making use of the new messaging system calls.

Requires the [msgpassing kernel](https://github.com/mveety/9front-messaging).

To pull down and switch to my branch do something like this:
```
bind -ac /dist/9front /
cd /
{
	echo '[remote "mveety]'
	echo '    url=https://github.com/mveety/9front-messaging.git'
	echo ''
} >> /.git
git/pull -f -u mveety
git/branch -b remotes/mveety/msgpassing -n msgpassing
```
