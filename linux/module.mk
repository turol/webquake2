sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


SUBDIRS:= \
	# empty line

DIRS:=$(addprefix $(d)/,$(SUBDIRS))

$(eval $(foreach directory, $(DIRS), $(call directory-module,$(directory)) ))


FILES:= \
	net_udp.c \
	sys_linux.c \
	# empty line


SRC_$(d):=$(addprefix $(d)/,$(FILES))


SRC_client+=$(addprefix $(d)/,snd_linux.c vid_menu.c vid_so.c)

ref_gl_SRC:=$(addprefix $(d)/, gl_sdl.c qgl_linux.c)

SRC_shlinux:=$(addprefix $(d)/,q_shlinux.c glob.c)


d  := $(dirstack_$(sp))
sp := $(basename $(sp))
