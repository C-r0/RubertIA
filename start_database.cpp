#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#define MAX_TEMAS 20 

struct Pesos {
    float valores[MAX_TEMAS];
    int total_ativos;
};

int main() {
    std::map<std::string, Pesos> memoria;
    std::ifstream txt_file("conhecimento.txt");
    std::string palavra;
    
    while (txt_file >> palavra) {
        Pesos p = {0};
        int i = 0;
        
        for(i = 0; i < 20; i++) {
            txt_file >> p.valores[i];
        }
        p.total_ativos = i;
        memoria[palavra] = p;
    }

    std::ofstream bin_file("Database/Database.bin", std::ios::binary);
    for (auto const& [word, data] : memoria) {
        size_t size = word.size();
        bin_file.write((char*)&size, sizeof(size));
        bin_file.write(word.c_str(), size);
        bin_file.write((char*)&data, sizeof(Pesos));
    }

    std::cout << "Database criado com " << MAX_TEMAS << " temas." << std::endl;
    return 0;
}
