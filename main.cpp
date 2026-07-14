#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 8 > topology{};
    topology.start();
}
