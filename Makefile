TARGET=_target_sdcc_nrf24le1_32
CCFLAGS=-Isdk/include --std-c99 -I sdk/$(TARGET)/include/ --opt-code-size
LDFLAGS= -Lsdk/$(TARGET)/lib -lnrf24le1
PROGRAMS = main.ihx
SOURCES = ${PROGRAMS:ihx=c}
LIBNRF = sdk/$(TARGET)/lib/nrf24le1.lib

all: .deps ${PROGRAMS}

-include .deps

.deps: ${SOURCES}
	sdcc $(CCFLAGS) -M $(SOURCES) > .deps.tmp
	sed -i -e 's/.rel:/.ihx:/' .deps.tmp
	mv .deps.tmp .deps

${PROGRAMS}: ${SOURCES} $(LIBNRF)
	sdcc --model-large $(CCFLAGS) $(LDFLAGS) main.c

$(LIBNRF):
	make -C sdk all

clean:
	rm -rf  main.asm  main.cdb  main.ihx  main.lk  main.lst  main.map  main.mem  main.omf  main.rel  main.rst  main.sym .deps

.PHONY: all clean
