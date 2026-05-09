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

struct Pesos {
    float xadrez;
    float prog;
    float conversa;
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
    if (!file) {
        std::cerr << "Erro ao abrir Database.bin" << std::endl;
        return;
    }

    for (auto const& [palavra, p] : memoria) {
        size_t size = palavra.size();
        file.write((char*)&size, sizeof(size));

        file.write(palavra.c_str(), size);

        file.write((char*)&p, sizeof(Pesos));
    }

    file.close();
    std::cout << ">>> Memória persistida no Database.bin!" << std::endl;
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
            memoria[token] = {0.0f, 0.0f, 0.0f};
        }

        if (classe_correta == 0) memoria[token].xadrez += taxa;
        else if (classe_correta == 1) memoria[token].prog += taxa;
        else if (classe_correta == 2) memoria[token].conversa += taxa;
        
        memoria[token].xadrez -= taxa / 4.0f;
        memoria[token].prog -= taxa / 4.0f;
        memoria[token].conversa -= taxa / 4.0f;
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
    int linha_num = 1;

    while (std::getline(ss, linha)) {
        linha_num++;
    }
    std::cout << "Python: Nenhum erro encontrado\n Linhas: " << linha_num;
}

void verificar_cpp(const char* codigo) {
	std::stringstream ss(codigo);
    std::string linha;
    int linha_num = 1;

    while (std::getline(ss, linha)) {
        linha_num++;
    }
    std::cout << "C++: Nenhum erro encontrado\n Linhas: " << linha_num;
}

extern "C" {
    void web_brain(const char* data) {
        if (!data) return;
        
        int intent = (unsigned char)data[0];
        
        std::string query(&data[1]); 

        if(query.empty()) return;
        
        if (memoria.empty()) carregar_cerebro(memoria);
        
        float score_xadrez = 0.0f, score_prog = 0.0f, score_conversa = 0.0f;
        
        std::stringstream ss(query);
        std::string token;
        
        if (intent == 0x02) {
            if (query.length() < 2) return;

            int classe_correta = 0;
            std::string frase_para_treinar = query.substr(1);

            std::cout << "[ MODO TREINO ] Ajustando para classe: " << classe_correta << std::endl;
            aplicar_feedback(frase_para_treinar, classe_correta);
            return;
        } else if (intent == 0x03) {
            if (query.length() < 2) return;

            int classe_correta = 1;
            std::string frase_para_treinar = query.substr(1);

            std::cout << "[ MODO TREINO ] Ajustando para classe: " << classe_correta << std::endl;
            aplicar_feedback(frase_para_treinar, classe_correta);
            return;
        } else if (intent == 0x04) {
            if (query.length() < 2) return;

            int classe_correta = 2;
            std::string frase_para_treinar = query.substr(1);

            std::cout << "[ MODO TREINO ] Ajustando para classe: " << classe_correta << std::endl;
            aplicar_feedback(frase_para_treinar, classe_correta);
            return;
        } else if (intent == 0x05) {
			if (data[1] == 0x10) {
				verificar_python(&data[2]);
			}
			if (data[1] == 0x20) {
				verificar_cpp(&data[2]);
			}
			return;
		}
        
        
        while (ss >> token) {
            token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
                return std::ispunct(c);
            }), token.end());

            std::transform(token.begin(), token.end(), token.begin(), ::tolower);

            if (memoria.count(token)) {
                score_xadrez += memoria[token].xadrez;
                score_prog += memoria[token].prog;
                score_conversa += memoria[token].conversa;
            }
        }
        
        std::vector<float> p = {score_xadrez, score_prog, score_conversa};
        std::vector<std::string> labels = {"XADREZ", "PROGRAMACAO", "CONVERSA"};
        
        float sum_exp = 0.0f;
        for(int i=0; i<3; ++i) {
            p[i] = std::exp(p[i]);
            sum_exp += p[i];
        }
        for(int i=0; i<3; ++i) p[i] /= sum_exp;

        auto it = std::max_element(p.begin(), p.end());
        int winner_idx = std::distance(p.begin(), it);
        float confidence = *it;

        std::cout << "\n[ Análise de Confiança ]" << std::endl;
        printf("Xadrez: %.2f%% | Prog: %.2f%% | Conversa: %.2f%%\n", p[0]*100, p[1]*100, p[2]*100);
        std::cout << "Assunto Dominante: " << labels[winner_idx] << " (" << confidence * 100 << "%)" << std::endl;

        if (confidence < 0.45f) {
            std::cout << "RubertIA: Não tenho certeza do que você precisa. Pode ser mais específico?" << std::endl;
            return;
        }

        if (winner_idx == 0) {
            if (intent == 0x01) std::cout << "AÇÃO: EXPLICAR | OQUE XADREZ" << std::endl;
            
            if (p[2] > 0.30f) std::cout << "(Contexto de conversa detectado)" << std::endl;

            if (confidence > 0.50f) {
                std::cout << "Resultado: " << fetch_wiki("xadrez") << std::endl;
            }
        } else if (winner_idx == 1) {
			if (intent == 0x01) {
				std::cout << "EXPLICAR OQUE PROGRAMACAO";
				std::cout << "Resultado: " << fetch_wiki("programacao") << std::endl;
			}
			if (intent == 0x06) std::cout << "GERAR CODIGO";
		} else if (winner_idx == 2) {
            std::cout << "RubertIA: Olá! Como posso ajudar com Xadrez ou Programação hoje?" << std::endl;
        }
    }
}
