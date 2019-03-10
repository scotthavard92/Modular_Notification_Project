# !/bin/bash
#
#
#Read Serial From Terminal

cd /
cd dev

screen tty.usbserial* 9600 #Make sure to set the proper baud rate

exit 0