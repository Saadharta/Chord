CC=gcc
OPT=-Wall -Werror -g
ARCHIVE_NAME = "Demiguel François-Etienne_CHORD.tgz"

STRUCT = chord_struct
CHORD = chord
CMD_CORE = chord_cmd_core
COMM = chord_comm

all : $(CHORD)

chord_struct.o : $(STRUCT).c
	$(CC) $(OPT) -c $(STRUCT).c -o $(STRUCT).o

chord.o : $(CHORD).c
	$(CC) $(OPT) -c $(CHORD).c -o $(CHORD).o

chord_cmd_core.o : $(CMD_CORE).c
	$(CC) -c $(CMD_CORE).c -o $(CMD_CORE).o

chord_comm.o : $(COMM).c
	$(CC) -c $(COMM).c -o $(COMM).o


$(CHORD) : $(STRUCT).o $(CMD_CORE).o $(COMM).o $(CHORD).o
	$(CC) $(OPT) $(STRUCT).o $(CMD_CORE).o $(COMM).o $(CHORD).o -o $(CHORD)

### Project management
archive: fclean
	tar zcvf ${ARCHIVE_NAME} --exclude=*.tgz *

clean :
	rm -rf *.o

fclean: clean
	rm -rf ${ARCHIVE_NAME}

.PHONY : all chord_struct.o chord.o chord_cmd_core.o chord_comm.o chord archive clean fclean
