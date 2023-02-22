# KallistiOS ##version##
#
# basic/threading/compiler_tls/Makefile
# (c)2001 Dan Potter
# (c)2023 Colton Pawielski
#

EXAMPLE_NAME=compiler_tls

all: rm-elf $(EXAMPLE_NAME).elf

include $(KOS_BASE)/Makefile.rules

OBJS = $(EXAMPLE_NAME).o

clean:
	-rm -f $(EXAMPLE_NAME).elf $(OBJS)

rm-elf:
	-rm -f $(EXAMPLE_NAME).elf

general_threading_test.elf: $(OBJS) 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(EXAMPLE_NAME).elf -ftls-model=local-exec $(KOS_START) $(OBJS) $(DATAOBJS) $(OBJEXTRA) $(KOS_LIBS) 

run: $(EXAMPLE_NAME).elf
	$(KOS_LOADER) $(EXAMPLE_NAME).elf

dist:
	-rm -f $(OBJS)
	$(KOS_STRIP) $(EXAMPLE_NAME).elf
