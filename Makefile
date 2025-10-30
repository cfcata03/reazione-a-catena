CC = gcc
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE -g
LDFLAGS =

# Targets
TARGETS = master atomo attivatore alimentazione

# Object files
SHARED_OBJ = shared.o config.o

all: $(TARGETS)

master: master.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

atomo: atomo.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

attivatore: attivatore.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

alimentazione: alimentazione.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o
	ipcs -m | grep $(USER) | awk '{print $$2}' | xargs -n 1 ipcrm -m 2>/dev/null || true
	ipcs -s | grep $(USER) | awk '{print $$2}' | xargs -n 1 ipcrm -s 2>/dev/null || true
	ipcs -q | grep $(USER) | awk '{print $$2}' | xargs -n 1 ipcrm -q 2>/dev/null || true

.PHONY: all clean
