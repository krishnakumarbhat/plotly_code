#!/bin/sh

#------CREATING  CONTAINER AND EXECUTING SET OF LOG IN CONTAINER---------

docker run -it -v $(pwd)/INPUT_LOGS/:/input/ -v $(pwd)/:/output/ tools_mudp:18_0_55 /RUN_MUDP.sh $1 $2 $3 ;echo $?
#------------END OF SCRIPT-------------
