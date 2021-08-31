#!/bin/bash
# get the absolute path of config_environment.sh
conf_abs_path="$(cd `dirname ${BASH_SOURCE[0]}`;pwd -P)"

echo "Configure environment variables for XEYE-MX"
export MV_TOOLS_DIR=${conf_abs_path}/xos/tools/
echo "Configure MV_TOOLS_DIR: $MV_TOOLS_DIR"

