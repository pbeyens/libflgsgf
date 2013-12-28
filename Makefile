CXX      = g++
DEBUG    = -g
CXXFLAGS = -Werror -Wall -I.
LDFLAGS  = -shared
LINK     = $(CXX)

TARGET = libflgsgf.so
OBJS = sgf.o
SRCS = sgf.c

.SUFFIXES: .o .c

all: $(OBJS)
	$(LINK) $(LDFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CXX) $(CXXFLAGS) $(DEBUG) -c $< -o $@

install: ${TARGET}
	sudo cp ${TARGET} /usr/local/lib	

clean:
	rm -f ${OBJS} 2> /dev/null
	rm -f $(TARGET) 2> /dev/null
