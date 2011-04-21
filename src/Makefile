CC = g++

CFLAGS = -Wall -g

RM = rm -f

OBJS = main.o reader.o stree.o STC.o usuario.o

MAIN = stc

$(MAIN): $(OBJS)
	@echo ""
	@echo " --- COMPILANDO PROGRAMA ---"
	@$(CC) $(CFLAGS) $(OBJS) -lm -o $(MAIN)
	@echo ""

%.o: %.c %.h
	@echo " --- COMPILANDO OBJETO \"$@\""
	@$(CC) $(CFLAGS) $< -c 

clean:
	$(RM) $(MAIN) *o
	clear

run: $(MAIN)
	./$(MAIN)

