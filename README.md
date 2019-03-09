This program searches random initial configurations in Conway's Game
of Life and periodically uploads results to a remote server. You can
read more information about the distributed search at the following URL:

- https://catagolue.appspot.com/

An automatic live Twitter feed of new discoveries found by the search
was established by Ivan Fomichev:

- https://twitter.com/conwaylife

There is also an automatically-updated summary page, with animations
of interesting objects and various charts:

- https://catagolue.appspot.com/statistics

The search was originally performed by people running instances of
a Python script; this repository contains the source code for a C++
program which is 20 times faster. The prefix 'apg-' stands for _Ash
Pattern Generator_ and the suffix '-luxe' refers to the capabilities
vis-a-vis previous versions (apgmera, apgnano, and apgsearch).

Compilation and execution
=========================

**Note:** apgluxe can only run on **x86-64** machines. If you have an
ancient computer, then the recommended alternative is the Python
script.

This program is designed to be compiled using `gcc` or `clang`. If you
have one of these compilers installed, then building apgluxe is as
simple as running:

    bash recompile.sh

in the repository directory. If compilation succeeded, the last two
lines should resemble the following:

    apgluxe v4.984-ll2.1.21: Rule b3s23 is correctly configured.
    apgluxe v4.984-ll2.1.21: Symmetry C1 is correctly configured.

which means you are ready to run the program like so:

    ./apgluxe [OPTIONS]

The options may include, for example:

- **-k mypassword**      Upload soups where 'mypassword' is your key
- **-n 5000000**         Run 5000000 soups per upload
- **-p 4**               Parallelise across 4 threads
- **--rule b36s245**     Run the custom rule B36/S245
- **--symmetry D2_+1**   Run soups with odd bilateral symmetry

Example usage
-------------

This invocation will upload results every 20 million soups:

    ./apgluxe -n 20000000

If you want to upload soups non-anonymously, use the -k flag and
provide a valid payosha256 key. The correct syntax is as follows:

    ./apgluxe -n 20000000 -k mykey

where 'mykey' is replaced with your payosha256 key (available from
https://catagolue.appspot.com/payosha256 -- note the case-sensitivity).
Omitting this parameter will cause soups to be uploaded anonymously.

If you have a quad-core computer and would prefer not to run four
separate instances, then use the -p command to parallelise:

    ./apgluxe -n 20000000 -k mykey -p 4

This will use C++11 multithreading to parallelise across 4 threads, thus
producing and uploading soups approximately four times more quickly. Note
that this does not work on Cygwin.

Installation
============

Linux / Mac OS X users
----------------------

Compiling and running apgluxe is easy, as explained above. To download
the source code, use the following command:

    git clone https://gitlab.com/apgoucher/apgmera.git

Then you can enter the directory and compile the search program using:

    cd apgmera
    ./recompile.sh

If the online repository is updated at all, you can update your local
copy in-place by running:

    ./recompile.sh --update

in the repository directory.

Windows users (precompiled)
---------------------------

There is a precompiled Windows binary, only for `b3s23/C1`, available from
[here](https://catagolue.appspot.com/binaries/apgluxe-windows-x86_64.exe).
When executed, it will prompt you for the haul size, number of CPUs to use,
and your [payosha256 key](https://catagolue.appspot.com/payosha256). For
finer control, it can be run from the Command Prompt with any combination
of the options mentioned in the Example Usage above.

Compiling from source has the advantage of allowing other rules and
symmetries to be explored. Moreover, it allows certain optimisations to
be applied to specifically target the machine you're using, conferring
a marginal speed boost.

Windows users (pre-Windows 10, Cygwin)
--------------------------------------

Install Cygwin64 (from https://cygwin.com), ensuring that the following
are checked in the list of plugins to install:

 - git
 - make
 - gcc-g++
 - python (2 or 3)

Open a Cygwin terminal, which will behave identically to a Linux terminal
but run inside Windows. This reduces your problem to the above case.

If you get the error `stoll is not a member of std`, then you are using an
old version of GCC. Run the Cygwin setup program to ensure that gcc-g++ is
updated.

Note that the `-p` option for parallelisation does not work in Cygwin.

Windows 10 users
----------------

Even though the Cygwin64 solution above will work perfectly on Windows 10,
[one user](https://gitlab.com/hedgepiggy) noted that it does not fully
utilise the processor. Instead, you are encouraged to use WSL bash as
described [here](https://gitlab.com/apgoucher/apgmera/issues/2).

It seems that the order of magnitude difference quoted above is on the
extreme side; other users report [a 20 percent difference][1] between
WSL, VirtualBox, and Cygwin64 (in descending order of speed).

[1]: http://conwaylife.com/forums/viewtopic.php?f=7&t=3049&p=61174#p61174

Moreover, these comparisons were performed with the old threading model
(OpenMP threads), whereas apgluxe has subsequently migrated to pure C++11
threads for increased cross-platform support.

Credits and licences
====================

The software `apgluxe` and `lifelib` are both written by Adam P. Goucher and
available under an MIT licence. Thanks go to Dave Greene, Tom Rokicki, 'Apple
Bottom', Darren Li and Tod Hagan for contributions, suggestions, testing, and
feedback.

All third-party components are similarly free and open-source:

 - 'CRYSTALS-Dilithium: A Lattice-Based Digital Signature Scheme' is Licensed
   under Creative Commons License CC-BY 4.0;
 - The SHA3 (Keccak) hash function implementation, by Dr. Markku-Juhani O.
   Saarinen, is available under an MIT licence;
 - The SHA-256 hash function implementation, by Olivier Gay, is available
   under a BSD 3-clause licence;
 - The 'RSA Data Security, Inc. MD5 Message-Digest Algorithm' reference
   implementation can be copied, modified and used under the condition
   that the copyright notice is included;
 - The 'HappyHTTP' library, by Ben Campbell, can be copied, modified and
   used under the condition that the copyright notice is included.
