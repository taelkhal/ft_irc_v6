#include "multuplixing.hpp"
#include "../Server.hpp"
#include "../irc.hpp"
#include "../parse/parse.hpp"
#define welcome "      \\  \\     |\\  \\|\\  ___ \\ |\\  \\     |\\   ____|\\|\\   __  \\|\\   _ \\  _   \\|\\  ___ \\     \\\n" \
                " \\  \\    \\ \\  \\ \\   __/|\\ \\  \\    \\ \\  \\___|\\ \\  \\|\\  \\ \\  \\\\\\__\\ \\  \\ \\   __/|    \\\n" \
                "  \\ \\  \\  __\\ \\  \\ \\  \\_|/_\\ \\  \\    \\ \\  \\    \\ \\  \\\\\\  \\ \\  \\|__| \\  \\ \\  \\_|/__  \\\n" \
                "   \\ \\  \\|\\__\\_\\  \\ \\  \\_|\\ \\ \\  \\____\\ \\  \\____\\ \\  \\\\\\  \\ \\  \\    \\ \\  \\ \\  \\_|\\ \\ \\\n" \
                "    \\ \\____________\\ \\_______\\ \\_______\\ \\_______\\ \\_______\\ \\__\\    \\ \\__\\ \\_______\\\n" \
                "     \\|____________|\\|_______|\\|_______|\\|_______|\\|_______|\\|__|     \\|__|\\|_______|\r\n"


void send_message(int client_socket, const char* message) {
    send(client_socket, message, std::strlen(message), 0);
}
/*
:server.name 001 yournick :Welcome to the IRC Network yournick!username@hostname
:server.name 002 yournick :Your host is server.name, running version <version>
:server.name 003 yournick :This server was created <date>
:server.name 004 yournick server.name <version> <user_modes> <channel_modes>
*/

void send_welcome_message(int client_socket, server *ser, std::string nick) {
    (void )ser;
    std::string tmp = ":irc.ChatWladMina 001 " + nick + " :Welcome to the ChatWladMina Network, " + nick + "\r\n";
    // std::string = "<client>:Welcome to the " + ser->name + ""
    send_message(client_socket, tmp.c_str());
}

void send_host_info(int client_socket, server *ser, std::string nick) {
    std::string tmp = ":irc.ChatWladMina 002 " + nick + " :Your host is ChatWladMina, running version 4.5" + "\r\n";
    (void )ser;
    send_message(client_socket, tmp.c_str());
}

// Function to send server creation date (RPL_CREATED)
void send_creation_date(int client_socket, server *ser, std::string nick) {
    (void )ser;
    std::string tmp = ":irc.ChatWladMina 003 " + nick + " :This server was created " + "\r\n";
    send_message(client_socket, tmp.c_str());
}

// Function to send server information (RPL_MYINFO)
void send_server_info(int client_socket, server *ser, std::string nick) {
    (void )ser;
    std::string tmp = ":irc.ChatWladMina 004 " + nick + " ChatWladMina ChatWladMina(enterprise)-2.3(12)-netty(5.4c)-proxy(0.9) ChatWladMina" + "\r\n";
    // std::cout << tmp << std::endl;
    // send(client_socket, tmp.c_str(), tmp.length(), 0);
    send_message(client_socket, tmp.c_str());
}

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
client *AddClient(int newFd, server *ser)
{
    client *tmp = new client;
    addPollfd(newFd, ser);
    tmp->fd = newFd;
    // send_welcome_message(newFd);
    // send_host_info(newFd);
    // send_creation_date(newFd);
    // send_server_info(newFd);
    return tmp;
}

client *getClient(int fd, std::vector<client> &mycl)
{
    std::vector<client>::iterator it = mycl.begin();
    for (; it != mycl.end(); it++)
    {
        if (fd == it->fd)
            return &(*it);
    }
    return &(*it);
}

void handleCtrlZ(int signum) {
    std::cout << "Received signal: " << signum << std::endl;
    if(signum == SIGTSTP) {
        std::cout << "Ctrl+Z pressed. Suspending process..." << std::endl;
        // Your suspension logic here
        // For example, you can pause execution using sleep()
        // Or you can perform cleanup and then pause
        // For demonstration, let's just sleep for a while
        // sleep(5); // Sleeping for 5 seconds
        std::cout << "Process resumed after suspension." << std::endl;
    }
}

// class Server;
void mult(server *ser) {
    Server s;
    (void )s;
    char buff[1024];
    ser->indice = 0;
    std::vector <client> mycl;
    printf("start  %lu\n",ser->_fd.size());
    signal(SIGTSTP, handleCtrlZ);
    while (1) {
        // signal( ,han÷)
        if (poll(ser->_fd.data(), ser->_fd.size(), -1) == -1)
            throw std::runtime_error("poll");
        for (size_t i = 0; i < ser->_fd.size(); i++) {
            if (ser->_fd[i].revents & POLLIN) {
                if (ser->sock == ser->_fd[i].fd)
                {
                    int newFd = accept(ser->sock , NULL, NULL);
                    // printf("NEW CONNECTION->%d\n",newFd);
                    std::string t = welcome;
                    if (newFd == -1)
                        throw std::runtime_error("ERROR IN ACCEPTING\n");
                    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("Error: fcntl() failed");
                    std::string r = welcome;
                    send_message(newFd, r.c_str());
                    client *tmp = AddClient(newFd, ser);
                    tmp->setIndice(0);
                    send_message(newFd, "pleas enter ur pass:(in this format /quote pass [PASS]\r\n");
                    mycl.push_back(*tmp);
                    parseBuff(newFd, buff, ser, tmp, &s, mycl);
                }      
                else {
                    memset(buff, 0, std::strlen(buff));
                    int nbytes = recv(ser->_fd[i].fd, buff, sizeof(buff), 0);
                    if (nbytes == -1)
                        throw std::runtime_error("error in recv\n");
                    else if (nbytes == 0) {
                        // Client disconnected
                        mycl = removeClient(mycl, ser->_fd[i].fd);
                        ser->_fd.erase(ser->_fd.begin() + i);
                    } 
                    else {
                        // Data received
                        // buff[nbytes] = '\0';
                        parseBuff(ser->_fd[i].fd, buff, ser, getClient(ser->_fd[i].fd, mycl), &s, mycl);
                        // printf("\nReceived data from client %d:%s\n", ser->_fd[i].fd, buff);
                        s.handleClientData(ser->_fd[i].fd, nbytes, buff);
                    }
                }
            }
        }
    }
}

