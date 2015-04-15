#!/bin/bash
bin='test'
export LD_LIBRARY_PATH="../lib:"

pid=`more ant_wars.pid | awk '{print $2}'`

bin_pid=`ps -ef |grep -w $pid |grep -v "grep"`
if  test -n "${bin_pid}"
then
    kill  -9 $pid
    #echo  done 
fi
nohup ./${bin}  127.0.0.1  9999
#nohup ./${bin}  127.0.0.1  9999&
                   
