#!/bin/bash
#kill -9 BGPD and ZEBRA

#kill -  BGP process
#sudo ps axfc | egrep "(zebra|bgpd)" | grep -v grep | awk '{print "sudo kill -9 " $1}' | sh

#sudo pgrep collect | awk {'print "sudo kill -9 " $1}' | sh
for pid in $(ps -ef | grep "MNLR" | awk '{print $2}'); do sudo kill -9 $pid; done

#rm ~/updates.txt | sh
#rm ~/difference.txt | sh

#rm -rf /tmp/*.txt | sh

#rm -rf /tmp/*.log |sh

#sudo pgrep tshark | awk {'print "sudo kill -9 " $1}' | sh
