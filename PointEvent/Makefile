LDFLAGS			:=  -lpthread
TARGET			:= pointEvent
INCLUDE += -I./BMP -I./FrameBuffer -I./Graphic2D -I./EventCtrl

STRIP           := ls
CFLAGS	+=
OBJS = \
	PointEvent.o\
	./BMP/bmp.o\
	./FrameBuffer/FrameBuffer.o\
	./Graphic2D/Graphic2D.o\
	./EventCtrl/InputEvent.o\


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

