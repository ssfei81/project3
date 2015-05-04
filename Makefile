CC = g++
CFLAGS = -Wall -lpthread -std=c++0x

TARGET = router
TARGET2 = source
TARGET3 = destination


all: $(TARGET) $(TARGET2) $(TARGET3)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET)

$(TARGET2): $(TARGET2).cpp
	$(CC) $(CFLAGS) $(TARGET2).cpp -o $(TARGET2)

$(TARGET3): $(TARGET3).cpp
	$(CC) $(CFLAGS) $(TARGET3).cpp -o $(TARGET3)

clean:
	$(RM) $(TARGET) $(TARGET2) $(TARGET3)
