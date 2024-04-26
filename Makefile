#OBJS specifies which files to compile as part of the project
OBJS = main.cpp PlayBoard.cpp Tetromino.cpp logic.cpp rendering.cpp Texture.cpp Player.cpp audio.cpp settings.cpp onlan.cpp icon.o

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = tetris

#This is the target that compiles our executable
all : $(OBJS)

	g++ $(OBJS) -I src\include\SDL2 -L src\lib -L src\bin -w -Wl,-subsystem,console -lmingw32 -lwsock32 -lWs2_32 -liphlpapi -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2 -o $(OBJ_NAME)
    # g++ -I src\include\SDL2 -L src\lib -o main icon.o -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf