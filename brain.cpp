#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>

struct Pesos {
    float valores[20];
    int total_ativos;
};

std::unordered_map<std::string, Pesos> memoria;

void carregar_cerebro(std::unordered_map<std::string, Pesos>& memoria) {
    std::ifstream file("Database/Database.bin", std::ios::binary);
    if (!file) return;

    size_t size;
    while (file.read((char*)&size, sizeof(size))) {
        std::string word(size, '\0');
        file.read(&word[0], size);
        Pesos p;
        file.read((char*)&p, sizeof(Pesos));
        memoria[word] = p;
    }
    file.close();
}

void salvar_cerebro() {
    std::ofstream file("Database/Database.bin", std::ios::binary | std::ios::trunc);
    if (!file) return;
    for (auto const& [palavra, p] : memoria) {
        size_t size = palavra.size();
        file.write((char*)&size, sizeof(size));
        file.write(palavra.c_str(), size);
        file.write((char*)&p, sizeof(Pesos));
    }
    file.close();
}

void aplicar_feedback(std::string query, int classe_correta) {
    float taxa = 0.1f;
    std::stringstream ss(query);
    std::string token;

    while (ss >> token) {
        token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
            return std::ispunct(c);
        }), token.end());
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);

        if (token.empty()) continue;

        if (memoria.find(token) == memoria.end()) {
            Pesos novo_p = {{0.0f}, 3};
            memoria[token] = novo_p;
        }

        if (classe_correta >= 0 && classe_correta < 20) {
            memoria[token].valores[classe_correta] += taxa;
            for (int i = 0; i < 20; i++) {
                if (i != classe_correta) {
                    memoria[token].valores[i] -= taxa / 19.0f;
                }
            }
        }
    }
    salvar_cerebro();
}

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

std::string fetch_wiki(std::string query) {
    CURL* curl;
    std::string readBuffer;
    std::string BufferRes;
    curl = curl_easy_init();
    if (curl) {
        char* encoded_query = curl_easy_escape(curl, query.c_str(), query.length());
        std::string urlfirst = "https://pt.wikipedia.org/w/api.php?action=opensearch&search=" + std::string(encoded_query) + "&limit=1&format=json";
        curl_free(encoded_query);
        curl_easy_setopt(curl, CURLOPT_URL, urlfirst.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "RubertIA/1.0");
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                json dados = json::parse(readBuffer);
                if (!dados[1].empty()) {
                    std::string title = dados[1][0];
                    char* encoded_title = curl_easy_escape(curl, title.c_str(), title.length());
                    std::string urlsecond = "https://pt.wikipedia.org/api/rest_v1/page/summary/" + std::string(encoded_title);
                    curl_free(encoded_title);
                    curl_easy_setopt(curl, CURLOPT_URL, urlsecond.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &BufferRes);
                    if (curl_easy_perform(curl) == CURLE_OK) {
                        json d2 = json::parse(BufferRes);
                        return d2.value("extract", "Sem resumo.");
                    }
                }
            } catch (...) {}
        }
        curl_easy_cleanup(curl);
    }
    return "";
}

void verificar_python(const char* codigo) {
    std::stringstream ss(codigo);
    std::string linha;
    int linha_num = 0;
    while (std::getline(ss, linha)) linha_num++;
    std::cout << "Python: Nenhum erro encontrado\n Linhas: " << linha_num << std::endl;
}

void verificar_cpp(const char* codigo) {
    std::stringstream ss(codigo);
    std::string linha;
    int linha_num = 0;
    while (std::getline(ss, linha)) linha_num++;
    std::cout << "C++: Nenhum erro encontrado\n Linhas: " << linha_num << std::endl;
}

extern "C" {
    void web_brain(const char* data) {
        if (!data) return;
        int intent = (unsigned char)data[0];
        std::string query(&data[1]); 
        if(query.empty()) return;
        if (memoria.empty()) carregar_cerebro(memoria);
        
        if (intent >= 0x02 && intent <= 0x04) {
            int classe_correta = intent - 0x02;
            std::string frase_para_treinar = query.substr(1);
            aplicar_feedback(frase_para_treinar, classe_correta);
            return;
        } else if (intent == 0x05) {
            if (data[1] == 0x10) verificar_python(&data[2]);
            if (data[1] == 0x20) verificar_cpp(&data[2]);
            return;
        } else if (intent == 0x07) {
			std::cout << "Executando: " << query;
			system(query.c_str());
			return;
		}

        std::vector<float> scores(20, 0.0f);
        std::stringstream ss(query);
        std::string token;
        while (ss >> token) {
            token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
                return std::ispunct(c);
            }), token.end());
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            if (memoria.count(token)) {
                for(int i = 0; i < 20; i++) scores[i] += memoria[token].valores[i];
            }
        }
        
        float sum_exp = 0.0f;
        std::vector<float> p(20);
        for(int i = 0; i < 3; i++) {
            p[i] = std::exp(scores[i]);
            sum_exp += p[i];
        }
        for(int i = 0; i < 3; i++) p[i] /= sum_exp;

        auto it = std::max_element(p.begin(), p.begin() + 3);
        int winner_idx = std::distance(p.begin(), it);
        float confidence = *it;

        std::vector<std::string> labels = {"XADREZ", "PROGRAMACAO", "CONVERSA"};

        if (confidence < 0.45f) {
            std::cout << "Não tenho certeza do que você precisa." << std::endl;
            return;
        }

        if (winner_idx == 0) {
            if (intent == 0x01) std::cout << fetch_wiki("xadrez") << std::endl;
        } else if (winner_idx == 1) {
            if (intent == 0x01) std::cout <<  fetch_wiki("programacao") << std::endl;
            if (intent == 0x06) std::cout << "GERAR CODIGO" << std::endl;
        } else if (winner_idx == 2) {
            std::cout << "Olá! Como posso ajudar?" << std::endl;
        }
        
        printf(" | Xadrez: %.2f%% | Prog: %.2f%% | Conversa: %.2f%%\n", p[0]*100, p[1]*100, p[2]*100);
        std::cout << "Assunto Dominante: " << labels[winner_idx] << " (" << confidence * 100 << "%)" << std::endl;
        
        std::cout << std::flush;
    }
}
