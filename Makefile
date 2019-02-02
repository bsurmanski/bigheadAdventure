all:
	gcc *.c -lSDL -lSDL_mixer -lSDL_image -g

web:
	alias python2="python2.7"
	mkdir -p bin/web
	emcc *.c \
		-s USE_SDL=1 \
		-s USE_SDL_IMAGE=1 \
		--preload-file res \
		--use-preload-plugins \
		-o bin/web/index.html
		#-s SDL_IMAGE_FORMATS='["png"]'\
