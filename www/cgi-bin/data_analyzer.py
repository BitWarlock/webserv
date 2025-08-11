#!/usr/bin/env python3

import os
import sys
import re
from collections import Counter
import urllib.parse

def analyze_text(text):
    words = re.findall(r'\b\w+\b', text)
    word_freq = Counter([w.lower() for w in words])
    char_count = len(text)
    word_count = len(words)
    sentence_count = len(re.split(r'[.!?]+', text))
    patterns = {
        'uppercase_words': len(re.findall(r'\b[A-Z]{2,}\b', text)),
        'lowercase_words': len(re.findall(r'\b[a-z]{2,}\b', text)),
        'numbers': len(re.findall(r'\d', text)),
        'special_chars': len(re.findall(r'[^a-zA-Z0-9\s]', text)),
        'urls': len(re.findall(r'https?://\S+', text))
    }
    return {
        'word_frequency': dict(word_freq.most_common(10)),
        'statistics': {
            'characters': char_count,
            'words': word_count,
            'sentences': sentence_count,
            'avg_word_length': sum(len(word) for word in words) / word_count if word_count > 0 else 0
        },
        'patterns': patterns
    }

def html_escape(s):
    return (s.replace('&', '&amp;')
             .replace('<', '&lt;')
             .replace('>', '&gt;')
             .replace('"', '&quot;'))

def get_posted_text():
    method = os.environ.get('REQUEST_METHOD', '').upper()
    if method != 'POST':
        return ''
    length_str = os.environ.get('CONTENT_LENGTH', '0')
    try:
        length = int(length_str)
    except ValueError:
        length = 0
    data = ''
    if length > 0:
        data = sys.stdin.read(length)
    content_type = os.environ.get('CONTENT_TYPE', '')
    if 'application/x-www-form-urlencoded' in content_type:
        parsed = urllib.parse.parse_qs(data, keep_blank_values=True)
        return parsed.get('text', [''])[0]
    return data

def generate_html_report():
    posted_text = get_posted_text()
    text_to_analyze = posted_text if posted_text.strip() else ""
    text_analysis = analyze_text(text_to_analyze)
    filled_textarea = html_escape(posted_text)

    html = """Content-Type: text/html\r\n\r\n<!DOCTYPE html>
<html>
<head>
    <title>Data Analyzer - Python CGI</title>
    <style>
        body { 
            font-family: 'Segoe UI', sans-serif; 
            background: #1a1a1a; 
            color: #ffffff; 
            margin: 20px; 
        }
        .container { 
            max-width: 1200px; 
            margin: 0 auto; 
            background: rgba(255,255,255,0.05); 
            padding: 30px; 
            border-radius: 15px; 
        }
        h1 { 
            color: #ff5722; 
            text-align: center; 
            margin-bottom: 30px; 
        }
        .section { 
            margin-bottom: 30px; 
        }
        .section h2 { 
            color: #ff5722; 
            margin-bottom: 15px; 
        }
        .grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); 
            gap: 20px; 
        }
        .card { 
            background: rgba(255,255,255,0.1); 
            padding: 20px; 
            border-radius: 10px; 
            border-left: 4px solid #ff5722; 
        }
        .stat-grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); 
            gap: 15px; 
            margin-top: 15px; 
        }
        .stat-item { 
            text-align: center; 
            padding: 10px; 
            background: rgba(255,255,255,0.05); 
            border-radius: 8px; 
        }
        .stat-number { 
            font-size: 1.5em; 
            color: #ff5722; 
            font-weight: bold; 
        }
        .stat-label { 
            color: #cccccc; 
            font-size: 0.9em; 
            margin-top: 5px; 
        }
        .table { 
            width: 100%; 
            border-collapse: collapse; 
            margin-top: 15px; 
        }
        .table th, .table td { 
            padding: 10px; 
            text-align: left; 
            border-bottom: 1px solid rgba(255,255,255,0.1); 
        }
        .table th { 
            background: rgba(255, 87, 34, 0.2); 
            color: #ff5722; 
            font-weight: bold; 
        }
        .table tr:hover { 
            background: rgba(255,255,255,0.05); 
        }
        .back-btn { 
            background: #666; 
            color: white; 
            padding: 10px 20px; 
            text-decoration: none; 
            border-radius: 5px; 
            display: inline-block; 
            margin-top: 20px; 
        }
        .word-cloud { 
            display: flex; 
            flex-wrap: wrap; 
            gap: 10px; 
            margin-top: 15px; 
        }
        .word-item { 
            background: rgba(255, 87, 34, 0.2); 
            padding: 5px 10px; 
            border-radius: 15px; 
            font-size: 0.9em; 
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Data Analyzer</h1>
"""

    html += f"""
        <div class="section">
            <h2>Text Analysis</h2>
            <div class="grid">
                <div class="card">
                    <h3 style="color: #ff5722; margin-bottom: 15px;">Analyze Your Text</h3>
                    <form method="POST">
                        <textarea name="text" rows="8" style="width:100%; background: rgba(255,255,255,0.05); color: #fff; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 10px;">{filled_textarea}</textarea>
                        <br/><br/>
                        <button type="submit" class="back-btn">Analyze</button>
                    </form>
                </div>

                <div class="card">
                    <h3 style="color: #ff5722; margin-bottom: 15px;">Text Statistics</h3>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['statistics']['characters']}</div>
                        <div class="stat-label">Characters</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['statistics']['words']}</div>
                        <div class="stat-label">Words</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['statistics']['sentences']}</div>
                        <div class="stat-label">Sentences</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['statistics']['avg_word_length']:.1f}</div>
                        <div class="stat-label">Avg Word Length</div>
                    </div>
                </div>
                
                <div class="card">
                    <h3 style="color: #ff5722; margin-bottom: 15px;">Pattern Detection</h3>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['patterns']['uppercase_words']}</div>
                        <div class="stat-label">Uppercase Words</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['patterns']['lowercase_words']}</div>
                        <div class="stat-label">Lowercase Words</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['patterns']['numbers']}</div>
                        <div class="stat-label">Numbers</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{text_analysis['patterns']['special_chars']}</div>
                        <div class="stat-label">Special Characters</div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <h3 style="color: #ff5722; margin-bottom: 15px;">Most Common Words</h3>
                <div class="word-cloud">
"""

    for word, count in text_analysis['word_frequency'].items():
        if len(word) > 2:
            html += f'<span class="word-item">{word} ({count})</span>'

    html += """
                </div>
            </div>
        </div>
    
        <a href="/cgi-bin/cgi_test.html" class="back-btn">Back to CGI Testing</a>
    </div>
</body>
</html>
"""

    return html

if __name__ == "__main__":
    print(generate_html_report()) 
