# Hello Triangle em Vulkan (português brasileiro)

Este repositório implementa os capítulos de "Drawing a triangle" a "Vertex buffer creation" do [vulkan-tutorial](https://vulkan-tutorial.com/) com comentários em português brasileiro. Nomes de atributos, métodos, classes e funções estáticas permanecem iguais aos originais.


## Compilando

Instale as dependências necessárias:

Debian/Ubuntu
```
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers spirv-tools glslc
```

Arch Linux
```
sudo pacman -S vulkan-devel shaderc
```

Clone o repositório e compile:

```
git clone https://github.com/Israel-Burlamaqui/hello-triangle-vulkan-ptbr.git
cd hello-triangle-vulkan-ptbr/

./compile_shaders.sh
make
```

Depois execute:

```
make test
make clean
```

