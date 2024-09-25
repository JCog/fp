#!/usr/bin/env python3

import argparse
import subprocess
from ninja_syntax import Writer
from io import StringIO
from pathlib import Path


def run(cmd: str) -> str:
    return subprocess.run(cmd, capture_output=True, shell=True, check=False).stdout.decode()


parser = argparse.ArgumentParser(description="Creates an fp build script")
parser.add_argument("--cflags", default=[], help="CFLAGS to be used for the build", action="append", nargs="+")
parser.add_argument("--cppflags", default=[], help="CPPFLAGS to be used for the build", action="append", nargs="+")
parser.add_argument("--ldflags", default=[], help="LDFLAGS to be used for the build", action="append", nargs="+")
parser.add_argument("--version", type=str, help="fp version to be displayed on the title screen")
parser.add_argument("--ndebug", action="store_true", help="Disables debug logging")

args = parser.parse_args()

VERSIONS = ["us", "jp"]

CC = "mips64-gcc"
LD = "mips64-g++"
AS = "mips64-gcc -x assembler-with-cpp"
OBJCOPY = "mips64-objcopy"
GRC = "grc"
GENHOOKS = "./genhooks"
SRCDIR = "src"
BUILDDIR = "build"
LIBDIR = "lib"
RESDIR = "res"
RESDESC = f"{RESDIR}/resources.json"
OBJDIR = f"{BUILDDIR}/obj"
BINDIR = f"{BUILDDIR}/bin"
HOOKSDIR = f"{BUILDDIR}/patch"
HOOKS = "hooks.gsc"

if args.version:
    FP_VERSION = args.version
else:
    tag_commit = run("git rev-list --abbrev-commit --tags --max-count=1").rstrip()
    tag = run(f"git describe --abbrev=0 --tags {tag_commit} 2>/dev/null || true").rstrip()
    commit = run("git rev-parse --short HEAD").rstrip()
    date = run('git log -1 --format=%cd --date=format:"%m-%d-%y"').rstrip()
    FP_VERSION = f"{commit}-{date}"
    if tag_commit == commit and tag != "":
        FP_VERSION = tag

FP_BIN_ADDRESS = "0x80400060"
CFLAGS = f"-c -std=gnu11 -Wall -ffunction-sections -fdata-sections -O2 -fno-reorder-blocks -fdiagnostics-color -Isrc {' '.join([i for l in args.cflags for i in l])}"
CPPFLAGS = f"-DURL=github.com/jcog/fp -DFP_VERSION={FP_VERSION} -DF3DEX_GBI_2 {' '.join([i for l in args.cppflags for i in l])}"
LDFLAGS = f"-T gl-n64.ld -L{LIBDIR} -nostartfiles -specs=nosys.specs -Wl,--gc-sections {' '.join([i for l in args.ldflags for i in l])}"

if args.ndebug:
    CFLAGS += " -DNDEBUG"
    CPPFLAGS += " -DNDEBUG"

outbuf = StringIO()
n = Writer(outbuf)
n.variable("ninja_required_version", "1.3")
n.newline()

n.variable("cc", CC)
n.variable("ld", LD)
n.variable("as", AS)
n.variable("grc", GRC)
n.variable("objcopy", OBJCOPY)
n.variable("resdesc", RESDESC)
n.variable("fp_bin_addr", FP_BIN_ADDRESS)
n.variable("genhooks", GENHOOKS)
n.variable("cflags", CFLAGS)
n.variable("ldflags", LDFLAGS)
n.newline()

n.rule(
    "cc",
    command="$cc $cppflags $cflags -MMD -MF $out.d $in -o $out",
    description="CC $in",
    deps="gcc",
    depfile="$out.d",
)

n.rule(
    "ld",
    command="$ld $ldflags -Wl,--defsym,start=$fp_bin_addr -Wl,-Map=$map $in -o $out $libs",
    description="LD $out",
)

n.rule("grc", command="$grc $in -d $resdesc -o $out", description="GRC $in")

n.rule("as", command=f"$as {CPPFLAGS} $ldflags $in -o $out", description="AS $in")

n.rule("objcopy", command="$objcopy -S -O binary $in $out", description="OBJCOPY $in -> $out")

n.rule("genhooks", command="$genhooks $in > $out", description="GENHOOKS $in")

n.rule("sys_cc", command="gcc -O2 $in -o $out", description="GCC $in")

n.rule("clean", command=f"rm -rf {BUILDDIR} fp-jp.z64 fp-us.z64 fp-US.wad fp-JP.wad romc")

n.newline()


class CFile:
    def __init__(self, ver: str, path: str):
        self.cppflags = CPPFLAGS + " " + f"-DPM64_VERSION={ver.upper()}"
        self.path = path
        self.o_path = path.replace(SRCDIR, f"{OBJDIR}/{ver.lower()}").replace(".c", ".o")

    def build(self) -> None:
        n.build(self.o_path, rule="cc", inputs=self.path, variables={"cppflags": self.cppflags})


class ResFile:
    def __init__(self, ver: str, path: str):
        self.path = path
        self.o_path = f"{OBJDIR}/{ver.lower()}/{RESDIR}/{Path(path).stem}.o"

    def build(self) -> None:
        n.build(self.o_path, rule="grc", inputs=self.path)


for ver in VERSIONS:
    BIN = f"{BINDIR}/{ver}/fp.bin"
    ELF = f"{BINDIR}/{ver}/fp.elf"
    LDR_BIN = f"{BINDIR}/{ver}/ldr.bin"
    LDR_ELF = f"{BINDIR}/{ver}/ldr.elf"

    files = []
    for c_file in Path(SRCDIR).rglob("*.c"):
        f = CFile(ver, str(c_file))
        files.append(f)
        f.build()

    for res_file in Path(RESDIR).rglob("*.png"):
        f = ResFile(ver, str(res_file))
        files.append(f)
        f.build()

    n.build(
        ELF,
        rule="ld",
        inputs=[f.o_path for f in files],
        variables={"libs": f"-lpm-{ver}", "map": f"{BUILDDIR}/fp-{ver}.map"},
    )

    n.build(f"{HOOKSDIR}/{ver}/{HOOKS}", rule="genhooks", inputs=ELF)

    n.build(BIN, rule="objcopy", inputs=ELF, implicit=f"{HOOKSDIR}/{ver}/{HOOKS}")

    n.build(ver, "phony", BIN)

    n.build(LDR_ELF, rule="as", inputs=f"{SRCDIR}/asm/ldr.s")

    n.build(LDR_BIN, rule="objcopy", inputs=LDR_ELF)

    n.build(f"{ver}_ldr", "phony", LDR_BIN)

n.build("clean", rule="clean")
n.build("romc", rule="sys_cc", inputs="romc.c")

with open("build.ninja", "w") as f:
    f.write(outbuf.getvalue())
n.close()
