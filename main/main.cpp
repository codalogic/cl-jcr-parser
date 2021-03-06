//----------------------------------------------------------------------------
// Copyright (c) 2015-2017, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#include "cl-jcr-parser/all.h"

#include "cl-utils/command-line-args.h"

#include <iostream>

struct TestConfig
{
    bool is_parse_only;

    TestConfig() : is_parse_only( false ) {}
};

void help()
{
    std::cerr <<
            "             jcrcheck - Codalogic JCR Checker\n"
            "\n"
            "Usage:\n"
            "    jcrcheck [ flags ] <jcr-file-list>\n"
            "\n"
            "Flags:\n"
            "    -h:\n"
            "    -?:\n"
            "        Print this help information\n"
            "\n"
            "    -parse-only:\n"
            "        Only do the parse phase\n"
            "    -json <file>:\n"
            "        Specify JSON file to be validated against specified JCR files\n"
            "\n"
            "<jcr-file-list> - One or more JCR files to verify.\n"
            ;
}

bool capture_command_line( TestConfig * p_test_config, cljcr::Config * p_config, int argc, char ** argv )
{
    clutils::CommandLineArgs cla( argc, argv );

    if( ! cla )
    {
        help();
        return false;
    }

    for( ; cla; ++cla )
    {
        if( cla.is_flag( "?", "h" ) )
        {
            help();
            return false;
        }

        else if( cla.is_flag( "parse-only" ) )
        {
            p_test_config->is_parse_only = true;
        }

        else if( cla.is_flag( "json", 1, "-json flag must include name of JSON file to validate" ) )
        {
            p_config->set_json( cla.next() );
            std::cerr << "-json: Validation of JSON files against JCR not supported yet!\n";
        }

        else if( cla.is_flag() )
            std::cerr << "Unknown flag: " << cla.flag() << "\n";

        else
            p_config->add_jcr( cla.current() );
    }

    if( ! p_config->has_jcr() )
    {
        std::cerr << "Error: No JCR files specified\n";
        help();
        return false;
    }

    return true;
}

bool parse_config_jcrs( cljcr::GrammarSet * p_grammar_set, const TestConfig & r_test_config, const cljcr::Config & r_config )
{
    cljcr::JCRParserWithReporter jcr_parser( p_grammar_set );
    bool is_errored = false;

    for( size_t i = 0; i < r_config.jcr_size(); ++i )
    {
        cljcr::JCRParser::Status result = jcr_parser.add_grammar( r_config.jcr( i ).c_str() );

        if( result != cljcr::JCRParser::S_OK )
        {
            is_errored = true;

            if( result == cljcr::JCRParser::S_UNABLE_TO_OPEN_FILE )
                std::cout << "Unable to open JCR file: " << r_config.jcr( i ) << "\n";

            else if( result == cljcr::JCRParser::S_INTERNAL_ERROR )
                std::cout << "An internal error occurred while processing JCR file: " << r_config.jcr( i ) << "\n";
        }
    }
    
    if( is_errored || r_test_config.is_parse_only )
        return ! is_errored;

    is_errored = (jcr_parser.link() != cljcr::JCRParser::S_OK);

    return ! is_errored;
}

bool parse_grammar_set( cljcr::GrammarSet * p_grammar_set, const TestConfig & r_test_config, const cljcr::Config & r_config )
{
    bool result = parse_config_jcrs( p_grammar_set, r_test_config, r_config );

    std::cout << p_grammar_set->error_count() << " error(s), " << p_grammar_set->warning_count() << " warning(s)\n";

    return result;
}

int main( int argc, char * argv[] )
{
    TestConfig test_config;
    cljcr::Config config;

    if( ! capture_command_line( &test_config, &config, argc, argv ) )
        return -1;

    cljcr::GrammarSet grammar_set;

    if( ! parse_grammar_set( &grammar_set, test_config, config ) )
        return -1;

    return 0;
}
