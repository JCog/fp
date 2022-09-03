## Latest Builds
For an up-to-date development build of fp without the need to build from source, click on the latest workflow run on the [ci page](https://github.com/JCog/fp/actions/workflows/build-patches.yml) and download the patch for the version you want under the Artifacts header

# Prerequisites
If you want to build fp on Windows, you'll have to install the Windows Subsystem for Linux (WSL). Instructions for this can be found: [here](https://docs.microsoft.com/en-us/windows/wsl/install)

fp requires glank's n64 toolchain to compile and build. If you're on a debian-based linux distribution, you can install a prebuilt package of the toolchain by running:

    sudo sh -c '(curl https://practicerom.com/public/packages/debian/pgp.pub || wget -O - https://practicerom.com/public/packages/debian/pgp.pub) | gpg --dearmor -o /usr/share/keyrings/practicerom.gpg && echo deb [signed-by=/usr/share/keyrings/practicerom.gpg] https://practicerom.com/public/packages/debian staging main > /etc/apt/sources.list.d/practicerom.list && apt update'
    sudo apt install practicerom-dev

For those on other operating systems or who want to build from source, follow the instructions on [the n64 repository](https://github.com/glankk/n64) (note that building the toolchain from scratch can take a good amount of time. The prebuilt package is recommended if you're able to use it)


# Building
Clone the fp source code by running:

`git clone https://github.com/jcog/fp.git`  

To build all fp binaries, run `make` in the root directory of the fp repository.
If you only want to patch a rom, you can skip this step and move on to the patching section.

# Patching
## N64
To create a patched ROM, run

    ./makerom <rom-file>

replacing `<rom-file>` with the path to a unmodified (and 100% legally obtained) Mario Story (J) or Paper Mario (U) ROM.

If you do not specifiy an output rom with `-o <output-rom>`, the newly built fp rom will be located in the root directory as either `fp-jp.z64` or `fp-us.z64`.

## Wii
To create a patched WAD for use with Wii VC, you must have gzinject installed. If you followed the above instructions to install the prebuilt toolchain, this will already be installed. If not, follow the instructions [here](https://github.com/krimtonz/gzinject). You will also need to generate the wii common key by running `gzinject -a genkey` in the root directory of the repository and following the instructions.

To patch a WAD, run

    ./makewad <wad>

A Paper Mario or Mario Story ROM can be provided with the `-m` flag. Otherwise, the patcher will use the ROM included in the provided WAD. An output WAD name can be specified with the `-o` flag. If one is not provided, the patched WAD will either be called `fp-JP.wad` or `fp-US.wad`.

It is also possible to pass arguments to gzinject by including them in the `makewad` arguments, though the defaults should work fine for most people.

## Wii U
To inject into the Wii U VC emulator, first download and install Phacox's Injector from [here](https://github.com/phacoxcll/PhacoxsInjector/releases). You will also need a dumped copy of the Virtual Console game. To inject a fp ROM:
- Select a fp ROM by clicking the "Choose" button in the injector window and selecting the ROM file
- Choose the Virtual Console base game to inject into by clicking the "Load base" button and selecting the folder that contains the game that you dumped
- Type a short name for the injected game
- Go to the "Injecting" menu on the left side of the window and either click the "Do not pack" button if you will be using loadiine or "Do pack" if you will be using WUP Installer
- Select the folder you want the injected game to be placed in and wait until it has finished