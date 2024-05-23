#include "../conf/configFile.hpp"
#include "../multuplixing/multuplixing.hpp"
#include "parse.hpp"
#include "../Server.hpp"
int matchWord (std::string t, std::string tmp)
{
    size_t i = 0;
    for (; t[i] != 32 && t[i] != '\t' && t[i]; i++){
        if (tmp[i] != t[i])
            return 0;
    }
    if (i != tmp.length())
        return 0;
    return 1;
}

int validNick(std::string buff, int fd, server *ser, client *cl, Server *s, std::vector<client> mycl)
{
    (void )ser;
    std::string tmp = buff.substr(5, buff.length() - 7);
    if (tmp.empty() || buff[0] == '$' || buff[0] == ':'){
        std::string t = "irssi " + tmp + " :Erroneus nickname\r\n";
        send_message(fd, t.c_str());
        send_message(fd, "pleas enter your nickname in this format(/quote nick [YOUR_NICKNAME]):\r\n");
        return 0;
    }
    for (int i = 0; tmp[i]; i++)
    {
        if (tmp[i] == 32 || tmp[i] == '.' || tmp[i] == '\t' || tmp[i] == '?' || tmp[i] == '!' || tmp[i] == ',' || tmp[i] == '@' || tmp[i] == '*'){
            std::string t = "irssi " + tmp + " :Erroneus nickname\r\n";
            send_message(fd, "pleas enter your nickname in this format(/quote nick [YOUR_NICKNAME]):\r\n");
            return 0;
        }
    }
    for (std::vector<client>::iterator it = mycl.begin(); it != mycl.end(); it++)
    {
        if (tmp == it->nick)
        {
            std::string t = "irssi " + tmp + " :Erroneus nickname\r\n";
            send_message(fd, t.c_str());
            send_message(fd, "pleas enter your nickname in this format(/quote nick [YOUR_NICKNAME]):\r\n");
            return 0;
        }
    }
    cl->nick = tmp;
    cl->setIndice(2);
    s->set_fd_users(cl->nick, fd);
    send_message(fd, "Pleas enter your name in this format (/quote user [USERNAME] 0 * [REALNAME])\r\n");
    
    return 1;

}
int validPass(std::string buff, std::string pass, server *ser, int fd, client *cl)
{
    (void )ser;
    (void )cl;
    if (buff.substr(5, buff.length() - 7) == pass){
        cl->setIndice(1);
        send_message(fd, "password match\r\n");
        send_message(fd, "pleas enter your nickname in this format(/quote nick [YOUR_NICKNAME]):\r\n");
        return 1;
    }
    else{
        send_message(fd, "irssi: Password incorrect\r\n");
        send_message(fd, "pleas enter ur pass:(in this format /quote pass [PASS]\r\n");
    }
    return 0;
}

int validUser(int fd, server *ser, client *cl, std::string buff, std::vector<client> mycl)
{
    (void )fd;
    (void )ser;
    (void )cl;
    (void )buff;
    std::vector<std::string> tmp = splitString(buff, ' ');
    if (tmp.size() != 5){
        send_message(fd, "irssi user : Not enought parametres");
        send_message(fd, "Pleas enter your name in this format (/quote user [USERNAME] 0 * [REALNAME])\r\n");
        return 0;
    }
    // printf("%lu|%lu|%c|%c|%s|%d|\n", tmp[2].length() , tmp[3].length(), tmp[2][0], tmp[3][0], tmp[1].c_str(), tmp[1].empty());
    if (tmp[1].empty() || tmp[2].length() != 1 || tmp[3].length() != 1 || tmp[2][0] != '0' || tmp[3][0] != '*'){
        send_message(fd, "irssi user : Errors in  parametres\n");
        send_message(fd, "Pleas enter your name in this format (/quote user [USERNAME] 0 * [REALNAME])\r\n");
        return 0;
    }
    for (std::vector<client>::iterator it = mycl.begin(); it != mycl.end(); it++)
    {
        if (tmp[1] == it->name)
        {
            std::string t = "irssi " + tmp[0] + " :Erroneus nickname\r\n";
            send_message(fd, t.c_str());
            send_message(fd, "pleas enter your nickname in this format(/quote nick [YOUR_NICKNAME]):\r\n");
            return 0;
        }
    }
    cl->setIndice(3);
    // for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
    //     std::cout << "look:" << *it << std::endl;
    send_welcome_message(fd, ser, cl->nick);
    send_host_info(fd, ser, cl->nick);
    send_creation_date(fd, ser, cl->nick);
    send_server_info(fd, ser, cl->nick);
    cl->name = tmp[1];
    return 1;
    // std::string tmp = buff.substr(5, buff.length() - 7);
    // if (tmp.empty()){
    //     return 0;
    // }


}

void parseBuff(int fd, std::string buff, server *ser, client *cl, Server *s, std::vector<client> mycl)
{
    // printf("\nReceived data from client %d:%s:%d\n", fd, buff.c_str(), cl->getIndice());
    if ((matchWord(buff, "pass") || matchWord(buff, "PASS")) && cl->getIndice() == 0){
        validPass(buff, ser->password, ser, fd, cl);
        }
    else if ((matchWord(buff, "user") || matchWord(buff, "USER")) && cl->getIndice() == 2){
        validUser(fd, ser, cl, buff, mycl);
        }
    else if ((matchWord(buff, "nick") || matchWord(buff, "NICK")) && cl->getIndice() == 1){
        validNick(buff, fd, ser, cl, s, mycl);
        }
    // else if (matchWord(buff, "JOIN") || matchWord(buff, "JOIN"))
    //     printf("\nword:JOIN\n");
    // else if (matchWord(buff, "kick") || matchWord(buff, "KICK"))
    //     printf("\nword:KICK\n");
    // else if (matchWord(buff, "topic") || matchWord(buff, "TOPIC"))
    //     printf("\nword:TOPIC\n");
    // else if (matchWord(buff, "invite") || matchWord(buff, "INVITE"))
    //     printf("\nword:INVITE\n");
    // else if (matchWord(buff, "privmsg") || matchWord(buff, "PRIVMSG"))
    //     printf("\nword:PRIVMSG\n");
    // else
    //     printf("i will send to client %d\n", fd);
}
