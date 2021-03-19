# sselp - simple print selection

SRC = simcrop.cpp
OBJ = ${SRC:.c=.o}

all: simcrop.cpp
	g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_tracking -o "simcrop" "simcrop.cpp"

clean:
	@echo cleaning
	@rm -f simcrop
	@rm -f /usr/local/bin/simcrop

install: all
	@echo installing executable file to /usr/local/bin
	@mkdir -p /usr/local/bin
	@cp -f simcrop /usr/local/bin
	@chmod 755 /usr/local/bin/simcrop

uninstall:
	@echo removing executable file from /usr/local/bin
	@rm -f /usr/local/bin/simcrop
