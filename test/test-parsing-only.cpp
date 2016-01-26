//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
// All rights reserved.
//
// The license for this file is based on the BSD-3-Clause license
// (http://www.opensource.org/licenses/BSD-3-Clause).
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// - Neither the name Codalogic Ltd nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------

#include "clunit.h"

#include "cl-jcr-parser/cl-jcr-parser.h"

using namespace cljcr;

#include "test-parser-harness.h"

void test_parsing_only( const char * p_jcr )
{
    TDOC( p_jcr );
    GrammarSet grammar_set;
    JCRParser jcr_parser( &grammar_set );
    JCRParser::Status status = jcr_parser.add_grammar( p_jcr, strlen( p_jcr ) );

    TCRITICALTEST( status == JCRParser::S_OK );
}

void test_parsing_only_bad_input( const char * p_jcr )
{
    TDOC( p_jcr );
    GrammarSet grammar_set;
    JCRParser jcr_parser( &grammar_set );
    JCRParser::Status status = jcr_parser.add_grammar( p_jcr, strlen( p_jcr ) );

    TCRITICALTEST( status != JCRParser::S_OK );
}

TFEATURE( "GrammarParser - Syntax parsing with no semantic interpretation - comments" )
{
    TCALL( test_parsing_only(
                        "" ) );
    TCALL( test_parsing_only(
                        "   " ) );
    TCALL( test_parsing_only(
                        "; Hello World" ) );
    TCALL( test_parsing_only(
                        "; Hello World" ) );
    TCALL( test_parsing_only(
                        "; Hello World\n"
                        "     " ) );
    TCALL( test_parsing_only(
                        "; Hello World\n"
                        "     \n"
                        "  ; Let's go..." ) );
    TCALL( test_parsing_only_bad_input(
                        "; Hello World ; Let's go...\n" ) );
    TCALL( test_parsing_only(
                        "; Hello World ;; Let's go...\n" ) );
    TCALL( test_parsing_only(
                        "" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - JCR directive" )
{
    TCALL( test_parsing_only(
                        "#jcr-version 0.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#\n"
                        "jcr-version\n"
                        " 0.5" ) );
    TCALL( test_parsing_only(
                        "#jcr-version 0.12" ) );
    TCALL( test_parsing_only(
                        "#jcr-version 10.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version 0.5 too long" ) );
    TCALL( test_parsing_only(
                        "; Start\n"
                        "#jcr-version 0.5" ) );
    TCALL( test_parsing_only(
                        "; Start\n"
                        "#jcr-version 0.5\n"
                        "; End" ) );
    TCALL( test_parsing_only(
                        "  #jcr-version 0.5\n"
                        "; End" ) );
    TCALL( test_parsing_only(
                        "; Start ; #jcr-version 0.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version0.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version a.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version 0:5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version 0.b" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - ruleset-id directive" )
{
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#ruleset-id \n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#ruleset-id\n" ) );
    TCALL( test_parsing_only(
                        "; A comment ; #ruleset-id http://www.example.com/jcr\n" ) );
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr\n"
                        "; A comment" ) );
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr\n"
                        "; A comment\n"
                        "; Another comment\n"
                        "#jcr-version 0.5\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - import directive" )
{
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#import http://www.example.com/jcr as\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#import http://www.example.com/jcr as \n" ) );
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr as my_1st-alias\n" ) );
    TCALL( test_parsing_only(
                        " ; Hello\n"
                        "#import http://www.example.com/jcr as  my_1st-alias\n" ) );
    TCALL( test_parsing_only(
                        " ; Hello\n"
                        "#jcr-version 0.5\n"
                        "#ruleset-id http://www.example.com/jcr\n"
                        "#import http://www.example.com/jcr as  my_1st-alias\n"
                        "#import http://www.example.com/jcr2 as  my_2nd-alias\n"
                        "\n" ) );
    TCALL( test_parsing_only(
                        " ; Hello\n"
                        "#jcr-version 0.5\n"
                        "  ; A comment\n"
                        "#ruleset-id http://www.example.com/jcr\n"
                        "  ; A comment\n"
                        "  ; A comment\n"
                        "#import http://www.example.com/jcr as  my_1st-alias\n"
                        "  ; A comment\n"
                        "  ; A comment\n"
                        "  ; A comment\n"
                        "  ; A comment\n"
                        "#import http://www.example.com/jcr2 as  my_2nd-alias\n"
                        "\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - TBD directive" )
{
    TCALL( test_parsing_only_bad_input(
                        "#TBD\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#TBD  we\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#TBD  we don't know\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "; Hello\n"
                        "#TBD\n"
                        "#jcr-version 0.5\n" ) );
}

TFEATURETODO( "Correctly populate and place annotations into Grammar" )
