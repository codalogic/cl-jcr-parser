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
    TCALL( test_parsing_only(
                        "#jcr-version 0.5 " ) );
    TCALL( test_parsing_only(
                        "#jcr-version 0.5 \t" ) );
    TCALL( test_parsing_only_bad_input(
                        "#\n"
                        "jcr-version\n"
                        " 0.5" ) );
    TCALL( test_parsing_only_bad_input(
                        "#jcr-version 0.12" ) );
    TCALL( test_parsing_only_bad_input(
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
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr \n" ) );
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
    {
        TSETUP( ParserHarness ph( "#ruleset-id http://www.example.com/jcr" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().ruleset_id == "http://www.example.com/jcr" );
    }
}

TFEATURE( "GrammarParser - Syntax parsing - import directive" )
{
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr\n" ) );
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr \n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#import http://www.example.com/jcr as\n" ) );
    TCALL( test_parsing_only_bad_input(
                        "#import http://www.example.com/jcr as \n" ) );
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr as my_1st-alias\n" ) );
    TCALL( test_parsing_only(
                        "#import http://www.example.com/jcr as my_1st-alias \n" ) );
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
    {
        TSETUP( ParserHarness ph( "#import http://www.example.com/jcr" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().unaliased_imports[0] == "http://www.example.com/jcr" );
    }
    {
        TSETUP( ParserHarness ph( "#import http://www.example.com/jcr as jcr" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().has_aliased_import( "jcr" ) );
        TCRITICALTEST( ph.grammar().get_aliased_import( "jcr" ).value() == "http://www.example.com/jcr" );
    }
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

TFEATURE( "GrammarParser - Syntax parsing - target_rule_name" )
{
    {
        TSETUP( ParserHarness ph( "my_rule other_rule\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.local_name == "other_rule" );
    }
    {
        TSETUP( ParserHarness ph( "#import http://foo.bar as foo\n my_rule foo.other_rule\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.rulesetid == "http://foo.bar" );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.local_name == "other_rule" );
    }
}

TFEATURE( "GrammarParser - Syntax parsing - Primitive rules" )
{
    TCALL( test_parsing_only(
                        "my_rule : null\n" ) );
    {
        TSETUP( ParserHarness ph( "my_rule : flubber\n" ) );
        TCRITICALTEST( ph.status() != JCRParser::S_OK );
    }
    {
        TSETUP( ParserHarness ph( "my_rule : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "another_rule : boolean\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "another_rule : true\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min == "true" );
        TTEST( ph.grammar().rules[0].min.to_bool() == true );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "true" );
        TTEST( ph.grammar().rules[0].max.to_bool() == true );
    }
    {
        TSETUP( ParserHarness ph( "another_rule : false\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min == "false" );
        TTEST( ph.grammar().rules[0].min.to_bool() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "false" );
        TTEST( ph.grammar().rules[0].max.to_bool() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : string\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : \"a string\"\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_LITERAL );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min == "a string" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "a string" );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : /pref\\d+/i\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_REGEX );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min == "/pref\\d+/i" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "/pref\\d+/i" );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : float\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : 1.0..\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_float() == 1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : -1.0..3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_float() == -1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_float() == 3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : -1.0..-3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_float() == -1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : ..-3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : 4.0\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_float() == 4.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_float() == 4.0 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : -3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_float() == -3.5e-2 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : integer\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : 100..\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_int() == 100 );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : -100..-50\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.to_int() == -100 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_int() == -50 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : ..50\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_int() == 50 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : 64\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_int() == 64 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.to_int() == 64 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : ip4\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IP4 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : ip6\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IP6 );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : idn\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IDN );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : fqdn\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FQDN );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : uri..http://example.com/foo#place\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_RANGE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "http://example.com/foo#place" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "http://example.com/foo#place" );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : uri..http://example.com/foo#place" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_RANGE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "http://example.com/foo#place" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max == "http://example.com/foo#place" );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : uri\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_TYPE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : phone\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::PHONE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : email\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::EMAIL );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : full-date\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DATE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : full-time\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TIME );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : date-time\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DATETIME );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : base64\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BASE64 );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "rule-2 : any\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ANY );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
}

TFEATURETODO( "Correctly populate and place annotations into Grammar" )
