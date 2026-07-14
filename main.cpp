#include "connections/serverTopology.h"

int main()
{
    Connections::ServerTopology< 3 > topology{};
    topology.start();
}
