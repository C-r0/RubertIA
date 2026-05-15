import requests
from bs4 import BeautifulSoup
import re
import feedparser
from flask import Flask, render_template, request, jsonify
import subprocess

app = Flask(__name__)

def buscarnoticias():
    RSS_URL = "https://www.noticiasaominuto.com/rss/ultima-hora"
    r = requests.get(RSS_URL, headers={"User-Agent":"rubert-bot/0.1"}, timeout=10)
    r.raise_for_status()
    feed = feedparser.parse(r.content)
    lista_noticias = feed.entries[:20]
    return lista_noticias

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/noticias')
def api_noticias():
    lista_noticias = buscarnoticias()
    dados = []
    for entry in lista_noticias:
        dados.append({
            "title": entry.title,
            "link": entry.link,
            "image": entry.enclosures[0].url if entry.get('enclosures') else ""
        })
    return jsonify(dados)

@app.route('/api/search')
def api_search():
    query = request.args.get('q')
    if not query:
        return jsonify([])

    SEARX_URL = "http://127.0.0.1:8080/search"

    params = {
        'q': query,
        'format': 'json',
        'categories': 'general',
        'language': 'pt-BR',
        'time_range': '',
        'safesearch': '0',
        'theme': 'simple'
    }

    headers = {
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "Accept": "application/json",
    }

    try:
        r = requests.get(SEARX_URL, params=params, headers=headers, timeout=10)
        
        r.raise_for_status()
            
        data = r.json()
        return jsonify(data.get('results', []))
    except Exception as e:
        print(f"Erro detalhado: {e}")
        return jsonify({"error": "Falha na comunicação com o motor de busca"}), 500

@app.route('/api/chat')
def api_chat():
    query = request.args.get('q')
    if not query:
        return jsonify(content="Envie uma pergunta.")
    
    try:
        process = subprocess.Popen(
            ['./build/RubertIA'], 
            stdin=subprocess.PIPE, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            text=True
        )
        res, error = process.communicate(input=query + '\n')

        if process.returncode != 0:
            return jsonify(content=f"Erro no parser: {error}")

        return jsonify(content=res)
    except Exception as e:
        return jsonify(content=f"Erro no servidor: {str(e)}")
        
if __name__ == '__main__':
    app.run(debug=True)
