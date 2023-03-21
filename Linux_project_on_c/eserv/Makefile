CC:=gcc
#CFLAGS=
#LDFLAGS=
#LIBS+=

release: clean
	$(CC) $(CFLAGS) -Wall -Werror=format-security -s main.c -o eserv $(LDFLAGS) $(LIBS)

clean:
	rm -rf at *~

