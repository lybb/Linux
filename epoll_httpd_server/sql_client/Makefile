ROOT_PATH=$(shell pwd)
INCLUDE_PATH=-I$(ROOT_PATH)/../lib/include
LIB_PATH=-L$(ROOT_PATH)/../lib/lib
lib=-lmysqlclient
CC=g++

.PHONY:all
all:cgi_insert cgi_delete cgi_select cgi_update
	cp cgi_insert $(ROOT_PATH)/../wwwroot/cgi_bin/
	cp cgi_delete $(ROOT_PATH)/../wwwroot/cgi_bin/
	cp cgi_select $(ROOT_PATH)/../wwwroot/cgi_bin/
	cp cgi_update $(ROOT_PATH)/../wwwroot/cgi_bin/

#sql_api:sql_api.cpp
#	$(CC) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(lib)

cgi_insert:sql_insert_cgi.cpp sql_api.cpp
	$(CC) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(lib)
cgi_delete:sql_delete_cgi.cpp sql_api.cpp
	$(CC) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(lib)
cgi_select:sql_select_cgi.cpp sql_api.cpp
	$(CC) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(lib)
cgi_update:sql_update_cgi.cpp sql_api.cpp
	$(CC) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(lib)
.PHONY:clean
clean:
	rm -f cgi_insert cgi_delete cgi_select cgi_update;cd ../wwwroot/cgi_bin/; rm -f cgi_insert cgi_delete cgi_select cgi_update;cd -
