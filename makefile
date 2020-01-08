run: lib/core.o lib/strategy.o lib/graphics.o
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE master.c lib/core.o lib/strategy.o lib/graphics.o -o master
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE gamer.c lib/core.o lib/strategy.o -o gamer
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE pawn.c lib/core.o lib/strategy.o -o pawn
	clear
	./master

compile: lib/core.o lib/strategy.o lib/graphics.o
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE master.c lib/core.o lib/strategy.o lib/graphics.o -o master
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE gamer.c lib/core.o lib/strategy.o -o gamer
	gcc -std=c89 -pedantic -D_POSIX_SOURCE -D_SVID_SOURCE pawn.c lib/core.o lib/strategy.o -o pawn

clean:
	rm -f *.o
	rm -f lib/*.o
