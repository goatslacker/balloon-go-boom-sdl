all:
	@g++ -o Balloon-Go-Boom balloon.cpp -lSDL_image

clean:
	rm Balloon-Go-Boom
