#!/bin/sh


#------SCRIPT IS CALLED TO EXECUTE APPLICATION WITH LOGS INSIDE CONTAINER-------


ch_k=0
ch_f=0


if [ -f "$1" -a -f "$2" ]
then
echo " "
else
       ch_k=1
fi


if [ ! -d "$3" ]
then
       ch_f=1
fi


exitcode=1
#----------VALIDATING INPUT PARAMETERS-------------------------
if [ $ch_k -eq 1 ]
then
       echo INCORRECT COFIGURATION FILE PATH OR LOG FILE PATH 
       exit $exitcode
fi


#--------------Running Application inside container-------------------- 


cd /OUTPUT/
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./
if [ $ch_f -eq 1 ]
then
./MUDP_Log_DataExtracter $1 $2
else
./MUDP_Log_DataExtracter $1 $2 $3
fi
exitcode=$?


if [ $exitcode -eq 0 ]
then
        echo Files Generated Successfully
        exit 0
else
        echo Files generated With Errror
        exit $exitcode
fi


