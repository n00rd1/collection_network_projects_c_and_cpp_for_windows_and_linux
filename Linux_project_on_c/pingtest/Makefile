CC:=gcc
#CFLAGS=
#LDFLAGS=
#LIBS+=

release: clean
	$(CC) $(CFLAGS) -Wall -Werror=format-security -s pingtest.c -o pingtest $(LDFLAGS) $(LIBS)

clean:
	rm -rf at *~

