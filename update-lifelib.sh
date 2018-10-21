#!/bin/bash
if [ -d ".git" ]
then
if [ -d "lifelib/avxlife" ]
then
printf "Ensuring lifelib is up-to-date...\n"
printf "import lifelib\nlifelib.reset_tree()\n" | python
else
printf "\033[33;1mDownloading lifelib...\033[0m\n"
fi

git submodule update --init
else
printf "Not a git repository; skipping updates...\n"
fi
