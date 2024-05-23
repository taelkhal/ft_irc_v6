#include "Server.hpp"

// void Server::create_channel(const std::string &channel_name,const std::string& user_name, int fd)
// {
//     std::map<std::string, std::vector<std::string> >::iterator it;
//     if(channel.find(channel_name) == channel.end())
//     { 
//         channel[channel_name].push_back(user_name);
//     }
//     else
//     {
//         channel[channel_name].push_back(user_name);
//         std::cout << "channel already exist" << std::endl;
//         // return false;
//     }
// }



int Server::if_member_invited(int fd, std::string &channel_joined)
{
    //workin on it
    std::cout << "channel joined :" << channel_joined << std::endl;
    std::map<std::string, std::vector<int> >::iterator it = nick_fd.begin();
    std::vector<int>::iterator it1;
    for(;it != nick_fd.end(); it++)
    {
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            if(*it1 == fd)
            {
                std::string memeber_i = it->first;
                std::cout << "it->first :" << it->first << std::endl;
                std::cout << "memeber_i :" << memeber_i << std::endl;
                bool p = if_invited(memeber_i, channel_joined);
                if(p == true)
                {
                    std::cout << "hhhhhhhhhh" << std::endl;
                    return 1;
                }
                break;  
            }
        }
    }
    return 0;
}
std::string Server::get_user(int fd)
{
    std::map<std::string, std::vector<int> >::iterator it;
    std::vector<int>::iterator it1;
    std::string user_c;

    for (it = nick_fd.begin(); it != nick_fd.end(); ++it)
    {
        std::vector<int>::iterator vec_it;
        for(vec_it = it->second.begin(); vec_it != it->second.end(); ++vec_it)
        {
            if (*vec_it == fd)
                user_c = it->first;
        }
    }
    return (user_c);
}
int Server::if_admin_inchannel(int fd, std::string &channel_perm)
{
    std::map<std::string, std::vector<int> >::iterator it = nick_fd.begin();
    std::vector<int>::iterator it1;
    for(;it != nick_fd.end(); it++)
    {
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            if(*it1 == fd)
            {
                std::string admmmin = '@' + it->first;
                bool p = if_member(admmmin, channel_perm);
                if(p == true)
                    return 1;
                break;  
            }
        }
    }
    return 0;
}
void Server::kick_memeber_invited(std::string &channel_kicked_from_i, std::string &user_kicked_i)
{
    std::map<std::string, std::vector<std::string> >::iterator it = invited.find(channel_kicked_from_i);
    std::vector<std::string>::iterator it1;
    it = invited.find(channel_kicked_from_i);
    if(it != invited.end())
    {
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            if(*it1 == user_kicked_i && user_kicked_i[0] != '@')
            {
                it->second.erase(it1);
                break;
            }
        }
    }
}

void Server::mode_k(std::string mode, std::string channel_name, std::string key)
{
    if (mode == "+k")
        channel_password[channel_name] = key;
    else if (mode == "-k")
        channel_password.erase(channel_name);
}

int Server::kick_memeber(std::string &channel_kicked_from, std::string &user_kicked, std::string reason, int fd)
{
    std::map<std::string, std::vector<std::string> >::iterator it = channel.find(channel_kicked_from);
    std::vector<std::string>::iterator it1;


    for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    {
        std::cout << "*it1 :" << *it1 << std::endl;
        std::cout << "user_kicked :" << user_kicked << std::endl;
        
        std::string tmp = '@' + user_kicked;
        if((*it1 == user_kicked && if_member(user_kicked, channel_kicked_from)) || *it1 != get_admin(channel_kicked_from))
        {
            std::cout << "kick function x" << std::endl;
            it->second.erase(it1);
            std::string a =  "admin";
            std::cout << "reason :" << reason << std::endl;
            std::string kick_msg = ":" + a + " KICK " + channel_kicked_from + " " + user_kicked + " :" + reason + "\n";
            kick_memeber_invited(channel_kicked_from, user_kicked);
            send(fd, kick_msg.c_str(), kick_msg.size(), 0);
            int fd_kicked = get_fd_users(user_kicked);
            send(fd_kicked, kick_msg.c_str(), kick_msg.size(), 0);
            return 0;
            break;
        }
        else if(*it1 == tmp)
            return 2;
    }
    return 1;
}
void Server::create_channel(const std::string &channel_name, const std::string& user_name, int fd)
{
    (void)fd;
    std::map<std::string, std::vector<std::string> >::iterator it = channel.find(channel_name);
    if (it == channel.end())
    { 
        std::vector<std::string> users;
        users.push_back("@" + user_name); // Admin user
        channel[channel_name] = users;
    }
    else
    {
        std::string join_msg = ":" + user_name + " JOIN " + channel_name + "\r\n";
        sendMessageToChannel(channel_name, join_msg);
        channel[channel_name].push_back(user_name);
        
        std::cout << "channel already exists" << std::endl;
    }
}
            

void Server::set_fd_users(const std::string &user_name,int fd)
{
    if(nick_fd.find(user_name) == nick_fd.end())
    {
        nick_fd[user_name].push_back(fd);
    }
    else
    {
        std::cout << "user already exist" << std::endl;
    }
}

bool Server::if_invited(std::string user_inchannel, std::string channels_name)
{
    std::vector<std::string>::iterator it1;
    std::map<std::string, std::vector<std::string> >::iterator it;
    std::cout << "hna hna" << std::endl;
    it = invited.find(channels_name);

    if(it != invited.end())
    {
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            std::cout << "it1 : " << *it1 << std::endl;
            if(*it1 == user_inchannel)
            {
                return true;
            }
        }
    }
    return(false);
}


bool Server::if_member(std::string user_inchannel, std::string channels_name)
{
    std::vector<std::string>::iterator it1;
    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channels_name);
    if(it != channel.end())
    {
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            // std::cout << "fd: " << fd << std::endl;
            // std::cout << "it1 : " << *it1 << std::endl;
            std::string ad_check = *it1;
            if(&ad_check[1] == user_inchannel)
                return true;    
            if(*it1 == user_inchannel)
                return true;
        }
    }
    return(false);
}
int Server::get_fd_users(std::string &users_in_channel)
{
    // std::cout << users_in_channel << std::endl;

    // exit (0);
    int fd;
    std::vector<int>::iterator it1;
    std::map<std::string, std::vector<int> >::iterator it;
    if (users_in_channel[0] == '@')
    {
        it = nick_fd.find(&users_in_channel[1]);
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
            fd = *it1;
    }
    else
    {
        it = nick_fd.find(users_in_channel);
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            fd = *it1;
        }
    }
    // it = nick_fd.find(&users_in_channel[1]);
    // std::cout << "------>>>>>>" << users_in_channel << std::endl;
    // for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    // {
    //     fd = *it1;
    // }
    return fd;
}

void Server::invited_to_channel(const std::string &channel_invited_to, const std:: string& user_invited)
{
    std::map<std::string, std::vector<std::string> >::iterator it = invited.find(channel_invited_to);
    std::vector<std::string>::iterator it1;
    
    if (it == invited.end())
    {
        invited[channel_invited_to].push_back(user_invited);
    }
    else
    {
        if(it->second.size() == 0)
            invited[channel_invited_to].push_back(user_invited);
        else
        {
            for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
            {
                if(*it1 == user_invited)
                {
                    std::cout << "user already invited" << std::endl;
                    break;
                }
            }
            invited[channel_invited_to].push_back(user_invited);
        }   
    }
}

void Server::mode_i(std::string mode, std::string name_channel_i)
{

    std::map<std::string, std::vector<std::string> >::iterator it;
    if (mode == "+i")
        invited[name_channel_i];
    else if (mode == "-i")
        invited.erase(name_channel_i);
}

void Server::mode_l(std::string mode, std::string channel_name_l, std::string key,int fd)
{
    // lmochkill kayn hna khfif drif
    size_t i = key.find_first_not_of("0123456789");


    int limit = std::atoi(key.c_str());
    if (i != std::string::npos)
    {
        std::string msg_error = ":" + get_user(fd) + " PRIVMSG " + channel_name_l + " :Error: invalid Arguments " + "\r\n";
        send(fd, msg_error.c_str(), msg_error.size(), 0);
    }
    else if (mode == "+l")
    {
        channel_limit[channel_name_l] = limit;
        std::cout << "limit : "<< channel_limit[channel_name_l] << std::endl;
        std::string msg_l = ":server.host MODE " + channel_name_l + " " + mode + " by " + get_admin(channel_name_l) + "\n";
        send(fd, msg_l.c_str(), msg_l.size(), 0);
    }
    else if (mode == "-l")
    {
        channel_limit.erase(channel_name_l);
        std::string msg_l = ":server.host MODE " + channel_name_l + " " + mode + " by " + get_admin(channel_name_l) + "\n";
        send(fd, msg_l.c_str(), msg_l.size(), 0);
    }

}

void Server::mode_o(std::string mode, std::string channel_name_o, std::string key_o, int fd)
{
    std::map<std::string, std::vector<std::string> >::iterator it;

    int check = if_admin_inchannel(fd, channel_name_o);
    it = channel.find(channel_name_o);
    std::vector<std::string>::iterator it1;
    if (mode == "+o")
    {
        if (check == 1)
        {
            if (if_member(key_o, channel_name_o) == true)
            {
                std::vector<std::string>& members = it->second;
                for (std::vector<std::string>::iterator it1 = members.begin(); it1 != members.end(); ++it1)
                {
                    if (*it1 == key_o)
                    {
                        it1 = members.erase(it1);
                        members.insert(it1, "@" + key_o);
                        std::string msg_o = ":server.host MODE " + channel_name_o + " " + mode + " by " + get_admin(channel_name_o) + " and set " + key_o + " as operator\n";
                        send(fd, msg_o.c_str(), msg_o.size(), 0);
                        break;
                    }
                }
            }
            else
            {
                if(key_o.size())
                {
                    std::string serv = "Server";
                    std::string user_not_found = ":" + serv + " PRIVMSG " + channel_name_o + " the user : " + key_o + " not member of this channel.\r\n";
                    send(fd, user_not_found.c_str(), user_not_found.size(), 0); 
                }
                else
                {
                    std::string msg_error = ":" + get_user(fd) + " PRIVMSG " + channel_name_o + " :Error: invalid Arguments " + "\r\n";
                    send(fd, msg_error.c_str(), msg_error.size(), 0);
                }
            }
        }

    }
    else if (mode == "-o")
    {
        std::string admin_h = get_admin(channel_name_o);

        admin_h = &admin_h[1];
        if (check == 1 && admin_h != key_o)
        {
            if (if_member("@" + key_o, channel_name_o) == true)
            {
                std::vector<std::string>& members = it->second;
                for (std::vector<std::string>::iterator it1 = members.begin(); it1 != members.end(); ++it1)
                {
                    if (*it1 == "@" + key_o)
                    {
                        it1 = members.erase(it1);
                        members.insert(it1, key_o);
                        std::string msg_o = ":server.host MODE " + channel_name_o + " " + mode + " by " + get_admin(channel_name_o) + " and unset " + key_o + " as operator\n";
                        send(fd, msg_o.c_str(), msg_o.size(), 0);
                        break;
                    }
                }
            }
            else
            {
                if(key_o.size())
                {
                    std::string serv = "Server";
                    std::string user_not_found = ":" + serv + " PRIVMSG " + channel_name_o + " the user : " + key_o + " not member of this channel.\r\n";
                    send(fd, user_not_found.c_str(), user_not_found.size(), 0); 
                }
                else
                {
                    std::string msg_error = ":" + get_user(fd) + " PRIVMSG " + channel_name_o + " :Error: invalid Arguments " + "\r\n";
                    send(fd, msg_error.c_str(), msg_error.size(), 0);
                }
            }
        }
    }
}

void Server::mode_topic(std::string mode, std::string channel_name_t, int fd)
{
    if (mode == "+t")
    {
        std::vector<std::string>::iterator it = std::find(topic_mode.begin(), topic_mode.end(), channel_name_t);
        if (it != topic_mode.end())
        {
            topic_mode.erase(it);
            std::string msg_t = ":server.host MODE " + channel_name_t + " +t by " + get_admin(channel_name_t) + "\n";
            send(fd, msg_t.c_str(), msg_t.size(), 0);
        }
        else
        {
            std::string msg_error = ":server.host MODE " + channel_name_t + " +t by " + get_admin(channel_name_t) + "\n";
            send(fd, msg_error.c_str(), msg_error.size(), 0);
        }
        // fix the segfault 
        // add messages
    }
    else if (mode == "-t")
    {
        topic_mode.push_back(channel_name_t);
        std::string msg_t = ":server.host MODE " + channel_name_t + " -t by " + get_admin(channel_name_t) + "\n";
        send(fd, msg_t.c_str(), msg_t.size(), 0);
    }
}

std::vector<int> Server::send_msg(const std::string &channel_msgs)
{
    std::vector<std::string>::iterator it1;
    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channel_msgs);
    std::vector<int> fds;
    for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    {
        int fd = get_fd_users(*it1);
        fds.push_back(fd);
    }
    return (fds);
}
