CXX = g++ 
CXXFLAGS = -Wall -Wextra -std=c++17 -I.
TARGET = physics_engine.out 

SRC_PHYS_ENGINE_DIR = physicsEngine
OBJ_PHYS_ENGINE_DIR = obj

SRCS_PHYS_ENGINE = $(wildcard $(SRC_PHYS_ENGINE_DIR)/*.cpp)

OBJS_PHYS_ENGINE     = $(patsubst $(SRC_PHYS_ENGINE_DIR)/%.cpp, $(OBJ_PHYS_ENGINE_DIR)/%.o, $(SRCS_PHYS_ENGINE))

physics_engine: $(TARGET)

$(TARGET): $(OBJS_PHYS_ENGINE)
	$(CXX) $(CXXFLAGS) $(OBJS_PHYS_ENGINE) -o $(TARGET)
	
$(OBJ_PHYS_ENGINE_DIR)/%.o: $(SRC_PHYS_ENGINE_DIR)/%.cpp
	@mkdir -p $(OBJ_PHYS_ENGINE_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run 
clean:
	rm -rf $(OBJ_PHYS_ENGINE_DIR) $(TARGET)

run:
	./$(TARGET)