TARGET = my_db.out

$(TARGET): main.o dict.o
	gcc -g -Wall main.o dict.o -o $(TARGET)

main.o: src/main.c include/dict.h
	gcc -g -Wall -c src/main.c

dict.o: src/dict.c include/dict.h
	gcc -g -Wall -c src/dict.c

clean:
	rm -f *.o $(TARGET)