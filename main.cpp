#include <iostream>
#include <memory>
#include "jsonParser/lexer.h"
#include "jsonParser/token.h"
#include "jsonParser/parser.h"
#include "utils/job.h"
#include <unistd.h>
#include <fcntl.h>

int main()
{
    Utils::Job job( ::open( "test.json", O_RDONLY ), 1, false );
    Json::Parser parser( job );

    parser.parse();

    std::cout << (*job.json).serialize() << std::endl;


    return 0;
}
