CC = g++ -std=c++20
INCLUDES = -I"./Include" -I"../VulkanCore/Include" 
CFLAGS = $(INCLUDES) -lglfw3 -lvulkan-1  -lglslang-default-resource-limits \
  -lglslang \
  -lSPIRV \
  -lOSDependent \
  -lMachineIndependent \
  -lGenericCodeGen \
  -lSPVRemapper \
  -lpthread 
MAIN_FILE = a.cpp 
FILES = $(MAIN_FILE) $(wildcard ../VulkanCore/Source/*.cpp) $(wildcard ../VulkanCore/Common/*.cpp) $(wildcard ./Source/*.cpp)
BUILDED_FILE = a.exe

run:
	$(CC) $(FILES) -o $(BUILDED_FILE) $(CFLAGS)
	./$(BUILDED_FILE)

# log:
# 	echo $(FILES)