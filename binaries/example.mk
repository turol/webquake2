# example of local configuration
# copy to local.mk


# location of source
TOPDIR:=..


LTO:=n
ASAN:=n
TSAN:=n
UBSAN:=n


# compiler options etc
CC:=gcc
CFLAGS:=-g -DNDEBUG -DLINUX
CFLAGS+=$(shell sdl-config --cflags)
OPTFLAGS:=-O2 -march=native -fno-strict-aliasing -ffloat-store


LDFLAGS:=-g
LDLIBS:=-lz -lm -ldl
LDLIBS_ref_gl:=-lGL -ljpeg -lpng $(shell sdl-config --libs)


SOCFLAGS+=-fPIC -DPIC


LTOCFLAGS:=-flto -fuse-linker-plugin -fno-fat-lto-objects
LTOLDFLAGS:=-flto -fuse-linker-plugin


OBJSUFFIX:=.o
EXESUFFIX:=-bin
SOSUFFIX:=.so
