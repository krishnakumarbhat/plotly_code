#!/bin/sh

#------CREATING  CONTAINER AND EXECUTING SET OF LOG IN CONTAINER---------

docker run -it -v $(pwd)/INPUT_LOGS/:/input/ -v $(pwd)/:/output/ tools_mudp:16_0_2 /RUN_MUDP.sh $1 $2 $3 ;echo $?
#------------END OF SCRIPT-------------
