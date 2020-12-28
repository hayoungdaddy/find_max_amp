#!/bin/sh

for file in `ls *`
do

sac <<EOF
read $file
write sac binary/$file
quit
EOF

done

