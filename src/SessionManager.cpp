/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 10:00:00 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/01/27 10:00:00 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/SessionManager.hpp"
#include <iomanip>
#include <algorithm>

SessionManager::SessionManager(const std::string& session_dir, int timeout) 
	: session_dir(session_dir), session_timeout(timeout)
{
	struct stat st;
	if (stat(session_dir.c_str(), &st) != 0)
		mkdir(session_dir.c_str(), 0755);
	
	session_file_path = session_dir + "/sessions.dat";
	loadSessionsFromFile();
}

SessionManager::~SessionManager()
{
	saveAllSessions();
}

std::string	SessionManager::generateSessionId()
{
	std::stringstream ss;
	time_t now = time(NULL);
	ss << "sess_" << now << "_" << rand();
	return ss.str();
}

std::string	SessionManager::hashSessionId(const std::string& id)
{
	unsigned long hash = 5381;
	for (size_t i = 0; i < id.length(); i++)
		hash = ((hash << 5) + hash) + id[i];
	
	std::ostringstream oss;
	oss << "sess_" << hash;
	return oss.str();
}

std::string	SessionManager::createSession(const std::string& user_agent, const std::string& ip)
{
	std::string session_id = generateSessionId();
	SessionData session;
	session.user_agent = user_agent;
	session.ip_address = ip;
	session.created_at = time(NULL);
	session.last_accessed = time(NULL);
	
	sessions[session_id] = session;
	saveSession(session_id);
	
	return session_id;
}

SessionData*	SessionManager::getSession(const std::string& session_id)
{
	std::map<std::string, SessionData>::iterator it = sessions.find(session_id);
	if (it != sessions.end())
	{
		time_t now = time(NULL);
		if (now - it->second.last_accessed < session_timeout)
		{
			it->second.last_accessed = now;
			return &(it->second);
		}
		else
		{
			sessions.erase(it);
			return NULL;
		}
	}
	return NULL;
}

bool	SessionManager::hasSession(const std::string& session_id)
{
	return getSession(session_id) != NULL;
}

void	SessionManager::updateSessionAccess(const std::string& session_id)
{
	std::map<std::string, SessionData>::iterator it = sessions.find(session_id);
	if (it != sessions.end())
	{
		it->second.last_accessed = time(NULL);
		saveSession(session_id);
	}
}

void	SessionManager::setSessionData(const std::string& session_id, const std::string& key, const std::string& value)
{
	std::map<std::string, SessionData>::iterator it = sessions.find(session_id);
	if (it != sessions.end())
	{
		it->second.data[key] = value;
		it->second.last_accessed = time(NULL);
		saveSession(session_id);
	}
}

std::string	SessionManager::getSessionData(const std::string& session_id, const std::string& key)
{
	std::map<std::string, SessionData>::iterator it = sessions.find(session_id);
	if (it != sessions.end())
	{
		std::map<std::string, std::string>::iterator data_it = it->second.data.find(key);
		if (data_it != it->second.data.end())
		{
			it->second.last_accessed = time(NULL);
			return data_it->second;
		}
	}
	return "";
}

void	SessionManager::removeSession(const std::string& session_id)
{
	sessions.erase(session_id);
	std::string file_path = session_dir + "/" + hashSessionId(session_id) + ".session";
	unlink(file_path.c_str());
}

void	SessionManager::cleanup()
{
	cleanupExpiredSessions();
	saveAllSessions();
}

void	SessionManager::cleanupExpiredSessions()
{
	time_t now = time(NULL);
	std::map<std::string, SessionData>::iterator it = sessions.begin();
	while (it != sessions.end())
	{
		if (now - it->second.last_accessed > session_timeout)
		{
			std::string file_path = session_dir + "/" + hashSessionId(it->first) + ".session";
			unlink(file_path.c_str());
			sessions.erase(it++);
		}
		else
			++it;
	}
}

void	SessionManager::saveSession(const std::string& session_id)
{
	std::map<std::string, SessionData>::iterator it = sessions.find(session_id);
	if (it != sessions.end())
	{
		std::string file_path = session_dir + "/" + hashSessionId(session_id) + ".session";
		std::ofstream file(file_path.c_str());
		if (file.is_open())
		{
			file << serializeSession(it->second);
			file.close();
		}
	}
}

void	SessionManager::loadSessionsFromFile()
{
	DIR* dir = opendir(session_dir.c_str());
	if (dir)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string filename = entry->d_name;
			if (filename.length() > 8 && filename.substr(filename.length() - 8) == ".session")
			{
				std::string file_path = session_dir + "/" + filename;
				std::ifstream file(file_path.c_str());
				if (file.is_open())
				{
					std::string content((std::istreambuf_iterator<char>(file)),
									  std::istreambuf_iterator<char>());
					file.close();
					
					try
					{
						SessionData session = deserializeSession(content);
						std::string session_id = filename.substr(0, filename.length() - 8);
						sessions[session_id] = session;
					}
					catch (...)
					{
						unlink(file_path.c_str());
					}
				}
			}
		}
		closedir(dir);
	}
}

void	SessionManager::saveAllSessions()
{
	for (std::map<std::string, SessionData>::iterator it = sessions.begin(); it != sessions.end(); ++it)
		saveSession(it->first);
}

std::string	SessionManager::serializeSession(const SessionData& session)
{
	std::ostringstream oss;
	oss << session.created_at << "|";
	oss << session.last_accessed << "|";
	oss << session.user_agent << "|";
	oss << session.ip_address << "|";
	
	for (std::map<std::string, std::string>::const_iterator it = session.data.begin(); it != session.data.end(); ++it)
		oss << it->first << ":" << it->second << ";";
	
	return oss.str();
}

SessionData	SessionManager::deserializeSession(const std::string& data)
{
	SessionData session;
	std::istringstream iss(data);
	std::string line;
	
	if (std::getline(iss, line, '|'))
		session.created_at = std::atoi(line.c_str());
	if (std::getline(iss, line, '|'))
		session.last_accessed = std::atoi(line.c_str());
	if (std::getline(iss, line, '|'))
		session.user_agent = line;
	if (std::getline(iss, line, '|'))
		session.ip_address = line;
	
	std::string data_str;
	if (std::getline(iss, data_str))
	{
		std::istringstream data_iss(data_str);
		std::string item;
		while (std::getline(data_iss, item, ';'))
		{
			size_t pos = item.find(':');
			if (pos != std::string::npos)
			{
				std::string key = item.substr(0, pos);
				std::string value = item.substr(pos + 1);
				session.data[key] = value;
			}
		}
	}
	
	return session;
} 
