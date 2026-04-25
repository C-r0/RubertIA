import requests
from bs4 import BeautifulSoup
import re
import feedparser
from flask import Flask, render_template, request, jsonify

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

    payload = {
        'q': query,
        'format': 'json',
    }

    headers = {
        "User-Agent": "Mozilla/5.0 (X11; Arch Linux; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "Origin": "http://127.0.0.1:8080",
        "Referer": "http://127.0.0.1:8080/"
    }

    try:
        r = requests.get(SEARX_URL, params=payload,  headers=headers, timeout=10)
        r.raise_for_status()
            
        search_results = r.json().get('results', [])
        return jsonify(search_results[:10])
    except Exception as e:
        print(f"Erro na busca POST: {e}")
        return jsonify({"error": "Falha na busca"}), 500

if __name__ == '__main__':
    app.run(debug=True)
