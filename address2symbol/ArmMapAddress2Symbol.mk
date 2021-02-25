# Makefile

CXX = g++
LD = g++

CXXFLAGS += -DBUILD_ARM_MAP_ADDRESS2SYMBOL

DEBUG ?= YES

TARGET ?= ArmMapAddress2Symbol

ifeq ($(DEBUG),YES)
CXXFLAGS += -g 
LDFLAGS += -g
endif

OBJS := ArmMapAddress2Symbol.o CommonAddress2Symbol.o

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(OBJS) $(TARGET)