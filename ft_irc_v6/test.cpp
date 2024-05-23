#include <iostream>
#include <fstream>
#include <vector>
#include <map>
// #include <thread>
// #include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstdlib>
#include <string>
#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include "poll.h"

class client{
    public:
        int fd;
        client(int fd){this->fd = fd;};

};
class server{
    public:
        int sock;
        int listen_fd;
        std::vector<struct pollfd> _fd;

};

void addPollfd(int fd, server *ser) {
    struct pollfd newPollfd;
    newPollfd.fd = fd;
    newPollfd.events = POLLIN;
    newPollfd.revents = 0;
    ser->_fd.push_back(newPollfd);
}

std::vector<client> removeClient(std::vector<client> mycl, int fd)
{
    for (std::vector<client>::iterator it = mycl.begin(); it != mycl.end(); it++)
    {
        if (fd == it->fd){
            mycl.erase(it);
            close (fd);
            return mycl;
        }
    }
    close (fd);
    return mycl;
}
client AddClient(int newFd, server *ser)
{
    client tmp(newFd);
    addPollfd(newFd, ser);
    send(newFd, "connection accepted\n", 21, 0);
    // sleep(5);
    send (newFd, "Pleas enter password of this server (eg:/quote pass [password]):", 65, 0);
    // exit (1);
    return tmp;
}

void getSocket(server *ser)
{
    if ((ser->sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        throw ("creating socket");
    }
    printf("socket for server is %d\n", ser->sock);
    int optval = 1;
    if (setsockopt(ser->sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        throw std::runtime_error("Error: setsockopt() failed");
    fcntl(ser->sock, F_SETFL, O_NONBLOCK);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9887);
    sa.sin_addr.s_addr = INADDR_ANY;
    if (bind(ser->sock, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        throw std::runtime_error("Error: failed to bind socket");
    };
    if ((ser->listen_fd = listen(ser->sock, SOMAXCONN)) == -1){
        close(ser->sock);
        throw std::runtime_error("error in listen");
    }
    addPollfd(ser->sock, ser);
    printf("END CRATING SOCKETS...\n");
}


void mult(server *ser) {


    char buff[1024];
    // ser->indice = 0;
    std::vector <client> mycl;
    printf("start  %lu\n",ser->_fd.size());
    // signal(SIGTSTP, handleCtrlZ);
    while (1) {
        // signal( ,han÷)
        if (poll(ser->_fd.data(), ser->_fd.size(), -1) == -1)
            throw std::runtime_error("poll");
        for (size_t i = 0; i < ser->_fd.size(); i++) {
            if (ser->_fd[i].revents & POLLIN) {
                if (ser->sock == ser->_fd[i].fd)
                {
                    printf("NEW CONNECTION\n");
                    int newFd = accept(ser->sock , NULL, NULL);
                    // std::string t = welcome;
                    if (newFd == -1)
                        throw std::runtime_error("ERROR IN ACCEPTING\n");
                    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("Error: fcntl() failed");
                    // send_welcome_message(newFd);
                    // send_host_info(newFd);
                    // send_creation_date(newFd);
                    // send_server_info(newFd);
                    send(newFd,  "welcome\n", 9, 0);
                    send(newFd, "Pleas enter password of this server:\n", 38, 0);
                    send(newFd, "salam", 6, 0);
                    // sleep(2);
                    mycl.push_back(AddClient(newFd, ser));
                    // send (newFd, "Pleas enter password of this server:", 37, 0);
                    // parseBuff(newFd, buff, ser);
                    // passMsg(newFd);
                }
                else {
                    int nbytes = recv(ser->_fd[i].fd, buff, sizeof(buff), 0);
                    if (nbytes == -1)
                        throw std::runtime_error("error in recv\n");
                    else if (nbytes == 0) {
                        // Client disconnected
                        mycl = removeClient(mycl, ser->_fd[i].fd);
                        ser->_fd.erase(ser->_fd.begin() + i);
                    } 
                    else {
                        // parseBuff(ser->_fd[i].fd, buff, ser);
                        // Data received
                        buff[nbytes] = '\0';
                        printf("\nReceived data from client %d:%s\n", ser->_fd[i].fd, buff);
                    }
                }
            }
        }
    }
}


int main()
{
    try
    {
        server *ser;
        ser = new server;
        getSocket(ser);
        mult(ser);

    }
    catch(const std::exception& e)
    {
        std::cout << "errno set to " <<  strerror(errno) << '\n';
        std::cerr << e.what() << '\n';
    }
    
}

