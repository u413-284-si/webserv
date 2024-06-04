#include "../inc/Server.hpp"

/**
 * @brief Set the socket to nonblocking mode
 * 
 * Use fcntl to manipulate fd/sockets. Retrieve the currently set flags with
 * F_GETFL. Set the flag with with F_SETFL. Use bitwise OR to set the 
 * O_NONBLOCK bit in "flags" while preserving all other bits.
 * @param sock	Socket to be manipulated
 */
static void setNonblocking(int sock)
{
	int	flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		throw std::runtime_error("fcntl(F_GETFL)");
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl(F_SETFL)");
	}
}

Server::Server()
{
    // Create server socket
	_serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSock < 0) {
		throw std::runtime_error("socket");
	}

	// Set server socket to non-blocking
	setNonblocking(_serverSock);

	// Bind server socket
    struct sockaddr_in	server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if (bind(_serverSock, (struct sockaddr*)&server_addr, sizeof(server_addr))
		< 0) {
  		close(_serverSock);
		throw std::runtime_error("bind");
	}

	// Listen on server socket
	if (listen(_serverSock, 10) < 0) {
        close(_serverSock);
		throw std::runtime_error("listen");
	}

	// Create epoll instance
	_epfd = epoll_create1(0);
	if (_epfd < 0) {
        close(_serverSock);
		throw std::runtime_error("epoll_create1");
	}

	// Add server socket to epoll instance
	_ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
	_ev.data.fd = _serverSock;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _serverSock, &_ev) < 0) {
        close(_serverSock);
		close(_epfd);
		throw std::runtime_error("epoll_ctl");
	}
}

Server::~Server()
{
    close(_serverSock);
    close(_epfd);
}

void    Server::run(){
    while (1) {
        // Blocking call to epoll_wait
        _nfds = epoll_wait(_epfd, _events, MAX_EVENTS, -1);
        if (_nfds < 0) {
            close(_serverSock);
            close(_epfd);
            throw std::runtime_error("epoll_wait");
        }

        for (int n = 0; n < _nfds; ++n) {
            if (_events[n].data.fd == _serverSock)
                acceptConnection();
            else
                handleConnections(n);
        }
    }
}

void    Server::acceptConnection(){
    // Accept new connections
    while (1) {
        socklen_t	addr_len = sizeof(_clientAddr);
        _clientSock = accept(
            _serverSock, (struct sockaddr*)&_clientAddr, &addr_len);
        if (_clientSock < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // No more pending connections
            } else {
                std::cerr << "error: accept\n";
                break;
            }
        }

        // Set client socket to non-blocking
        setNonblocking(_clientSock);

        // Add client socket to epoll instance
        _ev.events = EPOLLIN | EPOLLET;
        _ev.data.fd = _clientSock;
        if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _clientSock, &_ev) < 0) {
            std::cerr << "error: failed to add _clientSock to epoll\n";
            close(_clientSock);
        }
    }
}

void    Server::handleConnections(int index){
     // Handle client data
    int _clientSock = _events[index].data.fd;
    while (1) {
        char	buffer[1024];
        int		bytesRead = read(_clientSock, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // No more data to read
            } else {
                std::cerr << "error: read\n";
                close(_clientSock);
                break;
            }
        } else if (bytesRead == 0) {
            // Connection closed by client
            close(_clientSock);
            break;
        } else {
            // Echo data back to client
            write(_clientSock, buffer, bytesRead);
        }
    }
}
