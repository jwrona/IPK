#Makefile
#author: Jan Wrona, xwrona00@stud.fit.vutbr.cz
#projet #1: webclient
#lecture: IPK

NAME = webclient
FNAME = webclFunc
CPP = g++
CPPFLAGS = -pedantic -Wall -Wextra
OFLAG = -o
CFLAG = -c
.SUFFIXES: .cpp .o

.cpp.o:
	$(CPP) $(CPPFLAGS) $(CFLAG) $<

all: $(NAME).o $(FNAME).o
	$(CPP) $(OFLAG) $(NAME) $(NAME).o $(FNAME).o

touch:
	touch $(NAME).cpp $(FNAME).cpp

compress:
	tar -cf xwrona00.tar $(NAME).cpp $(NAME).h $(FNAME).cpp Makefile
	gzip xwrona00.tar

clean:
	rm -f *.o *~ webclient
