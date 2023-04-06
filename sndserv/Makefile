# make CC=clang CC_SIZE=llvm-size ELF_PATCH=morello_elf SYS_HEADERS=<path>/morello-linux-headers/usr/include/ MUSL_HOME=<sysroot>

CC ?=
CC_SIZE ?=
SYS_HEADERS ?=
MUSL_HOME ?=
ELF_PATCH ?=

TARGET=aarch64-linux-musl_purecap
CFLAGS+=-c -g -Os -march=morello+c64 \
		--target=$(TARGET) --sysroot $(MUSL_HOME) \
		-I $(SYS_HEADERS)
LDFLAGS+=-fuse-ld=lld -march=morello+c64 \
		-Wl,--gc-sections --target=$(TARGET) \
		--sysroot $(MUSL_HOME) -rtlib=compiler-rt -static
CFLAGS+=-Wall -DMORELLO -DNORMALUNIX -DLINUX -DSNDSERV
LIBS+=-lm -lc

# subdirectory for objects
OBJDIR=../bin
OUTPUT=$(OBJDIR)/morello-doom-sndserv

DOOM_SRC = soundsrv.o sounds.o wadread.o linux.o
OBJS += $(addprefix $(OBJDIR)/, $(DOOM_SRC))

all:	 $(OUTPUT)

clean:
	rm -rf $(OBJDIR)
	rm -f $(OUTPUT)
	rm -f $(OUTPUT).gdb
	rm -f $(OUTPUT).map

$(OUTPUT):	$(OBJS)
	@echo [Linking $@]
	@$(CC) $(LDFLAGS) $(OBJS) \
	-o $(OUTPUT) $(LIBS) -Wl,-Map,$(OUTPUT).map
	@echo [Patch $(OUTPUT)]
	@$(ELF_PATCH) $(OUTPUT)
	@echo [Clean $(OBJDIR)]
	@rm -fr $(OBJDIR)/*.o
	@echo [Size]
	@$(CC_SIZE) $(OUTPUT)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	%.c
	@echo [Compiling $<]
	@$(CC) $(CFLAGS) -c $< -o $@

print:
	@echo OBJS: $(OBJS)