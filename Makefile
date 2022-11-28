CFLAGS = -O3 -Wall -Werror -I src/include -pthread 
SRCS_SERIAL_PACKET = src/utils/* src/serial/* 
SRCS_PARALLEL_PACKET = src/utils/* src/parallel/* src/queue/* src/locks/*
LM_FLAG = -lm

serial_packet: $(SRCS) ./src/include/* ./src/serial/*
	gcc $(CFLAGS) -o serial_packet $(SRCS_SERIAL_PACKET) $(LM_FLAG)

parallel_packet: $(SRCS) ./src/include/* ./src/parallel/* ./src/queue/* 
	gcc $(CFLAGS) -o parallel_packet $(SRCS_PARALLEL_PACKET) $(LM_FLAG)


