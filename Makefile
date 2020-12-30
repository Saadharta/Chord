CC=gcc
OPT=-Wall -Werror
ARCHIVE_NAME = "Demiguel François-Etienne_CHORD"

STRUCT = chord_struct
CHORD = chord
SPLITTER = cmd_splitter

all : $(CHORD)

chord_struct.o : $(STRUCT).c
	$(CC) $(OPT) -c $(STRUCT).c -o $(STRUCT).o

chord.o : $(CHORD).c
	$(CC) $(OPT) -c $(CHORD).c -o $(CHORD).o

cmd_splitter.o : $(SPLITTER).c
	$(CC) -c $(SPLITTER).c -o $(SPLITTER).o

$(CHORD) : $(STRUCT).o $(SPLITTER).o $(CHORD).o
	$(CC) $(OPT) $(STRUCT).o $(SPLITTER).o $(CHORD).o -o $(CHORD)

### Project management
archive: fclean
	tar zcvf ${ARCHIVE_NAME} --exclude=*.tgz *

clean :
	rm -rf *.o

fclean: clean
	rm -rf ${ARCHIVE_NAME}

.PHONY : all chord_struct.o chord.o cmd_cplitter.o chord archive clean fclean
