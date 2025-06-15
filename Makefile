CC = g++ 
CXXFLAGS = -std=c++20 -Wall -Wextra -I pong/Include -I VulkanCore/Include
LDFLAGS = -lglfw3 -lvulkan-1 -lglslang-default-resource-limits -lglslang -lSPIRV -lOSDependent -lMachineIndependent -lGenericCodeGen -lSPVRemapper -lpthread

MAIN_FILE = $(wildcard pong/*.cpp)
FILES = $(MAIN_FILE) $(wildcard VulkanCore/Source/*.cpp) $(wildcard VulkanCore/Common/*.cpp) $(wildcard pong/Source/*.cpp)
BUILDED_FILE = a.exe

OBJ_DIR = build
TARGET = bin/$(BUILDED_FILE)
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(FILES))
DEPS = $(OBJS:.o=.d)

all: $(TARGET)
	@echo "Build completo. Executando..."
	@./$(TARGET)

$(TARGET): $(OBJS)
	$(shell mkdir $(dir $@) 2>NUL || exit 0)
	$(CC) $^ -o $@ $(LDFLAGS)


$(OBJ_DIR)/%.o: %.cpp
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -rf $(OBJ_DIR) $(BIN_DIR)
	

# run:
# 	$(CC) $(FILES) -o $(BUILDED_FILE) $(CFLAGS)
# 	./$(BUILDED_FILE)

# log:
# 	echo $(FILES)

-include $(DEPS)