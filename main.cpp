#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 8 > topology{};
    return topology.start();
}
