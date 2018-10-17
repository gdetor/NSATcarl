#!/bin/bash

host=bahamut
dest_base=~/CARLsim/projects/nsat_bs/

echo "Now synchronizing NSAT files with $host"
rsync -avz --progress --exclude '.*.swp' src gdetorak@$host:$dest_base
rsync -avz --progress --exclude '.*.swp' include gdetorak@$host:$dest_base
rsync -avz --progress --exclude '.*.swp' params gdetorak@$host:$dest_base
rsync -avz --progress --exclude '.*.swp' bin gdetorak@$host:$dest_base
rsync -avz --progress --exclude '.*.swp' tools gdetorak@$host:$dest_base
rsync -avz --progress --exclude '.*.swp' Makefile gdetorak@$host:$dest_base

echo "Synchronization is Ok!"
