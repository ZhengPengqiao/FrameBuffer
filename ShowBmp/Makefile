LDFLAGS			:= 
TARGET			:= showBmp
INCLUDE += 

STRIP           := ls
CFLAGS	+=
OBJS = \
	ShowBmp.o\

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -O3  -o $@ $^ $(LDFLAGS)
	$(STRIP) $@


%.o:%.cpp
	$(CXX) $(CFLAGS) -c -O3 -o $@ $< $ 

%.o:%.c
	$(CXX) $(CFLAGS) -c -O3 -o $@ $< $ 

clean:
	@rm -rf *.o $(TARGET) $(OBJS)

