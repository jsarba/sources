trap "" 1 2 3 4 5 15

PATH=/usr/bin:/etc:/usr/sbin:/usr/ucb:$HOME/bin:/usr/bin/X11:/sbin:$HOME/sbin:.

export PATH

alias menu='menu.sh $HOME/menues/menudrp'

if [ -s "$MAIL" ]           # This is at Shell startup.  In normal
then echo "$MAILMSG"        # operation, the Shell checks
fi                          # periodically.
