sbin_PROGRAMS = sldog 

DEBUG_FLAGS = -ggdb
INCLUDES = -I$(top_srcdir)/include
AM_CFLAGS = $(DEBUG_FLAGS) -Wall @DB_CFLAGS@

sldog_SOURCES = main.c getpid.c ipparse.c errors.c config.c
sldog_LDFLAGS = @DB_LIBS@ 
