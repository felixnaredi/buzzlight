debug: main.c buzzlight.c
	gcc $^ -o $@.o -DDEBUG -g

buzzlight: main.c buzzlight.c
	gcc $^ -o $@ -O3
