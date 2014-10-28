CC = g++
CFLAGS = -g -Wall
LDFLAGS = -lavformat -lavcodec -lavutil -lasound `sdl2-config --cflags --libs` -lsfml-audio -lsfml-system -std=c++11

TARGET = audio

all: $(TARGET)

audio: main.o decode_audio.o sdl_audio.o sfml_audio.o alsa_audio.o ring_buffer.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o decode_audio.o alsa_audio.o sdl_audio.o sfml_audio.o ring_buffer.o $(LDFLAGS)

main.o: main.cpp ring_buffer.h sdl_audio.h sfml_audio.h alsa_audio.h
	$(CC) $(CFLAGS) -c main.cpp $(LDFLAGS)

decode_audio.o: decode_audio.cpp decode_audio.h ring_buffer.h
	$(CC) $(CFLAGS) -c decode_audio.cpp $(LDFLAGS)

alsa_audio.o: alsa_audio.cpp ring_buffer.h
	$(CC) $(CFLAGS) -c alsa_audio.cpp $(LDFLAGS)

sdl_audio.o: sdl_audio.cpp ring_buffer.h
	$(CC) $(CFLAGS) -c sdl_audio.cpp $(LDFLAGS)

sfml_audio.o: sfml_audio.cpp ring_buffer.h
	$(CC) $(CFLAGS) -c sfml_audio.cpp $(LDFLAGS)

ring_buffer.o: ring_buffer.cpp ring_buffer.h
	$(CC) $(CFLAGS) -c ring_buffer.cpp $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)

