#include "jobTools/jobValidator.h"
#include "jsonParser/lexer.h"
#include "jsonParser/parser.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <print>

int main()
{
    Utils::Job job( Utils::BorrowedFd{0}, 1 );
    Json::Lexer lexer( job );
    Json::Parser parser( lexer );
    JobTools::Validator validator( parser );
    std::println( "Error was: {}", Json::errorToString( parser.parse() ) );

    if ( !validator.validate() ) {
        std::println( "JOB IS INVALID" );
    } else
        std::println( "JOB IS VALID" );
}
