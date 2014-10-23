CC = g++
CFLAGS = -g -Wall
LDFLAGS = -lavformat -lavcodec -lavutil `sdl2-config --cflags --libs` -std=c++11

TARGET = audio

all: $(TARGET)

audio: main.o decode_audio.o ring_buffer.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o decode_audio.o ring_buffer.o $(LDFLAGS)

main.o: main.cpp ring_buffer.h sdl_audio.h
	$(CC) $(CFLAGS) -c main.cpp $(LDFLAGS)

decode_audio.o: decode_audio.cpp decode_audio.h
	$(CC) $(CFLAGS) -c decode_audio.cpp $(LDFLAGS)

ring_buffer.o: ring_buffer.cpp ring_buffer.h
	$(CC) $(CFLAGS) -c ring_buffer.cpp $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)

