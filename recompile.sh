#!/bin/bash
set -e

chmod 755 "recompile.sh"

# Ensures 'make' works properly:
rm -f ".depend" | true
rm -f "main.o" | true
make clean

# Ensures compilation will fail unless rule2asm succeeds:
rm -f "includes/params.h" | true

rulearg=`echo "$@" | grep -o "\\-\\-rule [^ ]*" | sed "s/\\-\\-rule\\ //"`
symmarg=`echo "$@" | grep -o "\\-\\-symmetry [^ ]*" | sed "s/\\-\\-symmetry\\ //"`
updatearg=`echo "$@" | grep -o "\\-\\-update" | sed "s/\\-\\-update/u/"`
profilearg=`echo "$@" | grep -o "\\-\\-profile" | sed "s/\\-\\-profile/u/"`
mingwarg=`echo "$@" | grep -o "\\-\\-mingw" | sed "s/\\-\\-mingw/u/"`
gpuarg=`echo "$@" | grep -o "\\-\\-cuda" | sed "s/\\-\\-cuda/u/"`

if ((${#mingwarg} != 0)); then
export USE_MINGW=1
fi

if ((${#profilearg} != 0)); then
export PROFILE_APGLUXE=1
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

# Ensure lifelib matches the version in the repository:
bash update-lifelib.sh
rm -rf "lifelib/avxlife/lifelogic" | true

launch=0

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

if ((${#gpuarg} != 0)); then
export USE_GPU=1
echo "Overriding rule and symmetry with b3s23/G1."
symmarg="G1"
rulearg="b3s23"
fi

echo "Configuring rule $rulearg; symmetry $symmarg"

python mkparams.py $rulearg $symmarg
make
if ((${#mingwarg} != 0)); then
exit 0
fi

newrule="$( grep 'RULESTRING' 'includes/params.h' | grep -o '".*"' | tr '\n' '"' | sed 's/"//g' )"

if [ "$launch" = "1" ]; then
    ./apgluxe --rule $newrule "$@"
else
    ./apgluxe --rule $newrule --symmetry $symmarg
fi

exit 0
