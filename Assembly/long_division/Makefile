CC = gcc
AS = nasm
ASFLAGS = -f elf64 -w+all -w+error -g
CFLAGS = -Wall -Wextra -std=c17 -g
LDFLAGS = -z noexecstack -g


all: mdiv_example

mdiv_example: mdiv.o mdiv_example.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.e *.o mdiv_example