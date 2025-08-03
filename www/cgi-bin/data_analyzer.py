#!/usr/bin/env python3

import os
import sys
import re
import json
import statistics
from collections import Counter, defaultdict
from datetime import datetime, timedelta
import random

# Sample data for analysis
sample_text = """
WebServ is a high-performance HTTP server implementation written in C++.
The server supports multiple concurrent connections and efficient request handling.
Key features include: CGI execution, file uploads, session management, and custom error pages.
The project demonstrates advanced networking concepts and system programming techniques.
Performance benchmarks show excellent throughput under high load conditions.
Security features include input validation, request sanitization, and proper error handling.
The codebase follows modern C++ standards and best practices for maintainability.
Documentation includes comprehensive API references and usage examples.
Testing covers unit tests, integration tests, and stress testing scenarios.
Deployment options include Docker containers and native system installation.
"""

sample_logs = [
    {"timestamp": "2024-01-15 10:30:00", "level": "INFO", "message": "Server started on port 8003", "ip": "192.168.1.100"},
    {"timestamp": "2024-01-15 10:31:15", "level": "INFO", "message": "Client connected from 192.168.1.101", "ip": "192.168.1.101"},
    {"timestamp": "2024-01-15 10:32:00", "level": "WARNING", "message": "Large file upload detected", "ip": "192.168.1.102"},
    {"timestamp": "2024-01-15 10:33:45", "level": "ERROR", "message": "CGI script execution failed", "ip": "192.168.1.100"},
    {"timestamp": "2024-01-15 10:35:20", "level": "INFO", "message": "File served successfully", "ip": "192.168.1.103"},
    {"timestamp": "2024-01-15 10:36:10", "level": "WARNING", "message": "Rate limit exceeded for IP", "ip": "192.168.1.104"},
    {"timestamp": "2024-01-15 10:37:30", "level": "INFO", "message": "Session created for user", "ip": "192.168.1.105"},
    {"timestamp": "2024-01-15 10:38:15", "level": "ERROR", "message": "Database connection timeout", "ip": "192.168.1.100"},
    {"timestamp": "2024-01-15 10:39:00", "level": "INFO", "message": "Configuration reloaded", "ip": "192.168.1.106"},
    {"timestamp": "2024-01-15 10:40:30", "level": "WARNING", "message": "Memory usage high", "ip": "192.168.1.107"}
]

sample_metrics = [
    {"metric": "response_time", "value": 45.2, "timestamp": "2024-01-15 10:30:00"},
    {"metric": "requests_per_second", "value": 1250, "timestamp": "2024-01-15 10:31:00"},
    {"metric": "memory_usage", "value": 78.5, "timestamp": "2024-01-15 10:32:00"},
    {"metric": "cpu_usage", "value": 65.3, "timestamp": "2024-01-15 10:33:00"},
    {"metric": "active_connections", "value": 89, "timestamp": "2024-01-15 10:34:00"},
    {"metric": "response_time", "value": 52.1, "timestamp": "2024-01-15 10:35:00"},
    {"metric": "requests_per_second", "value": 1180, "timestamp": "2024-01-15 10:36:00"},
    {"metric": "memory_usage", "value": 82.1, "timestamp": "2024-01-15 10:37:00"},
    {"metric": "cpu_usage", "value": 71.8, "timestamp": "2024-01-15 10:38:00"},
    {"metric": "active_connections", "value": 95, "timestamp": "2024-01-15 10:39:00"}
]

def analyze_text(text):
    """Analyze text content for patterns and statistics"""
    # Word frequency analysis
    words = re.findall(r'\b\w+\b', text.lower())
    word_freq = Counter(words)
    
    # Character analysis
    char_count = len(text)
    word_count = len(words)
    sentence_count = len(re.split(r'[.!?]+', text))
    
    # Common patterns
    patterns = {
        'uppercase_words': len(re.findall(r'\b[A-Z][a-z]+\b', text)),
        'numbers': len(re.findall(r'\d+', text)),
        'special_chars': len(re.findall(r'[^a-zA-Z0-9\s]', text)),
        'urls': len(re.findall(r'https?://\S+', text)),
        'emails': len(re.findall(r'\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b', text))
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

def analyze_logs(logs):
    """Analyze log entries for patterns and statistics"""
    # Log level distribution
    level_counts = Counter(log['level'] for log in logs)
    
    # IP address analysis
    ip_counts = Counter(log['ip'] for log in logs)
    
    # Time-based analysis
    timestamps = [datetime.strptime(log['timestamp'], '%Y-%m-%d %H:%M:%S') for log in logs]
    time_diffs = [(timestamps[i+1] - timestamps[i]).total_seconds() for i in range(len(timestamps)-1)]
    
    # Message pattern analysis
    message_patterns = Counter()
    for log in logs:
        # Extract common patterns from messages
        if 'error' in log['message'].lower():
            message_patterns['error_related'] += 1
        if 'connection' in log['message'].lower():
            message_patterns['connection_related'] += 1
        if 'file' in log['message'].lower():
            message_patterns['file_related'] += 1
        if 'user' in log['message'].lower():
            message_patterns['user_related'] += 1
    
    return {
        'level_distribution': dict(level_counts),
        'ip_distribution': dict(ip_counts.most_common(5)),
        'time_analysis': {
            'total_entries': len(logs),
            'time_span_minutes': (timestamps[-1] - timestamps[0]).total_seconds() / 60,
            'avg_interval_seconds': statistics.mean(time_diffs) if time_diffs else 0
        },
        'message_patterns': dict(message_patterns)
    }

def analyze_metrics(metrics):
    """Analyze performance metrics"""
    # Group metrics by type
    metric_groups = defaultdict(list)
    for metric in metrics:
        metric_groups[metric['metric']].append(metric['value'])
    
    analysis = {}
    for metric_name, values in metric_groups.items():
        analysis[metric_name] = {
            'count': len(values),
            'mean': statistics.mean(values),
            'median': statistics.median(values),
            'min': min(values),
            'max': max(values),
            'std_dev': statistics.stdev(values) if len(values) > 1 else 0
        }
    
    return analysis

def generate_html_report():
    """Generate the complete HTML report"""
    # Perform analyses
    text_analysis = analyze_text(sample_text)
    log_analysis = analyze_logs(sample_logs)
    metrics_analysis = analyze_metrics(sample_metrics)
    
    # Generate HTML
    html = """Content-Type: text/html

<!DOCTYPE html>
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
    
    # Text Analysis Section
    html += f"""
        <div class="section">
            <h2>Text Analysis</h2>
            <div class="grid">
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
        if len(word) > 2:  # Skip very short words
            html += f'<span class="word-item">{word} ({count})</span>'
    
    html += """
                </div>
            </div>
        </div>
"""
    
    # Log Analysis Section
    html += f"""
        <div class="section">
            <h2>Log Analysis</h2>
            <div class="grid">
                <div class="card">
                    <h3 style="color: #ff5722; margin-bottom: 15px;">Log Level Distribution</h3>
"""
    
    for level, count in log_analysis['level_distribution'].items():
        html += f"""
                    <div class="stat-item">
                        <div class="stat-number">{count}</div>
                        <div class="stat-label">{level}</div>
                    </div>
"""
    
    html += """
                </div>
                
                <div class="card">
                    <h3 style="color: #ff5722; margin-bottom: 15px;">Time Analysis</h3>
                    <div class="stat-item">
                        <div class="stat-number">{}</div>
                        <div class="stat-label">Total Entries</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{:.1f}</div>
                        <div class="stat-label">Time Span (minutes)</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number">{:.1f}</div>
                        <div class="stat-label">Avg Interval (seconds)</div>
                    </div>
                </div>
            </div>
        </div>
""".format(
        log_analysis['time_analysis']['total_entries'],
        log_analysis['time_analysis']['time_span_minutes'],
        log_analysis['time_analysis']['avg_interval_seconds']
    )
    
    # Metrics Analysis Section
    html += """
        <div class="section">
            <h2>Performance Metrics Analysis</h2>
            <div class="card">
                <table class="table">
                    <thead>
                        <tr>
                            <th>Metric</th>
                            <th>Count</th>
                            <th>Mean</th>
                            <th>Median</th>
                            <th>Min</th>
                            <th>Max</th>
                            <th>Std Dev</th>
                        </tr>
                    </thead>
                    <tbody>
"""
    
    for metric_name, stats in metrics_analysis.items():
        html += f"""
                        <tr>
                            <td>{metric_name.replace('_', ' ').title()}</td>
                            <td>{stats['count']}</td>
                            <td>{stats['mean']:.2f}</td>
                            <td>{stats['median']:.2f}</td>
                            <td>{stats['min']:.2f}</td>
                            <td>{stats['max']:.2f}</td>
                            <td>{stats['std_dev']:.2f}</td>
                        </tr>
"""
    
    html += """
                    </tbody>
                </table>
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