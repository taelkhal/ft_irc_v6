#ifndef PARSE_HPP
#define PARSE_HPP


#include "../multuplixing/multuplixing.hpp"
#include "../conf/configFile.hpp"
#include "../Server.hpp"
class client;
void parseBuff(int fd, std::string buff, server *ser, client *cl, Server *s, std::vector<client> );

#endif