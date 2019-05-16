CC			= gcc
CFLAGS		= -Wall
OBJFILES	= ConnectionHandler.o CommandHandler.o CommandLineHandler.o HttpHeaderInterpreter.o JsonOutput.o URIDecoder.o
TARGET		= wrc-server

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)
	
clean:
	rm -f $(OBJFILES) $(TARGET) *~