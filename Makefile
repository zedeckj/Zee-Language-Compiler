CC = cc
CFLAGS = -w 
prefix =
LDFLAGS = 

zeec: src/main.c src/compiler.c src/linearizer.c src/scanner.c src/tokenizer.c 
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@
