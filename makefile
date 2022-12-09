URL            ?= github.com/jcog/fp
ifeq ($(origin FP_VERSION), undefined)
  TAG_COMMIT     := $(shell git rev-list --abbrev-commit --tags --max-count=1)
  TAG            := $(shell git describe --abbrev=0 --tags ${TAG_COMMIT} 2>/dev/null || true)
  COMMIT         := $(shell git rev-parse --short HEAD)
  DATE           := $(shell git log -1 --format=%cd --date=format:"%m-%d-%y")
  FP_VERSION     := $(COMMIT)-$(DATE)
  ifeq ('$(TAG_COMMIT)', '$(COMMIT)')
    ifneq ('$(TAG)', '')
      FP_VERSION := $(TAG)
    endif
  endif
endif
CC             = mips64-gcc
LD             = mips64-g++
AS             = mips64-gcc -x assembler-with-cpp
OBJCOPY        = mips64-objcopy
GRC            = grc
GENHOOKS       = CPPFLAGS='$(subst ','\'', $(CPPFLAGS))' ./genhooks
SRCDIR         = src
BUILDDIR       = build
LIBDIR         = lib
RESDIR         = res
RESDESC        = $(RESDIR)/resources.json
OBJDIR         = $(BUILDDIR)/obj/$(VERSION)
BINDIR         = $(BUILDDIR)/bin/$(VERSION)
HOOKSDIR       = $(BUILDDIR)/patch/$(VERSION)
OUTDIR         = $(OBJDIR) $(OBJDIR)/$(RESDIR) $(BINDIR) $(HOOKSDIR)
ELF            = $(BINDIR)/fp.elf
BIN            = $(BINDIR)/fp.bin
LDR_ELF        = $(BINDIR)/ldr.elf
LDR_BIN        = $(BINDIR)/ldr.bin
HOOKS          = $(HOOKSDIR)/hooks.gsc
NDEBUG        ?= 0
VERSION       ?= jp

FP_BIN_ADDRESS = 0x80400060
CFLAGS         = -c -MMD -MP -std=gnu11 -Wall -ffunction-sections -fdata-sections -O2 -fno-reorder-blocks
ALL_CPPFLAGS   = -DURL=$(URL) -DFP_VERSION=$(FP_VERSION) -DF3DEX_GBI_2 $(CPPFLAGS)
ALL_LDFLAGS    = -T gl-n64.ld -L$(LIBDIR) -nostartfiles -specs=nosys.specs -Wl,--gc-sections $(LDFLAGS)

ifeq ($(NDEBUG), 1)
  CFLAGS       += -DNDEBUG
  ALL_CPPFLAGS += -DNDEBUG
endif

ifeq ($(VERSION), us)
  ALL_CPPFLAGS += -DPM64_VERSION=US
  ALL_LDFLAGS  += -Wl,-Map=$(BUILDDIR)/fp-us.map
  LIBS          = -lpm-us
else ifeq ($(VERSION), jp)
  ALL_CPPFLAGS += -DPM64_VERSION=JP
  ALL_LDFLAGS  += -Wl,-Map=$(BUILDDIR)/fp-jp.map
  LIBS          = -lpm-jp
else
  $(error VERSION must be either us or jp)
endif

C_FILES   := $(wildcard $(SRCDIR)/*.c)
RES_FILES := $(wildcard $(RESDIR)/fp/*.png)
OBJ       := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(C_FILES))))) \
             $(addprefix $(OBJDIR)/$(RESDIR)/, $(addsuffix .o, $(notdir $(basename $(RES_FILES)))))

.PHONY: fp ldr clean
fp: $(BIN) $(HOOKS)
ldr: $(LDR_BIN)
clean:
	rm -rf $(BUILDDIR)
	rm -f fp-jp.z64 fp-us.z64 fp-US.wad fp-JP.wad romc
romc: romc.c
	gcc -O2 $< -o $@

$(HOOKS): $(ELF) | $(HOOKSDIR)
	$(GENHOOKS) $< > $@

$(BIN): $(ELF) | $(BINDIR)
	$(OBJCOPY) -S -O binary $< $@

$(ELF): $(OBJ) | $(BINDIR)
	$(LD) $(ALL_LDFLAGS) -Wl,--defsym,start=$(FP_BIN_ADDRESS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(ALL_CPPFLAGS) $(CFLAGS) -o $@ $<

$(OBJDIR)/$(RESDIR)/%.o: $(RESDIR)/fp/%.png $(RESDESC) | $(OBJDIR)/$(RESDIR)
	$(GRC) $< -d $(RESDESC) -o $@

$(LDR_BIN): $(LDR_ELF) | $(BINDIR)
	$(OBJCOPY) -S -O binary $< $@

$(LDR_ELF): $(SRCDIR)/asm/ldr.s | $(BINDIR)
	$(AS) -MMD -MP $(ALL_CPPFLAGS) $(ALL_LDFLAGS) $< -o $@ $(LIBS)

$(OUTDIR):
	@mkdir -p $@
