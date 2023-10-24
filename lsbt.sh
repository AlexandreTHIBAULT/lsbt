#!/bin/bash
make

if [ ! -f "/bin/lsbt" ]; then
    PATH_PARENT=$( dirname $(realpath ${BASH_SOURCE[0]}) )
    cd $PATH_PARENT
    sudo ln -s $PATH_PARENT/lsbt /bin/lsbt
fi


