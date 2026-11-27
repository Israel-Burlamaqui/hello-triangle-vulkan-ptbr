CFLAGS = -std=c++17 -O2
LDFLAGS = -Iinclude -Isrc/common -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: main.cpp
	g++ $(CFLAGS) -o Vulkan_Application main.cpp src/programs/model_application.cpp $(LDFLAGS)

.PHONY: test clean

test: Vulkan_Application
	./Vulkan_Application

clean:
	rm -f Vulkan_Application