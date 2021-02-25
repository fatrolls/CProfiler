# Makefile

# CXXFLAGS += -DNO_SYMBOL
# CXXFLAGS += -DNO_HASH_MAP

CXXFLAGS += -I../

DEBUG ?= YES

TARGET ?= program_with_profiler

ifeq ($(DEBUG),YES)
CXXFLAGS += -DDEBUG_PROFILER 
CXXFLAGS += -g 
LDFLAGS += -g
endif

PROFILER_OBJS = $(ROOT_DIR)/profiler/profiler.o $(ROOT_DIR)/address2symbol/CommonAddress2Symbol.o

$(OBJS) : CXXFLAGS += -finstrument-functions

$(TARGET): $(PROFILER_OBJS) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(OBJS) $(PROFILER_OBJS) $(TARGET)