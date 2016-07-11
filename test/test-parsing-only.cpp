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

void test_parsing_bad_input( const char * p_jcr )
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
    TCALL( test_parsing_bad_input(
                        "; Hello World ; Let's go...\n" ) );
    TCALL( test_parsing_only(
                        "; Hello World ;; Let's go...\n" ) );
    TCALL( test_parsing_only(
                        "" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - JCR directive" )
{
    TCALL( test_parsing_only(
                        "#jcr-version 0.6" ) );
    TCALL( test_parsing_only(
                        "#jcr-version 0.6 " ) );
    TCALL( test_parsing_only(
                        "#jcr-version 0.6 \t" ) );
    TCALL( test_parsing_bad_input(
                        "#\n"
                        "jcr-version\n"
                        " 0.5" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version 0.12" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version 10.5" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version 0.6 too long" ) );
    TCALL( test_parsing_only(
                        "; Start\n"
                        "#jcr-version 0.6" ) );
    TCALL( test_parsing_only(
                        "; Start\n"
                        "#jcr-version 0.6\n"
                        "; End" ) );
    TCALL( test_parsing_only(
                        "  #jcr-version 0.6\n"
                        "; End" ) );
    TCALL( test_parsing_only(
                        "; Start ; #jcr-version 0.6" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version0.5" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version a.5" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version 0:5" ) );
    TCALL( test_parsing_bad_input(
                        "#jcr-version 0.b" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - ruleset-id directive" )
{
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr\n" ) );
    TCALL( test_parsing_only(
                        "#ruleset-id http://www.example.com/jcr \n" ) );
    TCALL( test_parsing_bad_input(
                        "#ruleset-id \n" ) );
    TCALL( test_parsing_bad_input(
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
                        "#jcr-version 0.6\n" ) );
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
    TCALL( test_parsing_bad_input(
                        "#import http://www.example.com/jcr as\n" ) );
    TCALL( test_parsing_bad_input(
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
                        "#jcr-version 0.6\n"
                        "#ruleset-id http://www.example.com/jcr\n"
                        "#import http://www.example.com/jcr as  my_1st-alias\n"
                        "#import http://www.example.com/jcr2 as  my_2nd-alias\n"
                        "\n" ) );
    TCALL( test_parsing_only(
                        " ; Hello\n"
                        "#jcr-version 0.6\n"
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

TFEATURE( "GrammarParser - Syntax parsing - multi-line directive" )
{
    TCALL( test_parsing_only(
                        "#{constraint foo \n"
                        "  $id == 'None' }\n" ) );
    {
        TSETUP( ParserHarness ph(
                        "#{constraint foo\n"
                        "    $name == /p\\d{1,5}/ && ; Must allow } and { in comments\n"
                        "    $when == \"} with {\"\n"
                        "}\n"
                        "$my_rule = $other_rule\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
    }
}

TFEATURE( "GrammarParser - Syntax parsing - TBD directive" )
{
    TCALL( test_parsing_only(
                        "#TBD\n" ) );
    TCALL( test_parsing_only(
                        "#TBD  we\n" ) );
    TCALL( test_parsing_only(
                        "#TBD  we don't know\n" ) );
    TCALL( test_parsing_only(
                        "; Hello\n"
                        "#TBD\n"
                        "#jcr-version 0.6\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - target_rule_name" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = $other_rule\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.local_name == "other_rule" );
    }
    {
        TSETUP( ParserHarness ph( "#import http://foo.bar as foo\n $my_rule = $foo.other_rule\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.rulesetid == "http://foo.bar" );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.local_name == "other_rule" );
    }
    TCALL( test_parsing_only(
                        "$my_rule = @{not} $other_rule\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - Primitive rules" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = : flubber\n" ) );
        TCRITICALTEST( ph.status() != JCRParser::S_OK );
    }
    //The following my_rule= : null test experiment with various placements of = and :
    {
        TSETUP( ParserHarness ph( "$my_rule = : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = :null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule =: null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule =:null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule= : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule= :null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=: null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=:null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$another_rule= : boolean\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$another_rule = : true\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_bool() == true );
        TTEST( ph.grammar().rules[0].min.as_bool() == true );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_bool() == true );
        TTEST( ph.grammar().rules[0].max.as_bool() == true );
    }
    {
        TSETUP( ParserHarness ph( "$another_rule = : false\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "another_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BOOLEAN );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_bool() == true );
        TTEST( ph.grammar().rules[0].min.as_bool() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_bool() == true );
        TTEST( ph.grammar().rules[0].max.as_bool() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : string\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : \"a string\"\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_LITERAL );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_string() == true );
        TTEST( ph.grammar().rules[0].min.as_string() == "a string" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_string() == true );
        TTEST( ph.grammar().rules[0].max.as_string() == "a string" );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : /pref\\d+/i\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::STRING_REGEX );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_string() == true );
        TTEST( ph.grammar().rules[0].min.as_string() == "/pref\\d+/i" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_string() == true );
        TTEST( ph.grammar().rules[0].max.as_string() == "/pref\\d+/i" );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : float\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FLOAT );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : 1.0..\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_float() == true );
        TTEST( ph.grammar().rules[0].min.as_float() == 1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : -1.0..3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_float() == true );
        TTEST( ph.grammar().rules[0].min.as_float() == -1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_float() == true );
        TTEST( ph.grammar().rules[0].max.as_float() == 3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : -1.0..-3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_float() == true );
        TTEST( ph.grammar().rules[0].min.as_float() == -1.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_float() == true );
        TTEST( ph.grammar().rules[0].max.as_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : ..-3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_float() == true );
        TTEST( ph.grammar().rules[0].max.as_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : 4.0\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_float() == true );
        TTEST( ph.grammar().rules[0].min.as_float() == 4.0 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_float() == true );
        TTEST( ph.grammar().rules[0].max.as_float() == 4.0 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : -3.5e-2\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DOUBLE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_float() == true );
        TTEST( ph.grammar().rules[0].min.as_float() == -3.5e-2 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_float() == true );
        TTEST( ph.grammar().rules[0].max.as_float() == -3.5e-2 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : integer\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : 100..\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::UINTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_uint() == true );
        TTEST( ph.grammar().rules[0].min.as_uint() == 100 );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : -100..-50\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_int() == true );
        TTEST( ph.grammar().rules[0].min.as_int() == -100 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_int() == true );
        TTEST( ph.grammar().rules[0].max.as_int() == -50 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : ..50\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_int() == true );
        TTEST( ph.grammar().rules[0].max.as_int() == 50 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : 64\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::UINTEGER );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_uint() == true );
        TTEST( ph.grammar().rules[0].max.as_uint() == 64 );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_uint() == true );
        TTEST( ph.grammar().rules[0].max.as_uint() == 64 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : ipv4\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IPV4 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : ipv6\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IPV6 );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : ipaddr\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IPADDR );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : idn\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::IDN );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : fqdn\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::FQDN );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : uri..http://example.com/foo#place\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_RANGE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_string() == true );
        TTEST( ph.grammar().rules[0].min.as_string() == "http://example.com/foo#place" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_string() == true );
        TTEST( ph.grammar().rules[0].max.as_string() == "http://example.com/foo#place" );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : uri..http://example.com/foo#place" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_RANGE );

        TTEST( ph.grammar().rules[0].min.is_set() == true );
        TTEST( ph.grammar().rules[0].min.is_string() == true );
        TTEST( ph.grammar().rules[0].min.as_string() == "http://example.com/foo#place" );
        TTEST( ph.grammar().rules[0].max.is_set() == true );
        TTEST( ph.grammar().rules[0].max.is_string() == true );
        TTEST( ph.grammar().rules[0].max.as_string() == "http://example.com/foo#place" );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : uri\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::URI_TYPE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : phone\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::PHONE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : email\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::EMAIL );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : date\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DATE );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : time\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TIME );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : datetime\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::DATETIME );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : hex\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::HEX );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : base32\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BASE32 );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : base32hex\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BASE32HEX );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : base64\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BASE64 );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : base64url\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::BASE64URL );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$rule-2 = : any\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "rule-2" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ANY );

        TTEST( ph.grammar().rules[0].min.is_set() == false );
        TTEST( ph.grammar().rules[0].max.is_set() == false );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : @{root} null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].annotations.is_root == true );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }
}

TFEATURE( "GrammarParser - Syntax parsing - root rule" )
{
    {
        TSETUP( ParserHarness ph( " integer" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].annotations.is_root == true );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "integer \n $my_rule=: null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[1].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[1].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "#ruleset-id http://example.com/jcr\n integer \n $my_rule = : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().ruleset_id == "http://example.com/jcr" );
        TCRITICALTEST( ph.grammar().rules.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[1].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[1].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( " {}" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].annotations.is_root == true );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( " [   ]" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].annotations.is_root == true );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TTEST( ph.grammar().rules[0].repetition.min == 1 );
        TTEST( ph.grammar().rules[0].repetition.max == 1 );
    }
    TCALL( test_parsing_bad_input(
                        ": junk\n" ) );
    TCALL( test_parsing_bad_input(
                        "* junk\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - Member name" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = \"Fred\" : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_literal() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "Fred" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = /p_ref\\d+/i : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_regex() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "/p_ref\\d+/i" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = /p_ref\\d+/i : $my_type\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_regex() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "/p_ref\\d+/i" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].target_rule.local_name == "my_type" );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = \"abc\xE0\xAC\x8Bz\" : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_literal() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "abc\xE0\xAC\x8Bz" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = \"X\\u0802A\" : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_literal() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "X\xE0\xA0\x82""A" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = \"abc\\uD808\\uDF45=Ra\" : null\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.is_literal() == true );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "abc\xF0\x92\x8D\x85=Ra" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TNULL );
    }

    TCALL( test_parsing_bad_input(
                        "$my_rule = /p_ref\\d+/i * integer\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - type-choice" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = : (null)\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : ( null | integer )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : ( null | integer |\"my string\" )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 3 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[2].type == Rule::STRING_LITERAL );
        TCRITICALTEST( ph.grammar().rules[0].children[2].min.is_string() == true );
        TCRITICALTEST( ph.grammar().rules[0].children[2].min.as_string() == "my string" );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : ( null | (integer|float ) | \"my string\" )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 3 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::TYPE_CHOICE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[2].type == Rule::STRING_LITERAL );
        TCRITICALTEST( ph.grammar().rules[0].children[2].min.is_string() == true );
        TCRITICALTEST( ph.grammar().rules[0].children[2].min.as_string() == "my string" );
    }
    {
        TSETUP( ParserHarness ph( "#import http://common.com as common \n $my_rule =: ( null | $common.my_type )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.rulesetid == "http://common.com" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.local_name == "my_type" );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : @{not} ( null | integer )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::TYPE_CHOICE );
        TCRITICALTEST( ph.grammar().rules[0].annotations.is_not == true );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TNULL );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::INTEGER );
    }
    TCALL( test_parsing_bad_input(
                        "$my_rule = : ( )\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = : ( | )\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = : ( integer | )\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = : ( integer * )\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = : ( integer : null )\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - object" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"member\" : integer }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = :{ \"member\" : integer }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"int-member\" : integer , \"string-member\" : string }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "int-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.name() == "string-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"int-member\" : integer | \"string-member\" : string }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Choice );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "int-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.name() == "string-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"int-member\" : integer , $other-rule }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "int-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.local_name == "other-rule" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.rulesetid == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.p_rule == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"member\" : integer @? }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"member\" : integer @0..1 }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { /p-\\d+/ : integer @+ }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_regex() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "/p-\\d+/" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule ={/p-\\d+/:integer@5..}\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_regex() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "/p-\\d+/" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule ={/p-\\d+/i:integer@5..12}\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_regex() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "/p-\\d+/i" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 12 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { ( \"int-member\" : integer , \"string-member\" : string )@? }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::None );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::OBJECT_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].member_name.name() == "int-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].member_name.name() == "string-member" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].repetition.max == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"flt\":float @?, ((\"int\": integer, \"flt2\":float) | \"str\" : string) }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "flt" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 0 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::OBJECT_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::OBJECT_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].member_name.name() == "int" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].children.size() == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.name() == "flt2" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].children.size() == 0 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].member_name.name() == "str" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].children.size() == 0 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"flt\":float @?, () }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "flt" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 0 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::OBJECT_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::None );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 0 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"member\" : { \"child\" : integer } }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::OBJECT );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { ( \"member\" : integer ) }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::OBJECT_GROUP );
    }
    TCALL( test_parsing_bad_input(
                        "$my_rule = : { \"flt\":float@?, \"flt2\":float | \"str\" : string }\n" ) );   // Can't mix sequence and choice combiners
    TCALL( test_parsing_bad_input(
                        "$my_rule = : {\"flt\":float @?, }\n" ) );  // No member defined after comma
}

TFEATURE( "GrammarParser - Syntax parsing - array" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = : [ integer ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@* ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = \"MyName\" : [ integer @* ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "MyName" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = \"MyName\":[integer@*]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].member_name.name() == "MyName" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer, string ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer | string ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Choice );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer | $my_type ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Choice );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].target_rule.local_name == "my_type" );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ (integer | string)@5 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::None );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ (integer , string)@5 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::None );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ : (integer | string)@5 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::None );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TYPE_CHOICE );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ : (integer , string)@5 ]\n" ) );
        TCRITICALTEST( ph.status() != JCRParser::S_OK );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ type (integer , string)@5 ]\n" ) );
        TCRITICALTEST( ph.status() != JCRParser::S_OK );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ float@5, ( integer | string) ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ float @5, ( integer | string) @?]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ float @5,(integer | string@+) @? ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.max == -1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ float @5 , (( integer, float) | string) ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::ARRAY_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
    }
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer, float | string ]\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer, ]\n" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - group" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = ( integer )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( \"name\" : integer )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "name" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( /p-\\d+/ : integer )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_regex() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "/p-\\d+/" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( $other-rule )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].rule_name == "" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::TARGET_RULE );
        TCRITICALTEST( ph.grammar().rules[0].children[0].target_rule.local_name == "other-rule" );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( float @5, ((integer, float) | string) )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::GROUP_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::GROUP_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( \"flt\":float@?, ((\"int\": integer, \"flt2\":float) | \"str\" : string) )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.name() == "flt" );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 0 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::GROUP_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );
        TCRITICALTEST( ph.grammar().rules[0].children[1].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::GROUP_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].child_combiner == Rule::Sequence );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].member_name.is_absent() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children.size() == 2 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].member_name.name() == "int" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].children.size() == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].type == Rule::FLOAT );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.name() == "flt2" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].children.size() == 0 );

        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::STRING_TYPE );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].member_name.is_literal() );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].member_name.name() == "str" );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].children.size() == 0 );
    }
    {
        // This rule should fail higher level validity parsing but is valid at the parsing level
        TSETUP( ParserHarness ph( "#{import http://foo.com as my_alias} $my_rule = ( float@5, (( 1..5, \"name\":float) | $my_alias.other-rule | [ integer, /p*/ ] ) )\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children.size() == 2 );
        TCRITICALTEST( ph.grammar().rules[0].child_combiner == Rule::Sequence );

            TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::FLOAT );
            TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
            TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
            TCRITICALTEST( ph.grammar().rules[0].children[0].member_name.is_absent() );

            TCRITICALTEST( ph.grammar().rules[0].children[1].type == Rule::GROUP_GROUP );
            TCRITICALTEST( ph.grammar().rules[0].children[1].children.size() == 3 );
            TCRITICALTEST( ph.grammar().rules[0].children[1].child_combiner == Rule::Choice );

                TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].type == Rule::GROUP_GROUP );
                TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children.size() == 2 );

                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].type == Rule::UINTEGER );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].min.is_uint() == true );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].min.as_uint() == 1 );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].max.is_uint() == true );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[0].max.as_uint() == 5 );

                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].type == Rule::FLOAT );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.is_literal() );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[0].children[1].member_name.name() == "name" );

                TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].type == Rule::TARGET_RULE );
                TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].target_rule.rulesetid == "http://foo.com" );
                TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].target_rule.local_name == "other-rule" );
                TCRITICALTEST( ph.grammar().rules[0].children[1].children[1].children.size() == 0 );

                TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].type == Rule::ARRAY );
                TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children.size() == 2 );

                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[0].type == Rule::INTEGER );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[0].member_name.is_absent() );

                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[1].type == Rule::STRING_REGEX );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[1].member_name.is_absent() );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[1].min.is_string() == true );
                    TCRITICALTEST( ph.grammar().rules[0].children[1].children[2].children[1].min.as_string() == "/p*/" );
    }
    TCALL( test_parsing_bad_input(
                        "$my_rule = (integer,float|string )\n" ) );   // Can't mix sequence and choice combiners
    TCALL( test_parsing_bad_input(
                        "$my_rule = (integer, )\n" ) );  // No member defined after comma
}

TFEATURE( "GrammarParser - Syntax parsing - repetition" )
{
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].rule_name == "my_rule" );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer @? ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@* ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@*%3 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 3 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [integer@+]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [integer@+%2]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 2 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [  integer @5  ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@5.. ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@5..%7 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 7 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@145.. ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer@145..%17 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == -1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 17 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=[integer@..5]" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=[integer@..5%2]" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 2 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=[integer@..145]" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule=[integer@..145%32]" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 0 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 32 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer @2..5 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer @2..5%4 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 4 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer @142..145 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 142 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 1 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = [ integer @142..145%24 ]\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::ARRAY );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 142 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 145 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 24 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { \"size\" : integer @2..5%4 }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 4 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = { (\"size\" : integer) @2..5%4 }\n" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::OBJECT );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );

        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::OBJECT_GROUP );
        TCRITICALTEST( ph.grammar().rules[0].children[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 4 );
    }
    {
        TSETUP( ParserHarness ph( "$my_rule = ( \"size\" : integer @2..5%4 )" ) );
        TCRITICALTEST( ph.status() == JCRParser::S_OK );
        TCRITICALTEST( ph.grammar().rules.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].type == Rule::GROUP );

        TCRITICALTEST( ph.grammar().rules[0].children.size() == 1 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].type == Rule::INTEGER );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.min == 2 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.max == 5 );
        TCRITICALTEST( ph.grammar().rules[0].children[0].repetition.step == 4 );
    }
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @142.]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @142..% ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @.. ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @..%24 ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @..145%-12 ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @-142..-145%24 ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @..-145 ]" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule = [ integer @-142.. ]" ) );
}

TFEATURE( "GrammarParser - Syntax parsing - annotations" )
{
    TCALL( test_parsing_only(
                        "$my_rule = @{not} $other_rule\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type} string, float ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type} string, @{when $type} float@? ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type} string, @{when $type}@{assert $ > 15.0} float @? ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type}@{assert $==\"in\" || $==\"out\"} string, float @? ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type}@{assert $==/^\\w{1,4}$/} string, float@? ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule = [ @{id type}@{assert $==/^\\w{1,4}$/ ; Must for 4 or less chars{};} string, float@? ]\n" ) );
    TCALL( test_parsing_only(
                        "$my_rule=[@{not}string,float ]\n" ) );
    TCALL( test_parsing_bad_input(
                        "$my_rule=[@{unknown}string,float ]\n" ) );
}
