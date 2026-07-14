#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 4 > topology{};
    topology.start();
}
