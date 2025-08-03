/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 10:00:00 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/01/27 10:00:00 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <map>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <dirent.h>
#include <stdlib.h>

struct SessionData
{
	std::map<std::string, std::string>	data;
	time_t								created_at;
	time_t								last_accessed;
	std::string							user_agent;
	std::string							ip_address;
	
	SessionData() : created_at(time(NULL)), last_accessed(time(NULL)) {}
};

class SessionManager
{
	private:
		std::map<std::string, SessionData>	sessions;
		std::string							session_file_path;
		std::string							session_dir;
		int									session_timeout;
		
		std::string	generateSessionId();
		std::string	hashSessionId(const std::string& id);
		void		loadSessionsFromFile();
		void		saveSessionsToFile();
		void		cleanupExpiredSessions();
		std::string	serializeSession(const SessionData& session);
		SessionData	deserializeSession(const std::string& data);
		
	public:
		SessionManager(const std::string& session_dir = "/tmp/webserv_sessions", 
					  int timeout = 86400);
		~SessionManager();
		
		std::string		createSession(const std::string& user_agent, const std::string& ip);
		SessionData*	getSession(const std::string& session_id);
		bool			hasSession(const std::string& session_id);
		void			updateSessionAccess(const std::string& session_id);
		void			setSessionData(const std::string& session_id, const std::string& key, const std::string& value);
		std::string		getSessionData(const std::string& session_id, const std::string& key);
		void			removeSession(const std::string& session_id);
		void			cleanup();
		
		void	saveSession(const std::string& session_id);
		void	loadAllSessions();
		void	saveAllSessions();
};

#endif 