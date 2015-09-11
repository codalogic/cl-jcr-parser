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

class ParserHarness
{
private:
    struct Members {
        GrammarSet grammar_set;
        JCRParser jcr_parser;
        JCRParser::Status status;

        Members() : jcr_parser( &grammar_set ) {}
    } m;

public:
    ParserHarness( const char * p_jcr )
    {
        m.status = m.jcr_parser.add_grammar( p_jcr, strlen( p_jcr ) );
    }
    const JCRParser::Status status() const { return m.status; }
    const Grammar & grammar() const { return m.grammar_set[0]; }
};

void test_c_wsp( const char * p_jcr )
{
    ParserHarness h( p_jcr );

    TCRITICALTEST( h.status() == JCRParser::S_OK );
    TCRITICALTEST( h.grammar().size() == 1 );
    TCRITICALTEST( h.grammar()[0].is_directive() );
    TCRITICALTEST( h.grammar()[0].directive().get() == "pedantic" );
}

void test_c_wsp_2_directives( const char * p_jcr )
{
    ParserHarness h( p_jcr );

    TCRITICALTEST( h.status() == JCRParser::S_OK );
    TCRITICALTEST( h.grammar().size() == 2 );
    TCRITICALTEST( h.grammar()[0].is_directive() );
    TCRITICALTEST( h.grammar()[0].directive().get() == "pedantic" );
    TCRITICALTEST( h.grammar()[1].is_directive() );
    TCRITICALTEST( h.grammar()[1].directive().get() == "language-compatible-members" );
}

TFEATURE( "GrammarParser - parsing c-wsp" )
{
    TCALL( test_c_wsp( "#pedantic" ) );
    TCALL( test_c_wsp( "    #pedantic" ) );
    TCALL( test_c_wsp( ";My first jcr\n"
                        "#pedantic" ) );
    TCALL( test_c_wsp( "   ;My first jcr\n"
                        "#pedantic" ) );
    TCALL( test_c_wsp( "   ;My first jcr\n"
                        ";More grammars\r\n"
                        "#pedantic" ) );
    TCALL( test_c_wsp( "   \n"
                        "   ;My first jcr\n"
                        ";More grammars\r\n"
                        "#pedantic" ) );
    TCALL( test_c_wsp( "   ;My first jcr\n"
                        ";More grammars\r\n"
                        "#pedantic\n"
                        "  ; Stuff at end" ) );

    TCALL( test_c_wsp_2_directives(
                        "#pedantic\n"
                        "#language-compatible-members" ) );
    TCALL( test_c_wsp_2_directives(
                        "    #pedantic\n"
                        "    #language-compatible-members" ) );
    TCALL( test_c_wsp_2_directives(
                        "; A comment\n"
                        "#pedantic\n"
                        "; Another comment\n"
                        "#language-compatible-members" ) );
    TCALL( test_c_wsp_2_directives(
                        "   ; A comment\n"
                        "#pedantic\n"
                        "   ; Another comment\n"
                        "#language-compatible-members" ) );
    TCALL( test_c_wsp_2_directives(
                        "   ; A comment\n"
                        "#pedantic\n"
                        "   ; Another comment\n"
                        "#language-compatible-members\r\n"
                        ";More comments\n" ) );
    TCALL( test_c_wsp_2_directives(
                        "   ; A comment\n"
                        "#pedantic\n"
                        "   ; Another comment\n"
                        "#language-compatible-members\r\n"
                        "    ;More comments\n" ) );
    TCALL( test_c_wsp_2_directives(
                        "   ; A comment\n"
                        "#pedantic\n"
                        "   ; Another comment\n"
                        "   ; Also a comment\n"
                        "#language-compatible-members\r\n"
                        "    ;More comments\n" ) );
}

void test_2_directives( const char * p_jcr )
{
    ParserHarness h( p_jcr );
    TCRITICALTEST( h.status() == JCRParser::S_OK );
    TCRITICALTEST( h.grammar().size() == 2 );

    TCRITICALTEST( h.grammar()[0].is_directive() );
    TTEST( h.grammar()[0].directive().get() == "name http://example.com/myjcr.jcr" );
    TCRITICALTEST( h.grammar()[0].directive().size() == 2 );
    TTEST( h.grammar()[0].directive()[0] == "name" );
    TTEST( h.grammar()[0].directive().get(1) == "http://example.com/myjcr.jcr" );

    TCRITICALTEST( h.grammar()[1].is_directive() );
    TCRITICALTEST( h.grammar()[1].directive().size() == 4 );
    TTEST( h.grammar()[1].directive().get() == "import http://example.com/otherjcr.jcr as other" );
    TTEST( h.grammar()[1].directive()[0] == "import" );
    TTEST( h.grammar()[1].directive().get(1) == "http://example.com/otherjcr.jcr" );
    TTEST( h.grammar()[1].directive().get(2) == "as" );
    TTEST( h.grammar()[1].directive().get(3) == "other" );
}

TFEATURE( "GrammarParser - parsing directives" )
{
    TCALL( test_2_directives(
                "#name http://example.com/myjcr.jcr\n"
                "#import http://example.com/otherjcr.jcr as other" ) );
    TCALL( test_2_directives(
                "#name http://example.com/myjcr.jcr\n"
                "#import http://example.com/otherjcr.jcr as other\n" ) );
    TCALL( test_2_directives(
                "\n"
                "#name http://example.com/myjcr.jcr\n"
                "\n"
                "#import http://example.com/otherjcr.jcr as other\n"
                "\n" ) );
    TCALL( test_2_directives(
                "; with comments\n"
                "#name http://example.com/myjcr.jcr\n"
                "; Another comment\n"
                "#import http://example.com/otherjcr.jcr as other\n"
                "\n" ) );
    TCALL( test_2_directives(
                "; with comments\n"
                "   \n"
                "#name http://example.com/myjcr.jcr\n"
                "\n"
                "; Another comment\n"
                "\n"
                "#import http://example.com/otherjcr.jcr as other\n"
                "; Yet another comment\n"
                "\n" ) );
}
