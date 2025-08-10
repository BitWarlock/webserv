/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/08/08 22:17:27 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include "../inc/SetupServers.hpp"

SetupServers::SetupServers(Config& config) : config(config), sock_number(0), fd_epoll(0), endpoints(0), sessionManager("/tmp/webserv_sessions", 86400)
{
	this->StartSetup();
}

SetupServers::~SetupServers()
{
	for (size_t i(0); i < sock_number; i++)
		close(fd_sockets[i]);
}

void    SetupServers::CheckPortIp(const std::string& host, const std::string& port, size_t pos_server, size_t pos_listen)
{
	std::vector<Server*>& servers = config.getServers();

	for (size_t i(0); i <= pos_server; i++)
	{
		for (size_t s(0); s < servers[i]->getListen().size(); s++)
		{
			if (i == pos_server && s == pos_listen)
				return ;

			std::string& _port_ = const_cast<std::string&> (servers[i]->getListen()[s].second);
			std::string& _host_ = const_cast<std::string&> (servers[i]->getListen()[s].first);
			std::string&    _port = const_cast<std::string&> (port);

			if (_port_ == port || _port_ == _port + "T")
			{
				if (host == "0.0.0.0" && _port_[_port_.size() - 1] != 'T' && _host_ != "0.0.0.0")
					_port_ += "T";
				else if (_host_ == host || _host_ == "0.0.0.0")
				{
					_port += "T";
					return ;
				}
			}
		}
	}
}

void    SetupServers::FlagSharedPortIp(void)
{
	std::vector<Server*>& servers = config.getServers();

	for (size_t i(0); i < servers.size(); i++)
	{
		for (size_t s(0); s < servers[i]->getListen().size(); s++)
		{
			const std::string &host = servers[i]->getListen()[s].first;
			const std::string &port = servers[i]->getListen()[s].second;

			CheckPortIp(host, port, i, s);
		}
	}
}

void    SetupServers::CreateSocket(Server& server)
{
	for (size_t i(0); i < server.getListen().size(); i++)
	{
		std::string port = server.getListen()[i].second;
		
		if (port[port.size() - 1] != 'T')
		{
			int fd_server = socket(AF_INET, SOCK_STREAM, 0);
			if (fd_server < 0)
			{
				std::cerr << YLW"Warning:make socket() function failed to create endpoint "
					<< "for " << server.getListen()[i].first << ":"<< port << ".\n" << RESET;
			}
			else
			{
				int opt = 1;
				setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
				this->fd_sockets.push_back(fd_server);
				this->servers[fd_server] = Server(server);			
				Advance();
			}
		}
	}
}

void    SetupServers::setAddrForBound(std::string& host, std::string& port, struct sockaddr_in& add_server)
{

	uint16_t            port_number = std::atoi(port.c_str());

	std::memset(&add_server, 0, sizeof(add_server));
	add_server.sin_family = AF_INET;
	add_server.sin_port = htons(port_number);
	add_server.sin_addr.s_addr = inet_addr(host.c_str());
}

void    SetupServers::Binding(Server& server, size_t index)
{
	size_t              s(0);

	for (size_t i(0); i < server.getListen().size(); i++)
	{
		struct sockaddr_in  add_server;
		std::string         host = server.getListen()[i].first;
		std::string         port = server.getListen()[i].second;

		if (port[port.size() - 1] != 'T')
		{
			setAddrForBound(host, port, add_server);
			if (bind(fd_sockets[s + index],  (struct sockaddr*) &add_server, sizeof(add_server)))
			{
				EraseFd(fd_sockets[s + index]);
				std::cerr << YLW"Warning: bind() function failed to bound "
					<< host << ":"<< port << ".\n" << RESET;
			}
			s++;
		}
		else
		{
			// Remove the 'T' marker from the port string
			std::vector<std::pair<std::string, std::string> >& listen_vec = server.getListen();
			listen_vec[i].second.erase(port.size() - 1);
		}
	}
}

void    SetupServers::CreateEpoll(void)
{
	fd_epoll = epoll_create(MAX_EVENTS);

	if (fd_epoll < 0)
	{
		throw std::runtime_error("Warning: epoll_create() function failed to create epoll instance.\n");
	}
}

struct epoll_event    SetupServers::InitEvents(int fd, int event)
{
	struct epoll_event          ev;

	ev.data.fd = fd;
	ev.events = event;

	return (ev);
}

void    SetupServers::AddSocketToEpoll(int fd, int event, int job)
{    
	int return_value = fcntl(fd, F_SETFL, O_NONBLOCK);

	if (return_value == -1)
	{
		std::cerr << "Warning: fcntl() function failed to set non-blocking mode" << std::endl;
		return;
	}
	
	struct epoll_event          ev = InitEvents(fd, event);
	
	return_value = epoll_ctl(fd_epoll, job, fd, &ev);

	if (return_value == -1)
	{
		std::cerr << "Warning: epoll_ctl() function failed to monitor a socket." << std::endl;
		return;
	}
}

void	SetupServers::RemoveSocketFromEpoll(int fd, int job)
{
	int	return_value = epoll_ctl(fd_epoll, job, fd, NULL);

	if (return_value == -1)
	{
		std::cerr << "Warning: epoll_ctl() function failed to remove socket from monitoring." << std::endl;
		return;
	}
}

void    SetupServers::WaitEpoll(void)
{
	number_events = epoll_wait(fd_epoll, events, MAX_EVENTS, -1);

	if (number_events < 0)
	{
		std::cerr << "Warning: epoll_wait() function failed to wait for events." << std::endl;
		return;
	}
}

void    SetupServers::AcceptConnection(int fd)
{
	int fd_accept = accept(fd, NULL, 0);

	if (fd_accept == -1)
	{
		std::cerr << "Warning: accept() function failed to accept new connection." << std::endl;
		return;
	}

	fd_sockets.push_back(fd_accept);
	this->servers[fd_accept] = this->servers[fd];
	
	Advance();
}

void    SetupServers::EraseFd(int fd)
{
	std::vector<int>::iterator    target;

	target = find(fd_sockets.begin(), fd_sockets.end(), fd);

	if (target == fd_sockets.end())
		return ;

	close (fd);
	
	fd_sockets.erase(target);
	
	Retreat();
}

Server*	SetupServers::GetBlockServer(int block)
{
	std::map<int, Server>::iterator	it = servers.find(block);

	return (&(it->second));
}

void    SetupServers::Run(void)
{
	std::map<int, ParseRequest> Requests;
	std::map<int, Response>		Responses;

	CreateEpoll();

	for (size_t i(0); i < endpoints; i++)
		AddSocketToEpoll(fd_sockets[i], EPOLLIN, EPOLL_CTL_ADD);

	while (1337)
	{
		WaitEpoll();
		for (int i(0); i < number_events; i++)
		{
			int	fd = events[i].data.fd;
			if (events[i].events & (EPOLLERR | EPOLLHUP))
				EraseFd(fd);
			else if (events[i].events & EPOLLIN)
			{
				if (fd_sockets.begin() + endpoints != find(fd_sockets.begin(), fd_sockets.begin() + endpoints, fd))
				{
					AcceptConnection(fd);
					AddSocketToEpoll(fd_sockets.back(), EPOLLIN, EPOLL_CTL_ADD);
				}
				else
				{
					Requests[fd].startParse(fd, config, GetBlockServer(fd));
					if (Requests[fd].getParseState() == FINISH || Requests[fd].getParseState() == ERROR)
						AddSocketToEpoll(fd, EPOLLOUT, EPOLL_CTL_MOD);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				size_t bytes(0);
				handleSessionManagement(Requests[fd]);
				Responses[fd].setSessionManager(&sessionManager);
				Responses[fd].StartForResponse(Requests[fd], fd);
				std::string	ResBody = Responses[fd].GetResponseBody();
				while (bytes < ResBody.length())
				{
					size_t n(0);
					n = send(fd, ResBody.c_str(), ResBody.length(), MSG_NOSIGNAL);
					if (n == std::string::npos)
						break;
					bytes += n;
				}
				EraseFd(fd);
				Requests.erase(fd);
				Responses.erase(fd);
			}
		}
	}
}

void    SetupServers::Advance(void) {this->sock_number++;}

void    SetupServers::Retreat(void) {this->sock_number--;}

void    SetupServers::StartSetup(void)
{
	static size_t         index(0);
	std::vector<Server*>& servers = config.getServers();

	FlagSharedPortIp();

	for (size_t i(0); i < servers.size(); i++)
	{
		try {
			CreateSocket(*(servers[i]));
			Binding(*(servers[i]), index);
			while (index < sock_number)
			{
				if (listen(fd_sockets[index], SOMAXCONN) < 0)
				{
					EraseFd(fd_sockets[index]);
					std::cerr << YLW"Warning: listen() function failed to listen.\n"<< RESET;
				}
				index++;
			}
		}
		catch (...){}
	}

	endpoints = sock_number;

	Run();
}

void	SetupServers::handleSessionManagement(ParseRequest& request)
{
	std::string session_id = request.getCookie("session_id");
	
	if (!session_id.empty())
	{
		SessionData* session = sessionManager.getSession(session_id);
		if (session)
		{
			sessionManager.updateSessionAccess(session_id);
		}
	}

}
