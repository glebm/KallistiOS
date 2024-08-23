# Makefile for the adpcm program.

CFLAGS = -O2 -Wall #-g#
#LDFLAGS = -g

all: adpcm

adpcm: adpcm.c bs_codec.c oki_codec.c yma_codec.c ymb_codec.c ymz_codec.c

clean:
	-rm -f adpcm.o adpcm
