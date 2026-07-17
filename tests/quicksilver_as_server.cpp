
#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 16 > server;
    return server.start();
}
