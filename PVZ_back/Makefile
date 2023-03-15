#SDL2-----------------------------------------------------------------------------
SDL2_path = D:\Game_programming\SDL2
SDL2_include = $(SDL2_path)\include\SDL2
SDL2_lib = $(SDL2_path)\lib

cpps = \
	LTexture.cpp \
	peashooter.cpp \
	init.cpp \
	background.cpp \
	LTimer.cpp \
	icon.cpp \
	bullets.cpp

flags = \
	-lmingw32 \
	-lSDL2main \
	-lSDL2 \
	-lSDL2_image \
	-lSDL2_ttf \
	-lSDL2_mixer \
	-mwindows

game:
	g++ -I $(SDL2_include) -L $(SDL2_lib)  *.cpp -o pvz main.res $(flags) & pvz.exe
SDL2:
	g++ -I $(SDL2_include) -L $(SDL2_lib)  main.cpp $(cpps) -o main main.res $(flags) & main