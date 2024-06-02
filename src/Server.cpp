#include "Server.hpp"

static void set_nonblocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		throw std::runtime_error("fcntl(F_GETFL)");
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl(F_SETFL)");
	}
}

Server::Server(){
    // Create server socket
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		throw std::runtime_error("socket");
	}

	// Set server socket to non-blocking
	set_nonblocking(server_sock);

	// Bind server socket
    struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))
		< 0) {
  		close(server_sock);
		throw std::runtime_error("bind");
	}

	// Listen on server socket
	if (listen(server_sock, 10) < 0) {
        close(server_sock);
		throw std::runtime_error("listen");
	}

	// Create epoll instance
	epfd = epoll_create1(0);
	if (epfd < 0) {
        close(server_sock);
		throw std::runtime_error("epoll_create1");
	}

	// Add server socket to epoll instance
	ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
	ev.data.fd = server_sock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &ev) < 0) {
        close(server_sock);
		close(epfd);
		throw std::runtime_error("epoll_ctl");
	}
}

Server::~Server(){
    close(server_sock);
    close(epfd);
}

void    Server::run(){
    while (1) {
        // Blocking call to epoll_wait
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            close(server_sock);
            close(epfd);
            throw std::runtime_error("epoll_wait");
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_sock)
                acceptConnection();
            else
                handleConnections(n);
        }
    }
}

void    Server::acceptConnection(){
    // Accept new connections
    while (1) {
        socklen_t addr_len = sizeof(client_addr);
        client_sock = accept(
            server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // No more pending connections
            } else {
                std::cerr << "error: accept\n";
                break;
            }
        }

        // Set client socket to non-blocking
        set_nonblocking(client_sock);

        // Add client socket to epoll instance
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_sock;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &ev) < 0) {
            std::cerr << "error: epoll_ctl: client_sock\n";
            close(client_sock);
        }
    }
}

void    Server::handleConnections(int index){
     // Handle client data
    int client_sock = events[index].data.fd;
    while (1) {
        char buffer[1024];
        int bytes_read = read(client_sock, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // No more data to read
            } else {
                std::cerr << "error: read\n";
                close(client_sock);
                break;
            }
        } else if (bytes_read == 0) {
            // Connection closed by client
            close(client_sock);
            break;
        } else {
            // Echo data back to client
            write(client_sock, buffer, bytes_read);
        }
    }
}
