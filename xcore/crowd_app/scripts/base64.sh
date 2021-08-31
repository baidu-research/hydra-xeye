#!/bin/bash

op=$1
src_file=$2
dst_file=$3


[ -z ${op} ] && echo "invalid operation: ${op}" && exit 1
[ -z ${src_file} ] && echo "invalid src file: ${src_file}" && exit 1
[ -z ${dst_file} ] && echo "invalid dst file:${dst_file}" && exit 1


if [ ${op} == "encode" ]; then
    base64 ${src_file} >> ${dst_file}
    if [ $? != 0 ]; then
        echo "can not encode file: ${src_file}"
        exit 1
    else
        exit 0
    fi
elif [ ${op} == "decode" ]; then
    base64 --decode ${src_file} >> ${dst_file}
    if [ $? != 0 ]; then
        echo "can not decode file: ${src_file}"
        exit 1
    else
        exit 0
    fi
else
    echo "operation: ${op} is not support"
fi
