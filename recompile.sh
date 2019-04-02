#!/bin/bash
set -e

chmod 755 *.sh

# Ensures 'make' works properly:
rm -f ".depend" | true
rm -f "main.o" | true
make clean

# Ensures compilation will fail unless rule2asm succeeds:
rm -f "includes/params.h" | true

updatearg=`echo "$@" | grep -o "\\-\\-update" | sed "s/\\-\\-update/u/"`
profilearg=`echo "$@" | grep -o "\\-\\-profile" | sed "s/\\-\\-profile/u/"`
mingwarg=`echo "$@" | grep -o "\\-\\-mingw" | sed "s/\\-\\-mingw/u/"`
gpuarg=`echo "$@" | grep -o "\\-\\-cuda" | sed "s/\\-\\-cuda/u/"`
immarg=`echo "$@" | grep -o "\\-\\-immediate" | sed "s/\\-\\-immediate/u/"`
rulearg=`echo "$@" | grep -o "\\-\\-rule [^ ]*" | sed "s/\\-\\-rule\\ //"`
symmarg=`echo "$@" | grep -o "\\-\\-symmetry [^ ]*" | sed "s/\\-\\-symmetry\\ //"`

if ((${#symmarg} != 0)); then
if [ "${symmarg:0:1}" = "G" ]; then
gpuarg="true"
elif [ "${symmarg:0:1}" = "H" ]; then
gpuarg="true"
fi
fi

if ((${#mingwarg} != 0)); then
export USE_MINGW=1
fi

if ((${#profilearg} != 0)); then
if ((${#gpuarg} != 0)); then
printf "\033[31;1mWarning: --cuda and --profile are incompatible; omitting the latter.\033[0m\n"
else
export PROFILE_APGLUXE=1
fi
fi

if ((${#updatearg} != 0)); then

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

if [ "$1" = "lifecoin" ]; then
    rulearg="b3s23"
    symmarg="C1"
    export LIFECOIN=1
    printf "Compiling lightweight \033[32;1mlifecoin\033[0m executable...\n"
    executable="./lifecoin search"
else
    printf "Compiling \033[32;1mapgluxe\033[0m without lifecoin support...\n"
    executable="./apgluxe"
fi

if ((${#rulearg} == 0)); then
rulearg="b3s23"
echo "Rule unspecified; assuming b3s23."
fi

if ((${#symmarg} == 0)); then
symmarg="C1"
echo "Symmetry unspecified; assuming C1."
fi

gpuarg2="false"

if ((${#gpuarg} != 0)); then
export USE_GPU=1

gpuarg2="true"
fi

echo "Configuring rule $rulearg; symmetry $symmarg"

if command -v "python3" &>/dev/null; then
    echo "Using $(which python3) to configure lifelib..."
    python3 mkparams.py $rulearg $symmarg $gpuarg2
else
    echo "Using $(which python) to configure lifelib..."
    python mkparams.py $rulearg $symmarg $gpuarg2
fi

make
if ((${#mingwarg} != 0)); then
exit 0
fi

symmarg="$( grep 'SYMMETRY'   'includes/params.h' | grep -o '".*"' | tr '\n' '"' | sed 's/"//g' )"
rulearg="$( grep 'RULESTRING' 'includes/params.h' | grep -o '".*"' | tr '\n' '"' | sed 's/"//g' )"

if ((${#immarg} != 0)); then
    $executable --rule $rulearg --symmetry $symmarg "$@"
else
    $executable --rule $rulearg --symmetry $symmarg
fi

exit 0
