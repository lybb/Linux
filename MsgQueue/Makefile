cli=client
ser=server
cc=gcc

cliSrc=client.c
serSrc=server.c

.PHONY:all
all:$(cli) $(ser)

$(cli):$(cliSrc)
	$(cc) -o $@ $^
$(ser):$(serSrc)
	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm -f $(cli) $(ser)
