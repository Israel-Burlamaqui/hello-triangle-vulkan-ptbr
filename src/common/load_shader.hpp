#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>

static std::vector<char> readFile(const std::string& filename)
{
    /*
        A vantagem de começar a ler um arquivo pelo fim é que podemos
        ver a posição, determinar o tamanho do arquivo e alocar um buffer.
    */
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Falha ao abrir o arquivo!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    /* 
        Depos disso, podemos voltar ao início e ler todos os bytes
        de uma vez só.
    */ 
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}