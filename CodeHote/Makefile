GCC_PREFIX=
CFLAGS=-O3 -pipe

all:ServUDP
	@echo "[By MrAs2an]"

ServUDP: ServUDP.c
	$(GCC_PREFIX)-g++ $(CFLAGS) -o ServUDP.out ServUDP.c -ljpeg `pkg-config --cflags opencv` `pkg-config --libs opencv`

clean: 
	rm -f  ServUDP.out 
