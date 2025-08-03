#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;

std::string formatFileSize(uintmax_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double fileSize = static_cast<double>(size);
    
    while (fileSize >= 1024.0 && unit < 4) {
        fileSize /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << fileSize << " " << units[unit];
    return oss.str();
}

std::string getFileType(const fs::path& path) {
    if (fs::is_directory(path)) return "Directory";
    if (fs::is_symlink(path)) return "Symlink";
    if (fs::is_regular_file(path)) return "File";
    return "Other";
}

int main() {
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<!DOCTYPE html>\n";
    std::cout << "<html>\n";
    std::cout << "<head>\n";
    std::cout << "<title>File Operations - C++ CGI</title>\n";
    std::cout << "<style>\n";
    std::cout << "body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }\n";
    std::cout << ".container { max-width: 1000px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }\n";
    std::cout << "h1 { color: #9c27b0; text-align: center; margin-bottom: 30px; }\n";
    std::cout << ".stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 30px; }\n";
    std::cout << ".stat-card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #9c27b0; text-align: center; }\n";
    std::cout << ".stat-number { font-size: 2em; color: #9c27b0; font-weight: bold; }\n";
    std::cout << ".stat-label { color: #cccccc; margin-top: 5px; }\n";
    std::cout << ".file-table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n";
    std::cout << ".file-table th, .file-table td { padding: 12px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }\n";
    std::cout << ".file-table th { background: rgba(156, 39, 176, 0.2); color: #9c27b0; font-weight: bold; }\n";
    std::cout << ".file-table tr:hover { background: rgba(255,255,255,0.05); }\n";
    std::cout << ".file-type { padding: 4px 8px; border-radius: 4px; font-size: 0.8em; }\n";
    std::cout << ".type-dir { background: #4caf50; color: white; }\n";
    std::cout << ".type-file { background: #2196f3; color: white; }\n";
    std::cout << ".type-other { background: #ff9800; color: white; }\n";
    std::cout << ".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }\n";
    std::cout << "</style>\n";
    std::cout << "</head>\n";
    std::cout << "<body>\n";
    std::cout << "<div class='container'>\n";
    std::cout << "<h1>File System Operations</h1>\n";
    
    try {
        fs::path current_path = fs::current_path();
        std::vector<fs::directory_entry> entries;
        
        // Collect directory entries
        for (const auto& entry : fs::directory_iterator(current_path)) {
            entries.push_back(entry);
        }
        
        // Sort entries (directories first, then files)
        std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
            bool a_is_dir = fs::is_directory(a);
            bool b_is_dir = fs::is_directory(b);
            if (a_is_dir != b_is_dir) return a_is_dir > b_is_dir;
            return a.path().filename() < b.path().filename();
        });
        
        // Calculate statistics
        int total_files = 0, total_dirs = 0, total_symlinks = 0;
        uintmax_t total_size = 0;
        
        for (const auto& entry : entries) {
            if (fs::is_directory(entry)) total_dirs++;
            else if (fs::is_symlink(entry)) total_symlinks++;
            else if (fs::is_regular_file(entry)) {
                total_files++;
                total_size += fs::file_size(entry);
            }
        }
        
        // Display statistics
        std::cout << "<div class='stats-grid'>\n";
        std::cout << "<div class='stat-card'>\n";
        std::cout << "<div class='stat-number'>" << entries.size() << "</div>\n";
        std::cout << "<div class='stat-label'>Total Items</div>\n";
        std::cout << "</div>\n";
        
        std::cout << "<div class='stat-card'>\n";
        std::cout << "<div class='stat-number'>" << total_dirs << "</div>\n";
        std::cout << "<div class='stat-label'>Directories</div>\n";
        std::cout << "</div>\n";
        
        std::cout << "<div class='stat-card'>\n";
        std::cout << "<div class='stat-number'>" << total_files << "</div>\n";
        std::cout << "<div class='stat-label'>Files</div>\n";
        std::cout << "</div>\n";
        
        std::cout << "<div class='stat-card'>\n";
        std::cout << "<div class='stat-number'>" << formatFileSize(total_size) << "</div>\n";
        std::cout << "<div class='stat-label'>Total Size</div>\n";
        std::cout << "</div>\n";
        std::cout << "</div>\n";
        
        // Display current directory
        std::cout << "<div style='background: rgba(255,255,255,0.1); padding: 15px; border-radius: 10px; margin-bottom: 20px;'>\n";
        std::cout << "<strong style='color: #9c27b0;'>Current Directory:</strong> " << current_path.string() << "\n";
        std::cout << "</div>\n";
        
        // Display file table
        std::cout << "<table class='file-table'>\n";
        std::cout << "<thead>\n";
        std::cout << "<tr>\n";
        std::cout << "<th>Name</th>\n";
        std::cout << "<th>Type</th>\n";
        std::cout << "<th>Size</th>\n";
        std::cout << "<th>Last Modified</th>\n";
        std::cout << "</tr>\n";
        std::cout << "</thead>\n";
        std::cout << "<tbody>\n";
        
        for (const auto& entry : entries) {
            std::string filename = entry.path().filename().string();
            std::string file_type = getFileType(entry.path());
            std::string size_str = "N/A";
            std::string modified_str = "N/A";
            
            try {
                if (fs::is_regular_file(entry)) {
                    size_str = formatFileSize(fs::file_size(entry));
                }
                
                auto time = fs::last_write_time(entry);
                auto time_t = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
                std::time_t c_time = static_cast<std::time_t>(time_t);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&c_time), "%Y-%m-%d %H:%M");
                modified_str = ss.str();
            } catch (...) {
                // Ignore errors for individual files
            }
            
            std::string type_class = "type-other";
            if (file_type == "Directory") type_class = "type-dir";
            else if (file_type == "File") type_class = "type-file";
            
            std::cout << "<tr>\n";
            std::cout << "<td>" << filename << "</td>\n";
            std::cout << "<td><span class='file-type " << type_class << "'>" << file_type << "</span></td>\n";
            std::cout << "<td>" << size_str << "</td>\n";
            std::cout << "<td>" << modified_str << "</td>\n";
            std::cout << "</tr>\n";
        }
        
        std::cout << "</tbody>\n";
        std::cout << "</table>\n";
        
    } catch (const std::exception& e) {
        std::cout << "<div style='color: #ff5722; background: rgba(255,87,34,0.1); padding: 15px; border-radius: 10px;'>\n";
        std::cout << "<strong>Error:</strong> " << e.what() << "\n";
        std::cout << "</div>\n";
    }
    
    std::cout << "<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>\n";
    std::cout << "</div>\n";
    std::cout << "</body>\n";
    std::cout << "</html>\n";
    
    return 0;
} 