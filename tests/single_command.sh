#!/bin/sh

#Single Command tests
echo "Testing Single Commands:"
echo hello world
ls -a

echo "Testing command with a multiple flags and different flag types"
ls -l -a -B -d
git status
echo 1 + 1
mkdir test_folder
rm -r test_folder
touch test_file
rm test_file
cat README.md
echo ls
echo git status