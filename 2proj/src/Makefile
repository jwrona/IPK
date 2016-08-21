#Makefile
#author: Jan Wrona, xwrona00@stud.fit.vutbr.cz
#projet #2, client, server
#lecture: IPK

CL_NAME = client
SE_NAME = server
CL_FNAME = clientFunc
SE_FNAME = serverFunc

CPP = g++
#CPP = clang++
CPPFLAGS = -pedantic -Wall -Wextra
OFLAG = -o
CFLAG = -c
.SUFFIXES: .cpp .o

.cpp.o:
	$(CPP) $(CPPFLAGS) $(CFLAG) $<


all: client server

client: $(CL_NAME).o $(CL_FNAME).o
	$(CPP) $(OFLAG) $(CL_NAME) $(CL_NAME).o $(CL_FNAME).o

server: $(SE_NAME).o $(SE_FNAME).o
	$(CPP) $(OFLAG) $(SE_NAME) $(SE_NAME).o $(SE_FNAME).o

tar.gz:
	tar -cf xwrona00.tar $(CL_NAME).cpp $(CL_NAME).h $(CL_FNAME).cpp $(SE_NAME).cpp $(SE_NAME).h $(SE_FNAME).cpp Makefile protokol.pdf
	gzip xwrona00.tar

clean:
	rm -f *.o *~ $(CL_NAME) $(SE_NAME)
