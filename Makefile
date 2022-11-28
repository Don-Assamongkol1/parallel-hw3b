CFLAGS = -O3 -Wall -Werror -I src/include -pthread 
SRCS_SERIAL_PACKET = src/utils/* src/serial/* 
LM_FLAG = -lm

serial_packet: $(SRCS) ./src/include/* ./src/serial/*
	gcc $(CFLAGS) -o serial_packet $(SRCS_SERIAL_PACKET) $(LM_FLAG)


