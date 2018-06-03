#!/usr/bin/env bash
#  Copyright 2018 Rice University
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  ========================================================================    

pem_file=$1
user=ubuntu
ip_len_valid=3
testSSHTimeout=3

if [[ ! -v PDB_INSTALL ]]; then
   pdb_dir="/tmp/pdb_install"
else
   pdb_dir=$PDB_INSTALL
fi

usage() {
    echo ""
    echo -e "\033[33;31m""    "Warning: This script deletes PlinyCompute stored data in remote""
    echo -e "    "machines in a cluster, use with care!"\e[0m"
    echo ""

    cat <<EOM

    Description: This script installs a distributed instance of PlinyCompute by
    copying the required executables and scripts to the nodes in a cluster. It 
    first cleans the contents of an installation of PlinyCompute in folder 
    '$pdb_dir' (if it exists).

    Usage: scripts/$(basename $0) <param1>

           param1: <pem_file>
                      Specify the private key to connect to other machines in
                      the cluster; the default is conf/pdb-key.pem

EOM
   exit -1;
}

[ -z $1 ] && { usage; } || [[ "$@" = *--help ]] && { usage; } || [[ "$@" = *-h ]] && { usage; }

if [ ! -f ${pem_file} ]; then
    echo -e "Pem file ""\033[33;31m""'$pem_file'""\e[0m"" not found, make sure the path and file name are correct!"
    exit -1;
fi

echo -e "\033[33;31m""Before installing PlinyCompute, this script deletes all PlinyCompute stored data, use it carefully!""\e[0m"
echo -e "PlinyCompute default installation path has been set to: ""\033[33;32m""$pdb_dir""\e[0m"

read -p "Do you want to delete all PlinyCompute stored data?i [Y/n] " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo "Installation process cancelled. All previous stored data remained unchanged."
    [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
fi

scripts/cleanupNode.sh force

# By default disable strict host key checking
if [ "$PDB_SSH_OPTS" = "" ]; then
  PDB_SSH_OPTS="-o StrictHostKeyChecking=no"
fi

if [ -z ${pem_file} ];
then
  PDB_SSH_OPTS=$PDB_SSH_OPTS
else
  PDB_SSH_OPTS="-i ${pem_file} $PDB_SSH_OPTS"
fi

echo $PDB_HOME/conf/serverlist

while read line
do
   [[ $line == *#* ]] && continue # skips commented lines
   [[ ! -z "${line// }" ]] && arr[i++]=$line # include only non-empty lines
done < $PDB_HOME/conf/serverlist

length=${#arr[@]}
echo "There are $length servers defined in $PDB_HOME/conf/serverlist"

for (( i=0 ; i<=$length ; i++ ))
do
   ip_addr=${arr[i]}
   if [ ${#ip_addr} -gt "$ip_len_valid" ]
   then
      # checks that ssh to a node is possible, times out after 3 seconds
      nc -w $testSSHTimeout $ip_addr 22
      if [ $? -eq 0 ]
      then
         echo -e "\n+++++++++++ install worker node at IP: $ip_addr"
         ssh $PDB_SSH_OPTS $user@$ip_addr "rm -rf $pdb_dir; mkdir $pdb_dir; mkdir $pdb_dir/bin; mkdir  $pdb_dir/logs; mkdir $pdb_dir/scripts; mkdir $pdb_dir/scripts/internal"
         scp $PDB_SSH_OPTS -r $PDB_HOME/bin/pdb-worker $user@$ip_addr:$pdb_dir/bin/ 
         scp $PDB_SSH_OPTS -r $PDB_HOME/scripts/cleanupNode.sh $PDB_HOME/scripts/stopWorker.sh $user@$ip_addr:$pdb_dir/scripts/
         scp $PDB_SSH_OPTS -r $PDB_HOME/scripts/internal/checkProcess.sh $PDB_HOME/scripts/internal/startWorker.sh $user@$ip_addr:$pdb_dir/scripts/internal
         ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; scripts/cleanupNode.sh force"
      else
         echo -e "Connection to ""\033[33;31m""IP: ${ip_addr}""\e[0m"", failed. Files were not installed."
      fi
   fi
done


