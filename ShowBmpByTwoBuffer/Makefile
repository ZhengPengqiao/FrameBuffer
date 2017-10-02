LDFLAGS			:=  -lpthread
TARGET			:= showBmpByTwoBuffer
INCLUDE += -I./BMP

STRIP           := ls
CFLAGS	+=
OBJS = \
	ShowBmpByTwoBuffer.o\
	./BMP/bmp.o\

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -O3  -o $@ $^ $(LDFLAGS) $(INCLUDE)
	$(STRIP) $@


%.o:%.cpp
	$(CXX) $(CFLAGS) -c -O3 -o $@ $< $ $(INCLUDE)

%.o:%.c
	$(CXX) $(CFLAGS) -c -O3 -o $@ $< $ $(INCLUDE)

clean:
	@rm -rf *.o $(TARGET) $(OBJS)

