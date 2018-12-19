all:
	gcc *.c -lSDL -lSDL_mixer -lSDL_image -g

osx:
	gcc *.c -g \
		-framework SDL2 -framework SDL2_mixer -framework SDL2_image \
		-I/Library/Frameworks/SDL2.framework/Headers \
		-I/Library/Frameworks/SDL2_mixer.framework/Headers \
		-I/Library/Frameworks/SDL2_image.framework/Headers
