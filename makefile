CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I.
TARGET = physics_engine.out

SRC_PHYS_ENGINE_DIR     = physicsEngine
SRC_GRAPHICS_ENGINE_DIR = graphicsEngine
SRC_CORE				= core
OBJ_DIR                 = obj

SRCS_PHYS_ENGINE     = $(wildcard $(SRC_PHYS_ENGINE_DIR)/*.cpp)
SRCS_GRAPHICS_ENGINE = $(wildcard $(SRC_GRAPHICS_ENGINE_DIR)/*.cpp)
SRCS_CORE			 = $(wildcard $(SRC_CORE)/*.cpp)
SRCS_ECS_TEST        = entitySystem/test.cpp

OBJS_PHYS_ENGINE     = $(patsubst $(SRC_PHYS_ENGINE_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_PHYS_ENGINE))
OBJS_GRAPHICS_ENGINE = $(patsubst $(SRC_GRAPHICS_ENGINE_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_GRAPHICS_ENGINE))
OBJS_CORE 			 = $(patsubst $(SRC_CORE)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_CORE))
OBJS = $(OBJS_PHYS_ENGINE) $(OBJS_GRAPHICS_ENGINE) $(OBJS_CORE)

physics_engine: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

$(OBJ_DIR)/%.o: $(SRC_PHYS_ENGINE_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_GRAPHICS_ENGINE_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_CORE)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run ecs_test
ecs_test: $(SRCS_ECS_TEST) entitySystem/ecs.h entitySystem/entity.h
	$(CXX) $(CXXFLAGS) $(SRCS_ECS_TEST) -o ecs_test

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run:
	./$(TARGET)
