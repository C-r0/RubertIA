#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

int main() {
    string p1, p2;

    cout << "Ola eu sou o Rubert me pergunte qualquer coisa\n";
    cin >> p1 >> p2;

    cout << "Separando pergunta\n";
    cout << p1 << "\n";
    cout << p2 << "\n\n";
	
	string p3 = p1+p2;

    unordered_map<string, vector<string>> indice;
    indice["expliquec++"].push_back("1");
	indice["comoc++"].push_back("1");
	indice["expliquecachorro"].push_back("2");
	indice["comocachorro"].push_back("2");
	indice["expliquebrincar"].push_back("3");
	indice["comobrincar"].push_back("4");

    for (auto t : indice[p3]) {
        cout << "Procurando por: " << t << "\n";

        std::ifstream arquivo("Database/Database.txt");
        if (!arquivo) {
            std::cerr << "Erro ao abrir o arquivo.\n";
            return 1;
        }

        char c;
        bool capturando = false;
        std::string resultado;

        while (arquivo.get(c)) { 
            if (c == t[0] && !capturando) { 
                capturando = true;
                continue; 
            }

            if (capturando) {
                if (c == '.') {
                    break;
                }
                resultado += c; 
            }
        }

        arquivo.close();

        if (!resultado.empty()) {
            std::cout << resultado << std::endl;
        } else {
            std::cout << "Não encontrou o padrão no arquivo para '" << t << "'.\n";
        }
    }

    return 0;
}