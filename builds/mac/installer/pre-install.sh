#!/bin/bash - 
#===============================================================================
#
#          FILE: pre-install.sh
# 
#         USAGE: ./pre-install.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Jeremy Lan 
#  ORGANIZATION: 
#       CREATED: 2017/08/28 17:14
#      REVISION:  ---
#===============================================================================

if [[ "$SUDO_USER" == "" ]] ; then
	echo "This script requires elevated privileges."
	sudo $0
	exit;
fi

# uninstall any ancient version
su $SUDO_USER -c "test -f /usr/local/bin/demvs && /usr/local/bin/demvs || true"
su $SUDO_USER -c "killall -9 mvsd && sleep 5"
METAVERSE=/Applications/Metaverse.app
if [ -d "$METAVERSE" ]; then
    su $SUDO_USER -c "rm -rf /Applications/Metaverse.app"
fi
