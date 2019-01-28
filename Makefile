all:
	gcc *.c -lSDL -lSDL_mixer -lSDL_image -g

osx:
	gcc *.c -g \
		-framework SDL2 -framework SDL2_mixer -framework SDL2_image \
		-I/Library/Frameworks/SDL2.framework/Headers \
		-I/Library/Frameworks/SDL2_mixer.framework/Headers \
		-I/Library/Frameworks/SDL2_image.framework/Headers -o bin/Bighead

	mkdir -p bin/Bighead.app/Contents/MacOS
	mkdir -p bin/Bighead.app/Contents/Resources
	echo "APPL????" > bin/Bighead.app/Contents/PkgInfo
	mv bin/Bighead bin/Bighead.app/Contents/MacOS/Bighead
	cp -r res/* bin/Bighead.app/Contents/Resources

web:
	alias python2="python2.7"
	mkdir -p bin/web/
	emcc *.c -o bin/web/index.html \
		-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'\
		-s USE_SDL_MIXER=2 \
		-s USE_OGG=1 \
		--preload-file res\
		-I/Library/Frameworks/SDL2.framework/Headers \
		-I/Library/Frameworks/SDL2_mixer.framework/Headers \
		-I/Library/Frameworks/SDL2_image.framework/Headers
