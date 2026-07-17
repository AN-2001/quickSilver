#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 64 > topology{};
    return topology.start();
}
