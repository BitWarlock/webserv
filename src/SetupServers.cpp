/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/17 18:26:33 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "../inc/SetupServers.hpp"

SetupServers::SetupServers(Config& config) : config(config), sock_number(0), fd_epoll(0), endpoints(0)
{
	this->StartSetup();
}

SetupServers::~SetupServers()
{
	for (size_t i(0); i < sock_number; i++)
		close(fd_sockets[i]);
	// delete &config;
}

void    SetupServers::CheckPortIp(const std::string& host, const std::string& port, size_t pos_server)
{
	std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

	for (size_t i(0); i < pos_server; i++)
	{
		for (size_t s(0); s < servers[i]->getListen().size(); s++)
		{
			if ((servers[i]->getListen()[s].first == host || host == "0.0.0.0")
					&& servers[i]->getListen()[s].second == port)
			{
				std::string&    _port = const_cast<std::string&> (port);
				_port += "T";
				return ;
			}
		}
	}
}

void    SetupServers::FlagSharedPortIp(void)
{
	std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

	for (size_t i(0); i < servers.size(); i++)
	{
		for (size_t s(0); s < servers[i]->getListen().size(); s++)
		{
			const std::string &host = servers[i]->getListen()[s].first;
			const std::string &port = servers[i]->getListen()[s].second;

			CheckPortIp(host, port, i);
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
				this->fd_sockets.push_back(fd_server);
				this->servers[fd_server] = Server(server);			
				Advance();
			}
		}
	}
}

void    SetupServers::setAddrForBound(std::string& host, std::string& port, struct sockaddr_in& add_server)
{

	uint16_t            port_number = atoi(port.c_str());

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
			std::string& str = const_cast<std::string&> ((server.getListen()[i].second));
			str.erase(port.size() - 1);
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
		throw std::runtime_error("Warning: fcntl() function failed to set non-blocking mode\n");
	}
	
	struct epoll_event          ev = InitEvents(fd, event);
	
	return_value = epoll_ctl(fd_epoll, job, fd, &ev);

	if (return_value == -1)
	{
		throw std::runtime_error("Warning: epoll_ctl() function failed to monitor a socket.\n");
	}
}

void	SetupServers::RemoveSocketFromEpoll(int fd, int job)
{
	int	return_value = epoll_ctl(fd_epoll, job, fd, NULL);

	if (return_value == -1)
	{
		throw std::runtime_error("Warning: epoll_ctl() function failed to monitor a socket.\n");
	}
}

void    SetupServers::WaitEpoll(void)
{
	number_events = epoll_wait(fd_epoll, events, MAX_EVENTS, -1);

	if (number_events < 0)
	{
		throw std::runtime_error("Warning: epoll_wait() function failed to waits for events.\n");
	}
}

void    SetupServers::AcceptConnection(int fd)
{
	int fd_accept = accept(fd, NULL, 0);

	if (fd_accept == -1)
	{
		throw std::runtime_error("Warning: accept() function failed to accept new connection.\n");
	}

	fd_sockets.push_back(fd_accept);
	
	servers[fd_accept] = servers[fd];
	
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

Server	SetupServers::GetBlockServer(int block)
{
	std::map<int, Server>::iterator	it = servers.find(block);

	return (it->second);
}

void SetupServers::Run(void)
{
	std::map<int, ParseRequest> Requests;
	std::map<int, HttpResponse> Responses;
	std::string input;

	CreateEpoll();

	for (size_t i(0); i < endpoints; i++)
	{
		std::cout << "Adding listening socket fd=" << fd_sockets[i] << " to epoll\n";
		AddSocketToEpoll(fd_sockets[i], EPOLLIN, EPOLL_CTL_ADD);
	}

	while (1337)
	{
		std::cout << "Waiting for events...\n";
		WaitEpoll();
		std::cout << "Processing " << number_events << " events\n";

		for (int i(0); i < number_events; i++)
		{
			int fd = events[i].data.fd;
			std::cout << "\nProcessing event on fd=" << fd 
				<< ", events=" << events[i].events << "\n";

			if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				std::cout << "Error or hangup event on fd=" << fd << "\n";
				EraseFd(fd);
				continue;
			}

			if (events[i].events & EPOLLIN)
			{
				if (fd_sockets.begin() + endpoints != find(fd_sockets.begin(), fd_sockets.begin() + endpoints, fd))
				{
					std::cout << "New connection on listening socket fd=" << fd << "\n";
					try
					{
						AcceptConnection(fd);
						std::cout << "Accepted new connection fd=" << fd_sockets.back() << "\n";
						AddSocketToEpoll(fd_sockets.back(), EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD);
					}
					catch (const std::exception& e)
					{
						std::cerr << "Error accepting connection: " << e.what() << "\n";
					}
				}
				else
				{
					std::cout << "Incoming data on client fd=" << fd << "\n";
					try
					{
						Requests[fd].startParse(fd, GetBlockServer(fd));
						std::cout << "Parse state: " << Requests[fd].getParseState() << "\n";

						if (Requests[fd].getParseState() == FINISH || Requests[fd].getParseState() == ERROR)
						{
							std::cout << "Request parsing complete, generating response...\n";
							Responses[fd].generateResponse(Requests[fd], GetBlockServer(fd));
							std::cout << "Response generated:\n" << Responses[fd].serialize() << "\n";

							AddSocketToEpoll(fd, EPOLLOUT | EPOLLRDHUP, EPOLL_CTL_MOD);
							std::cout << "Switched fd=" << fd << " to EPOLLOUT mode\n";
						}
					}
					catch (const std::exception& e)
					{
						std::cerr << "Error processing request: " << e.what() << "\n";
						EraseFd(fd);
					}
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				std::cout << "Sending response on fd=" << fd << "\n";
				try
				{
					std::string response = Responses[fd].serialize();
					std::cout << "Sending response (" << response.size() << " bytes):\n" 
						<< response.substr(0, 200) << (response.size() > 200 ? "..." : "") << "\n";

					ssize_t bytes_sent = send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
					std::cout << "Sent " << bytes_sent << " bytes\n";

					if (bytes_sent <= 0)
						throw std::runtime_error("Failed to send response");

					if (Responses[fd].isKeepAlive())
					{
						std::cout << "Keeping connection alive on fd=" << fd << "\n";
						Requests.erase(fd);
						Responses.erase(fd);
						AddSocketToEpoll(fd, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_MOD);
					}
					else
					{
						std::cout << "Closing connection on fd=" << fd << "\n";
						EraseFd(fd);
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "Error sending response: " << e.what() << "\n";
					EraseFd(fd);
				}
			}
		}
	}
}

void    SetupServers::Advance(void) {this->sock_number++;}

void    SetupServers::Retreat(void) {this->sock_number--;}

void    SetupServers::StartSetup(void)
{
	static size_t         index(0);
	std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

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
