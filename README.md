This program searches random initial configurations in Conway's Game
of Life and periodically uploads results to a remote server. You can
read more information about the distributed search at the following URL:

- https://catagolue.hatsya.com/

An automatic live Twitter feed of new discoveries found by the search
was established by Ivan Fomichev:

- https://twitter.com/conwaylife

There is also an automatically-updated summary page, with animations
of interesting objects and various charts:

- https://catagolue.hatsya.com/statistics

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

    apgluxe v5.0-ll2.2.0: Rule b3s23 is correctly configured.
    apgluxe v5.0-ll2.2.0: Symmetry C1 is correctly configured.

which means you are ready to run the program like so:

    ./apgluxe [OPTIONS]

The options may include, for example:

- `-k mypassword`      Upload soups where 'mypassword' is your key
- `-n 5000000`         Run 5000000 soups per upload
- `-p 4`               Parallelise across 4 threads
- `--rule b36s245`     Run the custom rule B36/S245
- `--symmetry D2_+1`   Run soups with odd bilateral symmetry
- `-L 1`               Save a local log of each haul
- `-t 1`               Disable uploading to Catagolue
- `-i 10`              Upload exactly 10 hauls before exiting

Example usage
-------------

This invocation will upload results every 20 million soups:

    ./apgluxe -n 20000000

If you want to upload soups non-anonymously, use the -k flag and
provide a valid payosha256 key. The correct syntax is as follows:

    ./apgluxe -n 20000000 -k mykey

where 'mykey' is replaced with your payosha256 key (available from
https://catagolue.hatsya.com/payosha256 -- note the case-sensitivity).
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

    git pull

in the repository directory.

Windows users (precompiled)
---------------------------

There is a precompiled Windows binary, only for `b3s23/C1`, available from
[here](https://catagolue.hatsya.com/binaries/apgluxe-windows-x86_64.exe).
When executed, it will prompt you for the haul size, number of CPUs to use,
and your [payosha256 key](https://catagolue.hatsya.com/payosha256). For
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

Speed boosts
============

There are several compilation flags that can be used for accelerated
searching.

GPU searching
-------------

If you have an NVIDIA GPU with at least 1.5 GB of memory, then it can be
used as a 'preprocessor' which discards uninteresting soups and delegates
the interesting soups to the CPU search program. Compilation uses:

    ./recompile.sh --cuda

Note that the program will upload to a different census (**b3s23/G1** instead
of **b3s23/C1**) as the process of discarding uninteresting soups heavily
distorts the census results. Work is in progress to allow the GPU to census
soups itself, thereby allowing CUDA-accelerated searching of **b3s23/C1**.

It will default to using the whole GPU (device 0, unless you explicitly
change `CUDA_VISIBLE_DEVICES` as explained in the next section) as a soup
preprocessor and 8 CPU threads to census the interesting soups. If you
have a very powerful GPU and it is under-utilised, you may want to increase
the number of CPU threads using the `-p` flag:

    ./apgluxe -n 1000000000 -p 12 -k mykey

This is particularly pertinent for the NVIDIA Volta V100 and RTX 2080 Ti
GPUs, which can each manage 1 080 000 soups per second if sufficiently many
CPU threads are being used. The Ampere A100 should theoretically be able to
manage considerably more than that, provided the number of threads is high
enough.

You can see the CPU usage using `htop` and the GPU usage using:

    watch -n 0.1 nvidia-smi

If the GPU utilisation is significantly below 100% for a significant amount
of time, then increasing the number of threads is recommended.

Multi-GPU searching
-------------------

Each process can only utilise a single GPU. The `CUDA_VISIBLE_DEVICES`
environment variable allows you to select the GPU to use (this is a standard
part of the CUDA runtime, and not a feature of apgsearch specifically). For
example, to run a process on GPU 3, use:

    CUDA_VISIBLE_DEVICES=3 ./apgluxe [OPTIONS]

This requires you to have firstly compiled with `./recompile.sh --cuda` as
described in the previous section.

To search on multiple GPUs, run one process on each GPU. You might want to
create a Bash script resembling the following (with one line per device) so
that you can conveniently run a process per GPU:

    #!/bin/bash
    KEY="insert_your_key_here"
    CPU_THREADS=8
    CUDA_VISIBLE_DEVICES=0 ./apgluxe -p "$CPU_THREADS" -n 1000000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=1 ./apgluxe -p "$CPU_THREADS" -n 1100000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=2 ./apgluxe -p "$CPU_THREADS" -n 1200000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=3 ./apgluxe -p "$CPU_THREADS" -n 1300000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=4 ./apgluxe -p "$CPU_THREADS" -n 1400000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=5 ./apgluxe -p "$CPU_THREADS" -n 1500000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=6 ./apgluxe -p "$CPU_THREADS" -n 1600000000 -k "$KEY" &
    CUDA_VISIBLE_DEVICES=7 ./apgluxe -p "$CPU_THREADS" -n 1700000000 -k "$KEY" &
    wait

**Warning:** The output to the terminal may look confusing and bizarre
because it contains the interleaved output from all of these processes.
This is to be expected and is no cause for concern.

Profile-guided optimisation (CPU only)
--------------------------------------

Profile-guided optimisation can be enabled with:

    ./recompile.sh --profile

This is only supported for the compilers GCC and Clang, rather than nvcc,
so is specific to CPU searching. The benefits are relatively mild.

Credits and licences
====================

The software `apgluxe` and `lifelib` are both written by Adam P. Goucher and
available under an MIT licence. Thanks go to Dave Greene, Tom Rokicki, 'Apple
Bottom', Darren Li, Arthur O'Dwyer, and Tod Hagan for contributions,
suggestions, testing, and feedback.

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
