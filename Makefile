PWD := $(shell pwd)
RM := rm -rf
ECHO := echo

SRC := src
BIN := bin
OBJ := obj
INC := include
ETC := etc

COMMON_OBJECTS := obj/libevquick.o obj/libvsb_frame.o

SERVER_OBJECTS := obj/server.o obj/libvsb_server.o
CLIENT_OBJECTS := obj/client.o obj/libvsb_client.o

CFLAGS := -Wall -g3 -Wextra -Wno-unused-parameter -c -I$(INC)
LDFLAGS := 
LIBS := 

default: all
all: $(BIN)/server $(BIN)/client

$(BIN)/server: $(SERVER_OBJECTS) $(COMMON_OBJECTS)
	@$(ECHO) "[Link]" server
	@mkdir -p $(@D)
	@$(CC) $(SERVER_OBJECTS) $(COMMON_OBJECTS) $(LIBS) $(LDFLAGS) -o $@

$(BIN)/client: $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
	@$(ECHO) "[Link]" client
	@mkdir -p $(@D)
	@$(CC) $(CLIENT_OBJECTS) $(COMMON_OBJECTS) $(LIBS) $(LDFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@$(ECHO) [Compile] $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	@$(ECHO) "[Cleanup]"
	@$(RM) $(BIN)/*
	@$(RM) $(OBJ)/*
	
install: default
	$(ECHO) "[Install]"
