LDFLAGS			:= 
TARGET			:= saveFBData
INCLUDE += 

STRIP           := ls
CFLAGS	+=
OBJS = \
	SaveFBData.o\

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

