#!/bin/bash
ffmpeg -i $1 -y -vcodec copy -acodec copy $2
if [ "$?" -ne 0 ]; then
	echo "ffmpeg convert h264 to flv fail !!!"
	exit 1
fi
