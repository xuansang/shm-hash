CC = gcc -g
CPP = g++ -g
#CCFLAGS = -Wall -g -Iinclude  -L./lib  -lsqlite3 -DLOGGER 
#CCFLAGS =  -I./include -levent -Llib ./lib/libsqlite3.a  ./lib/libsenseEIV.a
#UKEY_CCFLAGS = -Wall -g -static -Iinclude -L./lib
#STATIC_LIB = ./lib/libsqlite3.a ./lib/libpthread.a ./lib/libsenseEIV.a
SRC_CPP1 = ns_shm_hash.c test_hash.c 
SRC_CPP2 = ns_shm_hash.c test_hash_client.c 

CCFLAGS = -I /usr/include -I./ -lsqlite3 
CPP_FLAGS = -I /usr/include -fpermissive
SRC_CPP3 = test_hash.c
#OBJS = UkeyDBOpr.o Elite4S_Ukey.o  goServer.o socketLib.o cJSON.o  logger.o  Msg_Opr.o 

all:clean  test_hash test_hash_client

# ukey_test:test_ukey.cpp Ukey.h Elite4S_Ukey.cpp  common_ukey.h 
# 		$(CC) $(UKEY_CCFLAGS) -o  ukey_test test_ukey.cpp Ukey.h Elite4S_Ukey.cpp  common_ukey.h ./lib/libsenseEIV.a
test_hash: 
	$(CC) $(SRC_CPP1) $(CCFLAGS) -o test_hash

test_hash_client:
	$(CC) $(SRC_CPP2) $(CCFLAGS) -o sqlite_test2

# goServer.o: goServer.cpp goServer.h
# 	$(CC) $(CCFLAGS) -c goServer.cpp

# socketLib.o: socketLib.cpp socketLib.h
# 	$(CC) $(CCFLAGS) -c socketLib.cpp

# logger.o: logger.cpp logger.h
# 	$(CC) $(CCFLAGS) -c logger.cpp

# cJSON.o: cJSON.c cJSON.h
# 	g++ $(CCFLAGS) -c cJSON.c

# Elite4S_Ukey.o: Ukey.h common_ukey.h Elite4S_Ukey.cpp
# 	$(CC) $(CCFLAGS) -c Elite4S_Ukey.cpp

# UkeyDBOpr.o: UkeyDBOpr.h UkeyDBOpr.cpp
# 	$(CC) $(CCFLAGS) -c UkeyDBOpr.cpp

# Msg_Opr.o: common.h Msg_Opr.cpp
# 	$(CC) $(CCFLAGS) -c Msg_Opr.cpp


.PHONY:clean all
clean:
	rm -f *.o sqlite_test sqlite_test2 test_hash

#g++ goServer.cpp Msg_Opr.cpp logger.cpp cJSON.c UkeyDBOpr.cpp Elite4S_Ukey.cpp -I./include -levent -Llib ./lib/libsqlite3.a  ./lib/libsenseEIV.a -o goServer


