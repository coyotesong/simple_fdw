#!/bin/sh

# script to rename all of blackhole to something else
# the name should be lower case and not end in _fdw (the script adds _fdw
# where needed)

newname=$1

if test -z "$newname"
then
   echo usage: $0 newname
   exit 1;
fi

if expr "$newname" : ".*_fdw" > /dev/null
then
   echo "remove _fdw from new name - script will use it where appropriate"
   exit 1
fi


Newname=`perl -e "print qq(\\u$newname);"`


grep --exclude-dir=.git --exclude=newname.sh -rl simple . | xargs sed -i -e "s/simple/$newname/g"
grep --exclude-dir=.git --exclude=newname.sh -rl Simple . | xargs sed -i -e "s/Simple/$Newname/g"

mv simple_fdw.control ${newname}_fdw.control
mv src/simple_fdw.c src/${newname}_fdw.c
mv doc/simple_fdw.md doc/${newname}_fdw.md
mv sql/simple_fdw.sql sql/${newname}_fdw.sql
mv test/sql/simple.sql test/sql/${newname}.sql
mv test/expected/simple.out test/expected/${newname}.out





