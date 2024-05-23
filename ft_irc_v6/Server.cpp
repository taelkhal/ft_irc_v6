#include "Server.hpp"
#include "irc.hpp"
#include "multuplixing/multuplixing.hpp"

bool Server::_signal = false;

Server::Server() {}

Server::~Server() {}


void Server::set_av(std::string av)
{
    this->av = av;
}

std::string Server::get_av()
{
    return this->av;
}

std::string Server::get_admin(std::string channel_a)
{
    std::map<std::string, std::vector<std::string > >::iterator it;

    it = channel.find(channel_a);
    if (it != channel.end())
    {
        std::vector<std::string>::iterator it1 = it->second.begin();
        this->admin = *it1;
    }
    return (admin);
}


void Server::set_admin_perm(bool is_admin)
{
    this->admin_perm = is_admin;
}

bool Server::get_admin_perm()
{
    return this->admin_perm;
}

void Server::parseArgs(int ac, char **av) {
    if (ac != 3)
        throw std::runtime_error("Usage: ./ircserv <port> <password>");
    std::string port(av[1]);
    std::string pwd(av[2]);
    if (port.empty() || port.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error("Error: Invalid arguments");

    long _port = atol(av[1]);
    if (!(_port >= 1 && _port <= 65535))
        throw std::runtime_error("Error: Invalid arguments");

    if (pwd.empty())
        throw std::runtime_error("Error: Invalid arguments");

    this->_port = _port;
    this->_password = pwd;
}

void Server::receiveSignal(int signum) {
    _signal = true;
    (void)signum;
}

void Server::init() {
    signal(SIGINT, receiveSignal);
    signal(SIGQUIT, receiveSignal);

    createServerSocket();
    std::cout << ">>> SERVER STARTED <<<" << std::endl;
    std::cout << "Waiting for connections..." << std::endl;
}

void Server::run() {
    while (!_signal) {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret == -1)
            throw std::runtime_error("Error: poll() failed");

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocketFd)
                    handleClientConnection();
                // else
                //     handleClientData(_fds[i].fd);
            }
        }
    }
    closeFds();
}

void Server::createServerSocket() {
    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("Error: failed to create socket");

    int optval = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        throw std::runtime_error("Error: setsockopt() failed");

    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Error: fcntl() failed");

    bindServerSocket();

    if (listen(_serverSocketFd, SOMAXCONN) == -1)
        throw std::runtime_error("Error: listen() failed");

    addPollfd(_serverSocketFd, POLLIN, 0);
}

void Server::bindServerSocket() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(_port);
    sa.sin_addr.s_addr = INADDR_ANY;
    if (bind(_serverSocketFd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        throw std::runtime_error("Error: failed to bind socket");
    }
}

void Server::addPollfd(int fd, short events, short revents) {
    struct pollfd newPollfd;
    newPollfd.fd = fd;
    newPollfd.events = events;
    newPollfd.revents = revents;
    _fds.push_back(newPollfd);
}

void Server::handleClientConnection() {
    struct sockaddr_in client_addr;
    socklen_t clientAddrSize = sizeof(sockaddr_in);
    int newFd = accept(_serverSocketFd, (struct sockaddr *)&client_addr, &clientAddrSize);
    if (newFd == -1) {
        throw std::runtime_error("Error: accept() failed");
    }

    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) // set non-blocking to the new socket fd it's mean that the socket will not block the program
        throw std::runtime_error("Error: fcntl() failed");
    
    std::string entrepass = "Enter Password Please : \n";

    send(newFd, entrepass.c_str(), entrepass.size(), 0); 

    addPollfd(newFd, POLLIN, 0);
    _clients.push_back(Client(newFd, inet_ntoa((client_addr.sin_addr))));
    std::cout << "Client <" << newFd << "> Connected" << std::endl;
}

void Server::handleClientData(int fd, int nbytes , char *buffer) {
    std::string command;
    // char buffer[BUFFER_SIZE];
    // memset(buffer, 0, std::strlen(buffer));
    // printf("voilá:{%s}{%d}\n", buffer, fd);
    // fd_u = 0;
    

    // ssize_t nbytes; 
    // int is_invited = 0;
    // while ((nbytes = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        bool foundEof = false;
        for (ssize_t i = 0; i < nbytes; ++i) {
            if (buffer[i] == '\n') {
                foundEof = true;
                break;
            }
        }
    // printf("her\n");
        if (!foundEof) {
            buffer[nbytes] = '\0';
            command += buffer;
        } else {
            command.append(buffer, nbytes - 1);
            std::cout << "Received data from client " << fd << ": " << command << std::endl;
            printf("----->1{%s}\n", command.substr(0, 5).c_str());
            // printf("----->2{%s}\n", command.substr(0, 7).c_str());
            // process the command here

            // std::string user_c;
            // printf("pp\n");
            // this->fd_u = fd;
            // printf("ppw\n");
            bool is_admin = false;
            // std::string savenick;
            // std::cout << "fd_u" << fd_u << std::endl;
            
            if (command.substr(0, 5) == "join " || command.substr(0, 5) == "JOIN ")
            {
                std::istringstream iss(command);
                std::string cmd ,channelname, pass;
                iss >> cmd;
                iss >> channelname;

                std::getline(iss, pass); // Read the rest of the line as message text
                pass = pass.substr(1);
                pass = pass.substr(0, pass.size());
                // std::cout << "cmd :" << cmd << std::endl;
                // std::cout << "channelname :" << channelname << std::endl;
                pass = pass.substr(0, pass.size() - 1);
                // std::cout << "pass :" << pass << "." << std::endl;
                
                // exit(0);
                // size_t space_pos = command.find(' ');
                std::map<std::string, std::vector<std::string> >::iterator it;
                std::map<std::string, std::string>::iterator it_key;
                int check_key = 0;
                int key_valid = 0;
                if(channel_password.size() > 0)
                {
                    it_key = channel_password.find(channelname);
                    if (it_key != channel_password.end())
                        check_key = 1;
                    if (pass == it_key->second)
                        key_valid = 1;
                }
                std::map<std::string, int>::iterator it_limit;
                int limited = 0;
                if(channel_limit.size() > 0)
                {
                    it_limit = channel_limit.find(channelname);
                    if (it_limit != channel_limit.end())
                    {
                        it = channel.find(channelname);
                        int size = it->second.size();
                        if(it_limit->second == size)
                            limited = 1;
                    }
                }
                it = invited.find(channelname);
                int check = 0;
                if (it == invited.end())
                    check = 1;
                int check_invited = if_member_invited(fd, channelname);
                if((check == 0 && check_invited == 1) || check == 1)
                {
                    if((check_key == 1 && key_valid == 1) || check_key == 0)
                    {
                        if(limited == 0)
                        {
                            if (cmd == "join" || cmd =="JOIN") {
                                std::string user_c;

                                for (std::map<std::string, std::vector<int> >::iterator it = nick_fd.begin(); it != nick_fd.end(); ++it) 
                                {
                                    std::vector<int>::iterator vec_it = std::find(it->second.begin(), it->second.end(), fd);
                                    if (vec_it != it->second.end()) {
                                        user_c = it->first;
                                        break;
                                    }
                                }
                                std::cout << "user:" << user_c << std::endl;

                                create_channel(channelname, user_c, fd);

                                for (std::map<std::string, std::vector<int> >::iterator it = nick_fd.begin(); it != nick_fd.end(); ++it) 
                                {
                                    std::vector<int>::iterator vec_it = std::find(it->second.begin(), it->second.end(), fd);
                                    if (vec_it != it->second.end()) 
                                    {
                                        user_c = it->first;
                                        // Check if the user is an admin
                                        std::map<std::string, std::vector<std::string> >::iterator it_1 = channel.find(channelname);
                                        if (it_1 != channel.end()) 
                                        {
                                            // Iterate over all members of the channel
                                            for (size_t i = 0; i < it_1->second.size(); ++i) 
                                            {
                                                if (it_1->second[i][0] == '@' && it_1->second[i].substr(1) == user_c) 
                                                {
                                                    std::cout << user_c << " is an admin" << std::endl;
                                                    is_admin = true;
                                                    set_admin_perm(is_admin);
                                                    // set_admin(channelname);
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                                std::string join_msg = ":" + user_c + " JOIN " + channelname + "\r\n";
                                // std::string mode_msg = ":irc.ChatWladMina MODE " + channelname + " +nt\r\n";

                                std::map<std::string, std::vector<std::string> >::iterator it_1 = channel.find(channelname);
                                std::string names_msg = ":irc.ChatWladMinah 353 " + user_c +  " = " + channelname + " :";
                                std::vector<std::string>::iterator it_v;
                                for (it_v = it_1->second.begin(); it_v != it_1->second.end(); it_v++)
                                {
                                    if (is_admin == true) 
                                        names_msg += "@" + *it_v;
                                    else 
                                        names_msg += *it_v;
                                    if (it_v < it_1->second.end() - 1)
                                        names_msg += " ";
                                }
                                names_msg += "\n";

                                std::string endOfNamesMessage = ":irc.ChatWladMina 366 " + user_c + " " + channelname + " :End of /NAMES list.\r\n";
                                std::string channel_msg = ":irc.ChatWladMina 354 " + channelname + "\r\n";

                                // Send messages to the client
                                send(fd, join_msg.c_str(), join_msg.length(), 0);
                                it = channel.find(channelname);
                                if (it->second.size() == 1)
                                {
                                    std::string mode_msg = ":irc.ChatWladMina MODE " + channelname + " +nt\r\n";
                                    send(fd, mode_msg.c_str(), mode_msg.length(), 0);
                                }
                                else
                                {
                                    std::map<std::string, std::string>::iterator it;

                                    it = channel_topic.find(channelname);
                                    std::string topic;
                                    if (it == channel_topic.end())
                                        topic = "has no topic";
                                    else
                                        topic = it->second;
                                    std::string topic_msg = ":irc.ChatWladMina 332 " + user_c + " " + channelname + " :" + topic + " https://irc.com\r\n";
                                    send(fd, topic_msg.c_str(), topic_msg.length(), 0);
                                }
                                send(fd, names_msg.c_str(), names_msg.length(), 0);
                                send(fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
                                send(fd, channel_msg.c_str(), channel_msg.length(), 0);
                            }
                        }
                    }
                }
                /////**********************************************************************/////
                /////**********************************************************************/////
            }
            else if (command.substr(0, 7) == "privmsg" || command.substr(0, 7) == "PRIVMSG")
                    processMessage(command, fd);
            else if (command.substr(0, 5) == "kick " || command.substr(0, 5) == "KICK ")
            {
                // std::string p_msg;

                // size_t space_pos = command.find(' ');
                // if (space_pos != std::string::npos)
                // {
                //     p_msg = command.substr(space_pos + 1);
                //     p_msg = p_msg.substr(0, p_msg.size() - 1);
                // }
                std::istringstream iss(command);
                std::string command, channel_kicked_from, user_kicked, reason;
                // Parse the message
                iss >> command;

                iss >> channel_kicked_from;
                iss >> user_kicked;
                std::getline(iss, reason); // Read the rest of the line as message text
                reason = reason.substr(1);
                reason = reason.substr(0, reason.size());
                int perm = if_admin_inchannel(fd, channel_kicked_from);
                std::string user = get_user(fd);
                if(perm == 1)
                {
                    int check_ = 20;
                    if(user_kicked[0] != '@' || user == get_admin(channel_kicked_from))
                    {
                        check_ = kick_memeber(channel_kicked_from, user_kicked, reason, fd);
                        kick_memeber_invited(channel_kicked_from,user_kicked);
                    }
                    else
                    {
                        std::string user_not_found = ":" + admin + " PRIVMSG " + channel_kicked_from + " the user : " + user_kicked + " not member of this channel.\r\n";
                        send(fd, user_not_found.c_str(), user_not_found.size(), 0);
                    }
                    if(check_ == 1)
                    {
                        std::string user_not_found = ":" + admin + " PRIVMSG " + channel_kicked_from + " the user : " + user_kicked + " not member of this channel.\r\n";
                        send(fd, user_not_found.c_str(), user_not_found.size(), 0);
                    }
                    else if(check_ == 2)
                    {
                        std::string ser = "Server";
                        std::string not_admin = ":" + ser + " PRIVMSG " + channel_kicked_from + " You are not authorized to execute this command kick " + user_kicked + "\r\n";
                        send(fd, not_admin.c_str(), not_admin.size(), 0);
                    }
                }
                else
                {
                    std::string ser = "Server";
                    std::string not_admin = ":" + ser + " PRIVMSG " + channel_kicked_from + " You are not authorized to execute this command kick " + user_kicked + "\r\n";
                    send(fd, not_admin.c_str(), not_admin.size(), 0);
                }
            }
            else if(command.substr(0, 7) == "invite " || command.substr(0, 7) == "INVITE ")
            {
                std::string invite;

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos)
                {
                    invite = command.substr(space_pos + 1);
                    invite = invite.substr(0, invite.size() - 1);

                    std::istringstream iss(command);
                    std::string command, channel_invited_from, user_invited;

                    iss >> command;
                    iss >> user_invited;
                    iss >> channel_invited_from;
                    std::cout << "user_invited :" << user_invited << "."  << std::endl;
                    std::cout << "channel invited  from :" << channel_invited_from << "."  << std::endl;

                    std::map<std::string, std::vector<int> >::iterator it;
                    it = nick_fd.find(user_invited);
                    
                    if (it != nick_fd.end())
                    {
                        std::string invite_msg = ":" + get_admin(channel_invited_from) + " INVITE " + user_invited + " :" + channel_invited_from + "\r\n";
                        int invited_fd = get_fd_users(user_invited);
                        invited_to_channel(channel_invited_from, user_invited);
                        send(fd, invite_msg.c_str(), invite_msg.size(), 0);
                        send(invited_fd, invite_msg.c_str(), invite_msg.size(), 0);
                    }
                }

            }
            else if (command.substr(0, 5) == "mode " || command.substr(0, 5) == "MODE ")
            {
                std::istringstream iss(command);
                std::string cmd,name_channel, mode, key;

                iss >> cmd;
                iss >> name_channel;
                iss >> mode;
                iss >> key;           

                int check = if_admin_inchannel(fd, name_channel);
                if (check == 1)
                {
                    if (mode == "+i" || mode == "-i")
                    {
                        mode_i(mode, name_channel);
                        std::string msg_i = ":server.host MODE " + name_channel + " " + mode + " by " + get_admin(name_channel) + "\n";
                        send(fd, msg_i.c_str(), msg_i.size(), 0);
                    }
                    else if (mode == "+t" || mode == "-t")
                    {
                        mode_topic(mode, name_channel, fd);
                    }
                    else if (mode == "+k" || mode == "-k")
                    {
                        mode_k(mode, name_channel, key);
                        std::string msg_k = ":server.host MODE " + name_channel + " " + mode + " by " + get_admin(name_channel) + "\n";
                        send(fd, msg_k.c_str(), msg_k.size(), 0);
                    }
                    else if (mode == "+o" || mode == "-o")
                    {
                        mode_o(mode, name_channel, key, fd);
                    }
                    else if (mode == "+l" || mode == "-l")
                    {
                        mode_l(mode, name_channel, key, fd);
                    }
                }
            }
            else if (command.substr(0, 6) == "topic " || command.substr(0, 6) == "TOPIC ")
            {
                std::istringstream iss(command);
                std::string cmd, channel_nn, topic;


                iss >> cmd;
                iss >> channel_nn;
                iss >> topic;

                channel_topic[channel_nn] = topic;
                
                std::vector<std::string>::iterator it = std::find(topic_mode.begin(), topic_mode.end(), channel_nn);
                int check = if_admin_inchannel(fd, channel_nn);

                if (it != topic_mode.end() || check == 1)
                {
                    std::cout << "check :" << check << std::endl;
                    std::string topic_t = ":" + get_user(fd) + " TOPIC " + channel_nn + " :" + topic + "\r\n";
                    send(fd, topic_t.c_str(), topic_t.size(), 0);
                }
                else
                {
                    std::string msg_e = ":" + get_user(fd) + " PRIVMSG " + channel_nn + " :Error: You can't execute this command " + "\r\n";
                    send(fd, msg_e.c_str(), msg_e.size(), 0);
                }
            }
            else if (command.substr(0, 5) == "ping " || command.substr(0, 5) == "PING ")
            {
                ping(command, fd);
            }
        }

    if (nbytes == 0) {
        std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        clientCleanup(fd);
    } else if (nbytes == -1) {
        std::cerr << "Error reading data from client <" << fd << ">" << std::endl;
        clientCleanup(fd);
    }
}

void Server::clientCleanup(int fd) {
    for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (it->fd == fd) {
            _fds.erase(it);
            close(fd);
            break;
        }
    }

    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->getFd() == fd) {
            _clients.erase(it);
            break;
        }
    }
}

// std::string Server::formatCreationTime() {
//     std::time_put currentTime = std::time(NULL);
//     std::tm* localtime = std::locale(&currentTime);

//     char buffer[80]; 
//     std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localtime);
//     return std::string(buffer);
// }

// std::string Server::constructCreationTimeMessage(const std::string& channelName) {
//     std::stringstream ss;
//     ss << "Channel #" << channelName << " created " << formatCreationTime();
//     return ss.str();
// }
void Server::welcome(const std::string& nickname, int fd)
{
    std::string msg_one = ":irc.ChatWladMina 001 " + nickname + " :Welcome to the ChatWladMina Network, " + nickname + '\n';
    std::string msg_two = ":irc.ChatWladMina 002 " + nickname + " :Your host is ChatWladMina, running version 4.5" + '\n';
    std::string msg_tre = ":irc.ChatWladMina 003 " + nickname + " :This server was created " + '\n';
    std::string msg_foor = ":irc.ChatWladMina 004 " + nickname + " ChatWladMina ChatWladMina(enterprise)-2.3(12)-netty(5.4c)-proxy(0.9) ChatWladMina" + '\n';
    send(fd, msg_one.c_str(), msg_one.length(), 0);
    send(fd, msg_two.c_str(), msg_two.length(), 0);
    send(fd, msg_tre.c_str(), msg_tre.length(), 0);
    send(fd, msg_foor.c_str(), msg_foor.length(), 0);
}

void Server::closeFds() {
    for (size_t i = 0; i < _clients.size(); i++){
        int fd = _clients[i].getFd();
        std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        close(fd);
    }

    if (_serverSocketFd != -1)
        close(_serverSocketFd);

    _fds.clear();
}
void Server::ping(const std::string& command, int fd) {
    std::istringstream iss(command);
    std::string serverHostname = command.substr(5);
    std::string pongMessage = "PONG " + serverHostname + "\r\n";
    send(fd, pongMessage.c_str(), pongMessage.length(), 0);
    std::cout << "PONG" << std::endl;
}

void Server::processMessage(const std::string& message, int fd)
{
    std::istringstream iss(message);
    std::string command, channel_name_t, text;

    iss >> command;

    iss >> channel_name_t;
    std::getline(iss, text);
    text = text.substr(1);
    int fd_user = get_fd_users(channel_name_t);
    if (fd_user != -1)
    {
        text = text.substr(0, text.size() - 1);
        std::string prv_msg = ":" + get_user(fd) + " PRIVMSG " + channel_name_t + " :" + text + "\r\n";
        send(fd_user, prv_msg.c_str(), prv_msg.size(), 0);
    }

    std::map<std::string, std::vector<int> >::iterator it;

    std::vector<int>::iterator it1;
    std::string user_c;
    for (it = nick_fd.begin(); it != nick_fd.end(); ++it)
    {
        std::vector<int>::iterator vec_it;
        for(vec_it = it->second.begin(); vec_it != it->second.end(); ++vec_it)
        {
            if (*vec_it == fd)
            {
                user_c = it->first;
                fd_u = fd;
            }
        }
    }
    // text = text.substr(0, text.size());
    std::string tmp_mess = ":" + user_c + " PRIVMSG " + channel_name_t + " " + text + "\r\n";
    if(if_member(user_c, channel_name_t) == true)
        sendMessageToChannel(channel_name_t, tmp_mess);
}

void Server::sendMessageToChannel(const std::string& channell, const std::string& message) 
{

    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channell);
    std::cout << "here" << std::endl;
    if(it == channel.end())
    {
        // for(it = channel.begin(); it != channel.end();it++)
        // {
        //     std::cout << "first : "<<it->first << "."  << std::endl;
        // }
        std::cout << "Channel '" << channell << "' does not exist." << std::endl;
    }
    else
    {
        std::vector<int> fds = send_msg(channell);
        std::vector<int>::iterator in;

        for (size_t i = 0; i < fds.size(); ++i) 
        {
            int user = fds[i];
            if (fd_u != fds[i])
                sendMessageToUser(user, message);
        }

    } 
}

void Server::sendMessageToUser(int user, const std::string& message) 
{
    // std::cout << "message22222--->" << message << std::endl;
    send(user, message.c_str(), message.size(), 0);
    std::cout << "Sending message to user '" << user << "': " << message ;
}

