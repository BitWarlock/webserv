#!/bin/bash

# System monitoring and process management CGI script

# Get system information
get_system_info() {
    echo "Content-Type: text/html"
    echo ""
    echo "<!DOCTYPE html>"
    echo "<html>"
    echo "<head>"
    echo "<title>System Monitor - Shell CGI</title>"
    echo "<style>"
    echo "body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }"
    echo ".container { max-width: 1200px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }"
    echo "h1 { color: #4caf50; text-align: center; margin-bottom: 30px; }"
    echo ".section { margin-bottom: 30px; }"
    echo ".section h2 { color: #4caf50; margin-bottom: 15px; }"
    echo ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }"
    echo ".card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #4caf50; }"
    echo ".stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin-top: 15px; }"
    echo ".stat-item { text-align: center; padding: 10px; background: rgba(255,255,255,0.05); border-radius: 8px; }"
    echo ".stat-number { font-size: 1.5em; color: #4caf50; font-weight: bold; }"
    echo ".stat-label { color: #cccccc; font-size: 0.9em; margin-top: 5px; }"
    echo ".table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
    echo ".table th, .table td { padding: 10px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }"
    echo ".table th { background: rgba(76, 175, 80, 0.2); color: #4caf50; font-weight: bold; }"
    echo ".table tr:hover { background: rgba(255,255,255,0.05); }"
    echo ".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }"
    echo ".process-high { color: #ff5722; }"
    echo ".process-medium { color: #ff9800; }"
    echo ".process-low { color: #4caf50; }"
    echo "</style>"
    echo "</head>"
    echo "<body>"
    echo "<div class='container'>"
    echo "<h1>System Monitor</h1>"
}

# Get CPU and memory usage
get_resource_usage() {
    echo "<div class='section'>"
    echo "<h2>Resource Usage</h2>"
    echo "<div class='grid'>"
    
    # CPU Usage
    cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1)
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>CPU Usage</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${cpu_usage}%</div>"
    echo "<div class='stat-label'>Current CPU</div>"
    echo "</div>"
    echo "</div>"
    
    # Memory Usage
    total_mem=$(free -m | awk 'NR==2{printf "%.1f", $2/1024}')
    used_mem=$(free -m | awk 'NR==2{printf "%.1f", $3/1024}')
    free_mem=$(free -m | awk 'NR==2{printf "%.1f", $4/1024}')
    mem_percent=$(free | awk 'NR==2{printf "%.1f", $3/$2*100}')
    
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Memory Usage</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${total_mem} GB</div>"
    echo "<div class='stat-label'>Total Memory</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${used_mem} GB</div>"
    echo "<div class='stat-label'>Used Memory</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${mem_percent}%</div>"
    echo "<div class='stat-label'>Usage</div>"
    echo "</div>"
    echo "</div>"
    
    # Disk Usage
    disk_usage=$(df -h / | awk 'NR==2 {print $5}' | cut -d'%' -f1)
    disk_total=$(df -h / | awk 'NR==2 {print $2}')
    disk_used=$(df -h / | awk 'NR==2 {print $3}')
    
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Disk Usage</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${disk_usage}%</div>"
    echo "<div class='stat-label'>Usage</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${disk_used}/${disk_total}</div>"
    echo "<div class='stat-label'>Used/Total</div>"
    echo "</div>"
    echo "</div>"
    
    echo "</div>"
    echo "</div>"
}

# Get system uptime and load
get_system_stats() {
    echo "<div class='section'>"
    echo "<h2>System Statistics</h2>"
    echo "<div class='grid'>"
    
    # Uptime
    uptime_seconds=$(cat /proc/uptime | awk '{print $1}')
    uptime_days=$(echo "scale=0; $uptime_seconds/86400" | bc)
    uptime_hours=$(echo "scale=0; ($uptime_seconds%86400)/3600" | bc)
    uptime_minutes=$(echo "scale=0; ($uptime_seconds%3600)/60" | bc)
    
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>System Uptime</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${uptime_days}d ${uptime_hours}h ${uptime_minutes}m</div>"
    echo "<div class='stat-label'>Uptime</div>"
    echo "</div>"
    echo "</div>"
    
    # Load Average
    load_1=$(cat /proc/loadavg | awk '{print $1}')
    load_5=$(cat /proc/loadavg | awk '{print $2}')
    load_15=$(cat /proc/loadavg | awk '{print $3}')
    
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Load Average</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${load_1}</div>"
    echo "<div class='stat-label'>1 min</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${load_5}</div>"
    echo "<div class='stat-label'>5 min</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${load_15}</div>"
    echo "<div class='stat-label'>15 min</div>"
    echo "</div>"
    echo "</div>"
    
    # Process Count
    total_processes=$(ps aux | wc -l)
    running_processes=$(ps aux | grep -v "defunct" | wc -l)
    
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Processes</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${total_processes}</div>"
    echo "<div class='stat-label'>Total</div>"
    echo "</div>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>${running_processes}</div>"
    echo "<div class='stat-label'>Running</div>"
    echo "</div>"
    echo "</div>"
    
    echo "</div>"
    echo "</div>"
}

# Get top processes
get_top_processes() {
    echo "<div class='section'>"
    echo "<h2>Top Processes by CPU Usage</h2>"
    echo "<div class='card'>"
    echo "<table class='table'>"
    echo "<thead>"
    echo "<tr>"
    echo "<th>PID</th>"
    echo "<th>User</th>"
    echo "<th>CPU %</th>"
    echo "<th>Memory %</th>"
    echo "<th>Command</th>"
    echo "</tr>"
    echo "</thead>"
    echo "<tbody>"
    
    # Get top 10 processes by CPU usage
    ps aux --sort=-%cpu | head -11 | tail -10 | while read line; do
        pid=$(echo "$line" | awk '{print $2}')
        user=$(echo "$line" | awk '{print $1}')
        cpu=$(echo "$line" | awk '{print $3}')
        mem=$(echo "$line" | awk '{print $4}')
        cmd=$(echo "$line" | awk '{for(i=11;i<=NF;i++) printf $i" "; print ""}')
        
        # Determine CPU usage class
        cpu_class="process-low"
        if (( $(echo "$cpu > 10" | bc -l) )); then
            cpu_class="process-high"
        elif (( $(echo "$cpu > 5" | bc -l) )); then
            cpu_class="process-medium"
        fi
        
        echo "<tr>"
        echo "<td>$pid</td>"
        echo "<td>$user</td>"
        echo "<td class='$cpu_class'>$cpu%</td>"
        echo "<td>$mem%</td>"
        echo "<td style='max-width: 300px; overflow: hidden; text-overflow: ellipsis;'>$cmd</td>"
        echo "</tr>"
    done
    
    echo "</tbody>"
    echo "</table>"
    echo "</div>"
    echo "</div>"
}

# Get network statistics
get_network_stats() {
    echo "<div class='section'>"
    echo "<h2>Network Statistics</h2>"
    echo "<div class='grid'>"
    
    # Network interfaces
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Network Interfaces</h3>"
    
    ip addr show | grep -E "^[0-9]+:" | while read line; do
        interface=$(echo "$line" | awk '{print $2}' | cut -d: -f1)
        state=$(echo "$line" | grep -o "state [A-Z]*" | awk '{print $2}')
        if [ ! -z "$state" ]; then
            echo "<div style='margin: 10px 0; padding: 10px; background: rgba(255,255,255,0.05); border-radius: 5px;'>"
            echo "<strong style='color: #4caf50;'>$interface</strong> - $state"
            echo "</div>"
        fi
    done
    
    echo "</div>"
    
    # Active connections
    active_connections=$(ss -tuln | wc -l)
    echo "<div class='card'>"
    echo "<h3 style='color: #4caf50; margin-bottom: 15px;'>Active Connections</h3>"
    echo "<div class='stat-item'>"
    echo "<div class='stat-number'>$active_connections</div>"
    echo "<div class='stat-label'>Open Ports</div>"
    echo "</div>"
    echo "</div>"
    
    echo "</div>"
    echo "</div>"
}

# Main execution
get_system_info
get_resource_usage
get_system_stats
get_top_processes
get_network_stats

echo "<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>"
echo "</div>"
echo "</body>"
echo "</html>" 