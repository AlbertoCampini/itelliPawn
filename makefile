run: lib/core.o lib/strategy.o
	gcc master.c lib/core.o lib/strategy.o -o master
	gcc gamer.c lib/core.o lib/strategy.o -o gamer
	gcc pawn.c lib/core.o -o pawn
	clear
	./master

compile: lib/core.o lib/strategy.o
	gcc master.c lib/core.o lib/strategy.o -o master
	gcc gamer.c lib/core.o lib/strategy.o -o gamer
	gcc pawn.c lib/core.o -o pawn

clean:
	rm -f *.o
	rm -f lib/*.o
