LDFLAGS			:= 
TARGET			:= showTime
INCLUDE += 

STRIP           := ls
CFLAGS	+=
OBJS = \
	ShowTime.o\

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

