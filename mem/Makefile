ser=server
cli=client
cc=gcc

serSrc=server.c
cliSrc=client.c

.PHONY:all
all:$(ser) $(cli)

$(ser):$(serSrc)
	$(cc) -o $@ $^
$(cli):$(cliSrc)
	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm -f $(ser) $(cli)

