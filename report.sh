#!/bin/sh
####  Create by          : Dennis
####  Last Modified Date : 2015/10/12
####  Version            : V1.0.0
####  Decription         : If the process of mxreport is not exist，the shell will do starting the
####                       the process of mxreport and printing the start log to logfile. If the  
####                       process of mxreport is exist，the shell will do restarting the the process 
####                       of mxreport and printing the restart log to logfile. At the same time, the 
####                       mxreport's log have writing to the log. Also, this shell include cleanning 
####                       log files before two days.

#### Global Variable
LOG_FILE_PATH="/mnt/bf/mx/logs/reportLog`date \"+%Y%m%d\"`"
CLEAR_LOG_PATH="/mnt/bf/mx/logs/"
CLEAR_LOG_FILE="reportLog20*"
PROCESS_MX="mxreport"

echo >> $LOG_FILE_PATH
echo ----- `date "+%Y-%m-%d %T"` --- "Starting report.sh. Cleaning logs..." >> $LOG_FILE_PATH

#### Clean Log file 
REMOVE_FLAG=0
for i in $(ls -t $CLEAR_LOG_PATH$CLEAR_LOG_FILE); do
	echo $i & >> /dev/null
	REMOVE_FLAG=$(( $REMOVE_FLAG + 1 ))
	if [ $REMOVE_FLAG -gt 3 ]; then
		rm -rf $i >> $LOG_FILE_PATH
		echo `date "+%Y-%m-%d %T"` "Remove logfile: $i" >> $LOG_FILE_PATH
	fi
done


cd /mnt/bf/mx; ./$PROCESS_MX >> $LOG_FILE_PATH &

exit 0
