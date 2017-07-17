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

void help()
{
    std::cerr <<
            "             cljcrparser - Codalogic JCR Parser\n"
            "\n"
            "Usage:\n"
            "    cljcrparser [ flags ] <jcr-file-list>\n"
            "\n"
            "Flags:\n"
            "    -h:\n"
            "    -?:\n"
            "        Print this help information\n"
            "\n"
            "    -json <file>:\n"
            "        Specify JSON file to be validated against specified JCR files\n"
            "\n"
            "<jcr-file-list> - One or more JCR files to verify.\n"
            ;
}

bool capture_command_line( cljcr::Config * p_config, int argc, char ** argv )
{
    for( clutils::CommandLineArgs cla( argc, argv, &help ); cla; ++cla )
    {
        if( cla.is_flag( "?", "h" ) )
        {
            help();
            return false;
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

int main( int argc, char * argv[] )
{
    cljcr::Config config;
    if( ! capture_command_line( &config, argc, argv ) )
        return -1;
    // return process_config( config );
}
