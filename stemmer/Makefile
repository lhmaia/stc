CC = gcc

CFLAGS = -Wall -g

RM = rm -f

OBJS = stem.o rslpStemmer.o rslpTrie.o

MAIN = stem

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
