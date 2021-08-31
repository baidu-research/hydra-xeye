#!/bin/bash

# usage
# bash release.sh lipindian v1.07


red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

source $(pwd)/../../../config_environment.sh

store_name=$1
model_ver=$2

# command checking
declare -a cmd_used=("tput" "git" "awk" "make" "md5sum" "grep")

for cmd in "${cmd_used[@]}"
do
    if ! [ -x "$(command -v $cmd)" ]; then
     echo "${red}Error: $cmd is not installed.${reset}"
     exit 1
fi
done

# manually hanlde control + c
trap 'Ctrl + C, exit' INT

[ -z ${store_name} ] && echo "${red}you must specify this version is released for which store.${reset}" && exit 1

[ -z ${model_ver} ] && echo "${red}you must specify which model version is released for ${store_name} store.${reset}" && exit 1

VERSION_NUM=$(awk -F= '{if($1=="version") print $2}' ../../../../metadata)
: ${VERSON_NUM:=0.0.1}

get_version()
{
    if [ -d .git -o \
         -d ../.git -o \
         -d ../../.git -o \
         -d ../../../.git -o \
         -d ../../../../.git -o \
         -d ../../../../../.git ]; then
        extra="$(git rev-parse --verify --short HEAD)"
        VERSION_EXTRA="-$extra"
        if git diff-index --name-only HEAD |  grep -v "^scripts/package" | read dummy ; then
            VERSION_EXTRA="${VERSION_EXTRA}-dirty!"
        else
            VERSION_EXTRA="${VERSION_EXTRA}-commit"
        fi
    else
        echo -e "\n\n\033[31mERROR: oh no git found\033[37m\n\n"
        exit 1
    fi
}

get_version

VERSION=V$VERSION_NUM$VERSION_EXTRA

folder_name="crowd_firmware"
ver_file_name="readme.txt"

echo "Release for store: ${green}$store_name${reset}, "\
     "model version: ${green}$model_ver${reset}, " \
     "firmware_ver: ${green}$VERSION${reset}"

echo "Start to clean up the previous build files..."

make clean -j &> /dev/null
if [ $? != 0 ]; then
    echo "${red}ERROR: clean files failed...${reset}"
    exit 1
fi

echo "Start to build a firmware, this will take some times, please be patient..."



make -j &> /dev/null
if [ $? != 0 ]; then
    echo "${red}ERROR: Compilation failed...${reset}"
    exit 1
fi

rm -rf ${folder_name}


mkdir ${folder_name} &> /dev/null
if [ $? != 0 ]; then
    echo "${red}ERROR: create directory ${folder_name} FAILED${reset}"
    exit 1
fi

cp output/crowd.mvcmd crowd_firmware/firmware &> /dev/null
if [ $? != 0 ]; then
    echo "${red}error: copy firmware failed"
    exit 1
fi

config_file="conf_"
config_file+=$store_name
config_file+=".json"

cp config/${config_file} crowd_firmware/config &> /dev/null

if [ $? != 0 ]; then
    echo "${red}error: copy config file failed, no file named: ${config_file}${reset}"
    exit 1
fi

model_file="detect-"
model_file+=$model_ver

cp models/$model_file crowd_firmware/detect &> /dev/null

if [ $? != 0 ]; then
    echo "${red}error: copy model file failed, no file named: ${model_file}${reset}"
    exit 1
fi


echo "Computing MD5 and generate ${green}${ver_file_name}${reset}"
firmware_md5=`md5sum ./${folder_name}/firmware | awk -F ' ' '{print $1}'`
conf_md5=`md5sum ./${folder_name}/config | awk -F ' ' '{print $1}'`
detect_md5=`md5sum ./${folder_name}/detect | awk -F ' ' '{print $1}'`
if [ $? != 0 ]; then
    echo "${red}error: compute MD5 failed${reset}"
    exit 1
fi

[ -z ${firmware_md5} ] && echo "${red}error: firmware md5 is empty${reset}" && exit 1
[ -z ${conf_md5} ] && echo "${red}error: conf md5 is empty${reset}" && exit 1
[ -z ${detect_md5} ] && echo "${red}error: model md5 is empty${reset}" && exit 1

echo "firmware  MD5 : ${green}${firmware_md5}${reset}"
echo "conf.json MD5 : ${green}${conf_md5}${reset}"
echo "detect    MD5 : ${green}${detect_md5}${reset}"


echo "firmware   : ${firmware_md5}" >> ./${folder_name}/${ver_file_name}
if [ $? != 0 ]; then
    echo "${red}error: can not write firmware md5 to ${ver_file_name}${reset}"
    exit 1
fi

echo "config     : ${conf_md5}" >> ./${folder_name}/${ver_file_name}
if [ $? != 0 ]; then
    echo "${red}error: can not write conf md5 to ${ver_file_name}${reset}"
    exit 1
fi

echo "detect     : ${detect_md5}" >> ./${folder_name}/${ver_file_name}
if [ $? != 0 ]; then
    echo "${red}error: can not write detect md5 to ${ver_file_name}${reset}"
    exit 1
fi

echo "version    : ${VERSION}" >> ./${folder_name}/${ver_file_name}
if [ $? != 0 ]; then
    echo "${red}error: can not write version to ${ver_file_name}${reset}"
    exit 1
fi
tar_name=${VERSION}
tar_name+="_"
tar_name+=${model_ver}
tar_name+="_"
tar_name+=${store_name}
tar_name+=".tar"

echo "Creating tar file, Name: ${green}${tar_name}${reset}"
cd ./${folder_name} && \
    tar -cvf ../${tar_name} config detect firmware ${ver_file_name} &> /dev/null && \
    cd ..

if [ $? != 0 ]; then
    echo "${red}error: can not creat tar file${reset}"
    exit 1
fi

rm -rf ./${folder_name}
if [ $? != 0 ]; then
    echo "${red}error: remove ${folder_name} FAILED"
    exit 1
fi


tar_md5=`md5sum ./${tar_name} | awk -F ' ' '{print $1}'`
echo "${green}Congratulations, we are all done here.${reset}"
echo "The tar file's MD5 is: ${green}${tar_md5}${reset}"
echo "tar file location: `realpath ./${tar_name}`"
