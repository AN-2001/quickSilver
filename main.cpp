#include <iostream>
#include <memory>
#include "jsonParser/lexer.h"
#include "jsonParser/token.h"
#include "utils/job.h"

int main()
{
    Utils::Job job( 0, 1, false );
    Json::Lexer lexer( job );

    while ( lexer.peekToken().type != Json::Token::Type::Eof ) {
        std::cout << (std::string) lexer.getToken() << std::endl;
    }

    return 0;
}
