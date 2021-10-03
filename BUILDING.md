## Latest Builds
For an up-to-date development build of fp without the need to build from source, click on the latest workflow run on the [ci page](https://github.com/jcog/fp/actions) and download the patch for the version you want under the Artifacts header

# Prerequisites
If you want to build fp on Windows, you'll have to install the Windows Subsystem for Linux (WSL). Instructions for this can be found: [here](https://docs.microsoft.com/en-us/windows/wsl/install)

fp requires glank's n64 toolchain to compile and build. If you're on a debian-based linux distribution, you can install a prebuilt package of the toolchain by running:

    sudo sh -c '(curl https://practicerom.com/public/packages/debian/pgp.pub || wget -O - https://practicerom.com/public/packages/debian/pgp.pub) | apt-key add - && echo deb https://practicerom.com/public/packages/debian ./staging main > /etc/apt/sources.list.d/practicerom.list && apt update'
    sudo apt install practicerom-dev

For those on other operating systems or who want to build from source, follow the instructions on [the n64 repository](https://github.com/glankk/n64) (note that building the toolchain from scratch can take a good amount of time. The prebuilt package is recommended if you're able to use it)


# Building
Clone the fp source code by running:

`git clone https://github.com/jcog/fp.git`  

To build all fp binaries, run `make LDFLAGS=' -Wl,--defsym,start=0x80400060'` in the root directory of the fp repository.
If you only want to patch a rom, you can skip this step and move on to the patching section.

# Patching
To create a patched ROM, run

    build/makerom <rom-file>

replacing `<rom-file>` with the path to a unmodified (and 100% legally obtained) Mario Story (J) or Paper Mario (U) ROM.

If you do not specifiy an output rom with `-o <output-rom>`, the newly built fp rom will be located in the build directory as either `fp-PM64J.z64` or `fp-PM64U.z64`.
