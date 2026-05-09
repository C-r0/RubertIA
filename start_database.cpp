#include <iostream>
#include <fstream>
#include <string>
#include <map>

struct Pesos {
    float xadrez;
    float prog;
    float conv;
};

int main() {
    std::map<std::string, Pesos> memoria;
    std::ifstream txt_file("conhecimento.txt");
    std::string palavra;
    float p_xadrez, p_prog, p_conv;

    while (txt_file >> palavra >> p_xadrez >> p_prog >> p_conv) {
        memoria[palavra] = {p_xadrez, p_prog, p_conv};
    }

    std::ofstream bin_file("Database/Database.bin", std::ios::binary);
    for (auto const& [word, data] : memoria) {
        size_t size = word.size();
        bin_file.write((char*)&size, sizeof(size));
        bin_file.write(word.c_str(), size);
        bin_file.write((char*)&data, sizeof(Pesos));
    }

    std::cout << "Database criado" << std::endl;
    return 0;
}
