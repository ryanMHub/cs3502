CC	:= gcc
CFLAGS	:= -Wall -Wextra -pthread

PHASES	:= phase1 phase2 phase3 phase4 phase4_timeout

.PHONY: all clean

all: $(PHASES)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PHASES)
