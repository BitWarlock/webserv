#include "../inc/Response.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>

static std::string escapeHtml(const std::string& input)
{
	std::string out;
	out.reserve(input.size());
	for (size_t i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		switch (c)
		{
			case '&': out += "&amp;"; break;
			case '<': out += "&lt;"; break;
			case '>': out += "&gt;"; break;
			case '"': out += "&quot;"; break;
			case '\'': out += "&#39;"; break;
			default: out += c; break;
		}
	}
	return out;
}

static unsigned long long getDirectorySize(const std::string& path)
{
	unsigned long long total = 0;
	DIR* dir = opendir(path.c_str());
	if (!dir) return 0;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;
		std::string full = path;
		if (!full.empty() && full[full.size() - 1] != '/') full += "/";
		full += name;
		struct stat st;
		if (stat(full.c_str(), &st) == 0)
		{
			if (S_ISREG(st.st_mode)) total += static_cast<unsigned long long>(st.st_size);
			else if (S_ISDIR(st.st_mode)) total += getDirectorySize(full);
		}
	}
	closedir(dir);
	return total;
}

static std::string formatFileSize(unsigned long long size)
{
	const char* units[] = {"B", "KB", "MB", "GB", "TB"};
	int unit = 0;
	double v = static_cast<double>(size);
	while (v >= 1024.0 && unit < 4) { v /= 1024.0; unit++; }
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << v << " " << units[unit];
	return oss.str();
}

static std::string formatTime(time_t rawtime)
{
	struct tm* t = localtime(&rawtime);
	char buffer[20];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", t);
	return std::string(buffer);
}

struct DlEntry
{
	std::string name;
	std::string full_path;
	bool is_dir;
	bool is_file;
	unsigned long long size;
	time_t mtime;
};

static bool compareDlEntries(const DlEntry& a, const DlEntry& b)
{
	if (a.is_dir && !b.is_dir) return true;
	if (!a.is_dir && b.is_dir) return false;
	return a.name < b.name;
}

void    Response::GetListingPage(void)
{
	DIR* dir = opendir(path.c_str());
	if (!dir)
	{
		SetState(Forbidden);
		ResponseWithError(NONE);
		return;
	}

	std::vector<DlEntry> entries;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;
		DlEntry e;
		e.name = name;
		e.full_path = path;
		if (!e.full_path.empty() && e.full_path[e.full_path.size() - 1] != '/') e.full_path += "/";
		e.full_path += name;
		struct stat st;
		if (stat(e.full_path.c_str(), &st) == 0)
		{
			e.is_dir = S_ISDIR(st.st_mode);
			e.is_file = S_ISREG(st.st_mode);
			e.mtime = st.st_mtime;
			if (e.is_file) e.size = static_cast<unsigned long long>(st.st_size);
			else if (e.is_dir) e.size = getDirectorySize(e.full_path);
			else e.size = 0;
		}
		else
		{
			e.is_dir = false;
			e.is_file = false;
			e.mtime = 0;
			e.size = 0;
		}
		entries.push_back(e);
	}
	closedir(dir);

	std::sort(entries.begin(), entries.end(), compareDlEntries);

	int total_items = static_cast<int>(entries.size());
	int total_dirs = 0;
	int total_files = 0;
	unsigned long long total_size = 0;
	for (size_t i = 0; i < entries.size(); ++i)
	{
		if (entries[i].is_dir) total_dirs++;
		else if (entries[i].is_file) total_files++;
		total_size += entries[i].size;
	}

	std::string uri = Request.getUri();
	std::string uriEsc = escapeHtml(uri);
	std::string color = "#00897b";
	std::string linkColor = "#80cbc4";

	std::string parent = uri;
	if (parent.size() > 1 && parent[parent.size() - 1] == '/') parent.erase(parent.size() - 1);
	size_t slash = parent.find_last_of('/');
	if (slash == std::string::npos || slash == 0) parent = "/";
	else parent = parent.substr(0, slash + 1);

	std::ostringstream out;
	out << "<!DOCTYPE html>\n";
	out << "<html>\n";
	out << "<head>\n";
	out << "<title>Index of " << uriEsc << "</title>\n";
	out << "<style>\n";
	out << "body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }\n";
	out << ".container { max-width: 1000px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }\n";
	out << "h1 { color: " << color << "; text-align: center; margin-bottom: 20px; }\n";
	out << "a { color: " << linkColor << "; text-decoration: none; }\n";
	out << "a:hover { text-decoration: underline; }\n";
	out << ".nav { margin-bottom: 20px; }\n";
	out << ".back-btn { background: #666; color: white; padding: 8px 14px; text-decoration: none; border-radius: 5px; display: inline-block; margin-right: 10px; }\n";
	out << ".stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 30px; }\n";
	out << ".stat-card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid " << color << "; text-align: center; }\n";
	out << ".stat-number { font-size: 2em; color: " << color << "; font-weight: bold; }\n";
	out << ".stat-label { color: #cccccc; margin-top: 5px; }\n";
	out << ".file-table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n";
	out << ".file-table th, .file-table td { padding: 12px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }\n";
	out << ".file-table th { background: rgba(0, 137, 123, 0.2); color: " << color << "; font-weight: bold; }\n";
	out << ".file-table tr:hover { background: rgba(255,255,255,0.05); }\n";
	out << ".file-type { padding: 4px 8px; border-radius: 4px; font-size: 0.8em; }\n";
	out << ".type-dir { background: #4caf50; color: white; }\n";
	out << ".type-file { background: #2196f3; color: white; }\n";
	out << ".type-other { background: #ff9800; color: white; }\n";
	out << "</style>\n";
	out << "</head>\n";
	out << "<body>\n";
	out << "<div class='container'>\n";
	out << "<h1>Index of " << uriEsc << "</h1>\n";
	out << "<div class='nav'>\n";
	out << "<a class='back-btn' href='" << escapeHtml(parent) << "'>..</a>";
	out << "<a class='back-btn' href='/'>/</a>";
	out << "</div>\n";

	out << "<div class='stats-grid'>\n";
	out << "<div class='stat-card'>\n";
	out << "<div class='stat-number'>" << total_items << "</div>\n";
	out << "<div class='stat-label'>Total Items</div>\n";
	out << "</div>\n";
	out << "<div class='stat-card'>\n";
	out << "<div class='stat-number'>" << total_dirs << "</div>\n";
	out << "<div class='stat-label'>Directories</div>\n";
	out << "</div>\n";
	out << "<div class='stat-card'>\n";
	out << "<div class='stat-number'>" << total_files << "</div>\n";
	out << "<div class='stat-label'>Files</div>\n";
	out << "</div>\n";
	out << "<div class='stat-card'>\n";
	out << "<div class='stat-number'>" << formatFileSize(total_size) << "</div>\n";
	out << "<div class='stat-label'>Total Size</div>\n";
	out << "</div>\n";
	out << "</div>\n";

	out << "<table class='file-table'>\n";
	out << "<thead>\n<tr>\n<th>Name</th>\n<th>Type</th>\n<th>Size</th>\n<th>Last Modified</th>\n</tr>\n</thead>\n";
	out << "<tbody>\n";
	for (size_t i = 0; i < entries.size(); ++i)
	{
		const DlEntry& e = entries[i];
		std::string size_str = formatFileSize(e.size);
		std::string modified_str = e.mtime ? formatTime(e.mtime) : "N/A";
		std::string type_class = e.is_dir ? "type-dir" : (e.is_file ? "type-file" : "type-other");
		std::string type_label = e.is_dir ? "Directory" : (e.is_file ? "File" : "Other");
		std::string href = uri + e.name;
		if (e.is_dir) href += "/";
		out << "<tr>\n";
		out << "<td><a href='" << escapeHtml(href) << "'>" << escapeHtml(e.name) << (e.is_dir ? "/" : "") << "</a></td>\n";
		out << "<td><span class='file-type " << type_class << "'>" << type_label << "</span></td>\n";
		out << "<td>" << size_str << "</td>\n";
		out << "<td>" << modified_str << "</td>\n";
		out << "</tr>\n";
	}
	out << "</tbody>\n";
	out << "</table>\n";
	out << "</div>\n";
	out << "</body>\n";
	out << "</html>\n";

	std::string page = out.str();
	std::ostringstream len;
	len << page.size();
	responseBody = "HTTP/1.1 200 OK\r\n";
	responseBody += "Content-Type: text/html\r\n";
	responseBody += "Content-Length: " + len.str() + "\r\n";
	addCookiesToHeaders();
	responseBody += "Connection: close\r\n\r\n";
	responseBody += page;
} 