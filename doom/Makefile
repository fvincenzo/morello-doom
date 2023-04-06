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
CFLAGS+=-Wall -DMORELLO -DNORMALUNIX -DLINUX -DSNDSERV -DUNDEF_STD_KEYS
LIBS+=-lm -lc

OBJS+=$(OBJDIR)/i_video_fbdev.o
OBJS+=$(OBJDIR)/i_input_tty.o

# subdirectory for objects
OBJDIR=../bin
OUTPUT=$(OBJDIR)/morello-doom

DOOM_SRC = i_main.o dummy.o am_map.o doomdef.o doomstat.o dstrings.o d_event.o d_items.o d_iwad.o d_loop.o d_main.o d_mode.o d_net.o f_finale.o f_wipe.o g_game.o hu_lib.o hu_stuff.o info.o i_cdmus.o i_endoom.o i_joystick.o i_scale.o i_sound.o i_system.o i_timer.o memio.o m_argv.o m_bbox.o m_cheat.o m_config.o m_controls.o m_fixed.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o r_things.o sha1.o sounds.o statdump.o st_lib.o st_stuff.o s_sound.o tables.o v_video.o wi_stuff.o w_checksum.o w_file.o w_file_stdc_unbuffered.o w_main.o w_wad.o z_zone.o
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