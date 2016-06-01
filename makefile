# Global parameters
GCC = g++.exe
#FLAGS = -Wall -fmessage-length=0 -I..\includes -O2 -DDEBUG
FLAGS = -Wall -fmessage-length=0 -I..\includes -g -DDEBUG
TARGET = bin\alphachess.exe

all: $(TARGET)

# Create target application
$(TARGET): obj\main.o obj\alphachess.o obj\chessai.o obj\chessboard.o obj\chessgame.o obj\aboutdialog.o obj\savedgamesdialog.o obj\inputdialog.o obj\localgamedialog.o obj\networkgamedialog.o obj\preferencesdialog.o  obj\themesdialog.o obj\gameclient.o obj\capturepanel.o obj\chatpanel.o obj\chessboardpanel.o obj\historypanel.o obj\roompanel.o obj\playerpanel.o res\resources.res
	$(GCC) -mwindows -o $@ $^ -l ws2_32

#Resources
res\resources.res: res\resources.rc
	windres.exe --input-format=rc -O coff -o $@ -i $<

#Root
obj\main.o: src\main.cpp
	$(GCC) $(FLAGS) -o $@ -c $<

obj\alphachess.o: src\alphachess.cpp src\alphachess.h src\theme.h src\resource.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\gameclient.o: src\gameclient.cpp src\gameclient.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Chess
obj\chessai.o: src\chess\chessai.cpp src\chess\chessai.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\chessboard.o: src\chess\chessboard.cpp src\chess\chessboard.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\chessgame.o: src\chess\chessgame.cpp src\chess\chessgame.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Dialogs
obj\aboutdialog.o: src\dialogs\aboutdialog.cpp src\dialogs\aboutdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\savedgamesdialog.o: src\dialogs\savedgamesdialog.cpp src\dialogs\savedgamesdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\inputdialog.o: src\dialogs\inputdialog.cpp src\dialogs\inputdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\localgamedialog.o: src\dialogs\localgamedialog.cpp src\dialogs\localgamedialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\networkgamedialog.o: src\dialogs\networkgamedialog.cpp src\dialogs\networkgamedialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\preferencesdialog.o: src\dialogs\preferencesdialog.cpp src\dialogs\preferencesdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\promotiondialog.o: src\dialogs\promotiondialog.cpp src\dialogs\promotiondialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\themesdialog.o: src\dialogs\themesdialog.cpp src\dialogs\themesdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Panels
obj\capturepanel.o: src\panels\capturepanel.cpp src\panels\capturepanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\chatpanel.o: src\panels\chatpanel.cpp src\panels\chatpanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\chessboardpanel.o: src\panels\chessboardpanel.cpp src\panels\chessboardpanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\historypanel.o: src\panels\historypanel.cpp src\panels\historypanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\roompanel.o: src\panels\roompanel.cpp src\panels\roompanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\playerpanel.o: src\panels\playerpanel.cpp src\panels\playerpanel.h
	$(GCC) $(FLAGS) -o $@ -c $<

# Clean targets
clean:
	del obj\*.o
	del res\*.res
	del $(TARGET)
