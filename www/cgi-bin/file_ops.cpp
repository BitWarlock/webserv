#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <dirent.h>           // opendir, readdir
#include <unistd.h>           // opendir, readdir
#include <sys/stat.h>         // stat, S_ISDIR, etc.
#include <sys/types.h>        // off_t
#include <ctime>              // localtime, strftime
#include <cerrno>
#include <cstring>            // strerror

// Use unsigned long long for large sizes (C++98 doesn't have uint64_t in std)
typedef unsigned long long uint64;

// Recursively compute total size of all regular files inside a directory
uint64 getDirectorySize(const std::string& path) {
    uint64 total = 0;
    DIR* dir = opendir(path.c_str());
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string full_path = path + "/" + name;
        struct stat buf;

        if (stat(full_path.c_str(), &buf) == 0) {
            if (S_ISREG(buf.st_mode)) {
                total += static_cast<uint64>(buf.st_size);
            } else if (S_ISDIR(buf.st_mode)) {
                total += getDirectorySize(full_path);  // recursive
            }
            // Ignore symlinks, FIFOs, sockets, etc.
        }
    }
    closedir(dir);
    return total;
}

// Format file size into human-readable string (B, KB, MB, ...)
std::string formatFileSize(uint64 size) {
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

// Check file type using stat
bool isDirectory(const std::string& path) {
    struct stat buf;
    return (stat(path.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode));
}

bool isSymlink(const std::string& path) {
    struct stat buf;
    return (lstat(path.c_str(), &buf) == 0 && S_ISLNK(buf.st_mode));
}

bool isRegularFile(const std::string& path) {
    struct stat buf;
    return (stat(path.c_str(), &buf) == 0 && S_ISREG(buf.st_mode));
}

// Get file type as string
std::string getFileType(const std::string& path) {
    if (isDirectory(path)) return "Directory";
    if (isSymlink(path)) return "Symlink";
    if (isRegularFile(path)) return "File";
    return "Other";
}

// Get current working directory
std::string getCurrentPath() {
    char buffer[1024];
    if (getcwd(buffer, sizeof(buffer)))
        return std::string(buffer);
    return "./";
}

// Format timestamp (last modified)
std::string formatTime(time_t rawtime) {
    struct tm* timeinfo = localtime(&rawtime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
    return std::string(buffer);
}

// Get last write time of a file
time_t getLastWriteTime(const std::string& path) {
    struct stat buf;
    if (stat(path.c_str(), &buf) == 0)
        return buf.st_mtime;
    return 0;
}

// Struct to hold directory entry info
struct Entry {
    std::string name;
    std::string path;
    bool is_dir;
    bool is_link;
    bool is_file;
    uint64 size;  // For files: file size; for dirs: recursive content size

    Entry(const std::string& n, const std::string& p)
        : name(n), path(p), is_dir(false), is_link(false), is_file(false), size(0)
    {
        struct stat buf;
        if (lstat(p.c_str(), &buf) == 0) {
            is_dir = S_ISDIR(buf.st_mode);
            is_link = S_ISLNK(buf.st_mode);
            is_file = S_ISREG(buf.st_mode);
        }

        if (is_file) {
            size = static_cast<uint64>(buf.st_size);
        } else if (is_dir) {
            size = getDirectorySize(p);  // compute recursive size
        }
        // size remains 0 for symlinks and special files
    }
};

// Sort: directories first, then alphabetically
bool compareEntries(const Entry& a, const Entry& b) {
    if (a.is_dir && !b.is_dir) return true;
    if (!a.is_dir && b.is_dir) return false;
    return a.name < b.name;
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
        std::string current_path = getCurrentPath();
        std::vector<Entry> entries;

        DIR* dir = opendir(current_path.c_str());
        if (!dir) {
            std::cout << "<div style='color: #ff5722; background: rgba(255,87,34,0.1); padding: 15px; border-radius: 10px;'>\n";
            std::cout << "<strong>Error:</strong> Cannot open directory: " << strerror(errno) << "\n";
            std::cout << "</div>\n";
        } else {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                std::string name = entry->d_name;
                if (name == "." || name == "..") continue;

                std::string full_path = current_path + "/" + name;
                entries.push_back(Entry(name, full_path));
            }
            closedir(dir);
        }

        // Sort: directories first, then by name
        std::sort(entries.begin(), entries.end(), compareEntries);

        // Calculate statistics
        int total_items = entries.size();
        int total_dirs = 0, total_files = 0, total_symlinks = 0;
        uint64 total_data_size = 0;

        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].is_dir) {
                total_dirs++;
            } else if (entries[i].is_file) {
                total_files++;
                total_data_size += entries[i].size;
            } else if (entries[i].is_link) {
                total_symlinks++;
            }
        }

        // Display statistics
        std::cout << "<div class='stats-grid'>\n";
        std::cout << "<div class='stat-card'>\n";
        std::cout << "<div class='stat-number'>" << total_items << "</div>\n";
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
        std::cout << "<div class='stat-number'>" << formatFileSize(total_data_size) << "</div>\n";
        std::cout << "<div class='stat-label'>Total Size</div>\n";
        std::cout << "</div>\n";
        std::cout << "</div>\n";

        // Display current directory
        std::cout << "<div style='background: rgba(255,255,255,0.1); padding: 15px; border-radius: 10px; margin-bottom: 20px;'>\n";
        std::cout << "<strong style='color: #9c27b0;'>Current Directory:</strong> " << current_path << "\n";
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

        for (size_t i = 0; i < entries.size(); ++i) {
            const Entry& e = entries[i];
            std::string size_str = "N/A";
            std::string modified_str = "N/A";

            // Use precomputed size (file or recursive dir size)
            if (e.is_file || e.is_dir) {
                size_str = formatFileSize(e.size);
            } else if (e.is_link) {
                size_str = "Link";
            }

            time_t mtime = getLastWriteTime(e.path);
            if (mtime != 0) {
                modified_str = formatTime(mtime);
            }

            std::string type_class = "type-other";
            if (e.is_dir) type_class = "type-dir";
            else if (e.is_file) type_class = "type-file";

            std::cout << "<tr>\n";
            std::cout << "<td>" << e.name << "</td>\n";
            std::cout << "<td><span class='file-type " << type_class << "'>" << getFileType(e.path) << "</span></td>\n";
            std::cout << "<td>" << size_str << "</td>\n";
            std::cout << "<td>" << modified_str << "</td>\n";
            std::cout << "</tr>\n";
        }

        std::cout << "</tbody>\n";
        std::cout << "</table>\n";

    } catch (...) {
        std::cout << "<div style='color: #ff5722; background: rgba(255,87,34,0.1); padding: 15px; border-radius: 10px;'>\n";
        std::cout << "<strong>Error:</strong> An unknown error occurred.\n";
        std::cout << "</div>\n";
    }

    std::cout << "<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>\n";
    std::cout << "</div>\n";
    std::cout << "</body>\n";
    std::cout << "</html>\n";

    return 0;
}
