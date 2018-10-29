#!/bin/bash
set -e

chmod 755 "recompile.sh"

# Ensures 'make' works properly:
rm -f ".depend" | true
rm -f "main.o" | true

# Ensures compilation will fail unless rule2asm succeeds:
rm -f "includes/params.h" | true

# --------------------------------

updatearg=`echo "$@" | grep -o "\\-\\-update" | sed "s/\\-\\-update/u/"`

if ((${#updatearg} != 0))
then

printf "Checking for updates from repository...\033[30m\n"
newversion=`curl "https://gitlab.com/apgoucher/apgmera/raw/master/main.cpp" | grep "define APG_VERSION" | sed "s/#define APG_VERSION //"`
oldversion=`cat main.cpp | grep "define APG_VERSION" | sed "s/#define APG_VERSION //"`
if [ "$newversion" != "$oldversion" ]
then
printf "\033[0m...your copy of apgluxe does not match the repository.\n"
echo "New version: $newversion"
echo "Old version: $oldversion"
bash update-lifelib.sh
git pull
else
printf "\033[0m...your copy of apgluxe is already up-to-date.\n"
fi
else
echo "Skipping updates; use --update to update apgluxe automatically."
fi

# --------------------------------

# Ensure lifelib matches the version in the repository:
bash update-lifelib.sh
rm -rf "lifelib/avxlife/lifelogic" | true

launch=0

if [ "$1" = "lifecoin" ]; then
    rulearg="b3s23"
    symmarg="C1"
    export LIFECOIN=1
    if [ "$2" = "server" ]; then
        export FULLNODE=1
        printf "Compiling full \033[32;1mlifecoin-server\033[0m executable...\n"
    else
        printf "Compiling lightweight \033[32;1mlifecoin\033[0m executable...\n"
    fi
    launch=2
else
    printf "Compiling \033[32;1mapgluxe\033[0m without lifecoin support...\n"
    rulearg=`echo "$@" | grep -o "\\-\\-rule [a-z0-9-]*" | sed "s/\\-\\-rule\\ //"`
    symmarg=`echo "$@" | grep -o "\\-\\-symmetry [a-zA-Z0-9_+]*" | sed "s/\\-\\-symmetry\\ //"`

    if ((${#rulearg} == 0))
    then
    rulearg="b3s23"
    echo "Rule unspecified; assuming b3s23."
    else
    launch=1
    fi

    if ((${#symmarg} == 0))
    then
    symmarg="C1"
    echo "Symmetry unspecified; assuming C1."
    else
    launch=1
    fi
fi

echo "Configuring rule $rulearg; symmetry $symmarg"

python mkparams.py $rulearg $symmarg
make

if (($launch == 1))
then
./apgluxe "$@"
elif (($launch == 0))
then
./apgluxe --rule $rulearg --symmetry $symmarg
fi

exit 0
