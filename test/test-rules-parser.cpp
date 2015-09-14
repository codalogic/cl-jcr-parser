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

void test_simple_anonymous_value_rule( const char * p_jcr, SimpleType::Type expected_type )
{
    ParserHarness h( p_jcr );

    TCRITICALTEST( h.status() == JCRParser::S_OK );
    TCRITICALTEST( h.grammar().size() == 1 );
    TCRITICALTEST( h.grammar()[0].is_rule() );

    TCRITICALTEST( SimpleType::is_present( h.grammar()[0].rule() ) );

    const SimpleType * p_simple_type = SimpleType::from_rule( h.grammar()[0].rule() );
    TCRITICALTEST( p_simple_type != 0 );    // Actually redundant when we've already done SimpleType::is_present() check
    TTEST( p_simple_type->type() == expected_type );
    TTEST( h.grammar()[0].rule().rule_name() == "rule1" );
}

TFEATURE( "GrammarParser - parsing trivial simple value rules" )
{
    TCALL( test_simple_anonymous_value_rule( "rule1 : boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : null", SimpleType::TNULL ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : ip4", SimpleType::IP4 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : ip6", SimpleType::IP6 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : fqdn ", SimpleType::FQDN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : idn", SimpleType::IDN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : date-time", SimpleType::DATETIME ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : full-date", SimpleType::DATE ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : full-time", SimpleType::TIME ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : email", SimpleType::EMAIL ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : phone", SimpleType::PHONE ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : base64", SimpleType::BASE64 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : any", SimpleType::ANY ) );

    TCALL( test_simple_anonymous_value_rule( "rule1 : BOOLEAN", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : NULL", SimpleType::TNULL ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : IP4", SimpleType::IP4 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : IP6", SimpleType::IP6 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : FQDN ", SimpleType::FQDN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : IDN", SimpleType::IDN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : DATE-TIME", SimpleType::DATETIME ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : FULL-DATE", SimpleType::DATE ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : FULL-TIME", SimpleType::TIME ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : EMAIL", SimpleType::EMAIL ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : PHONE", SimpleType::PHONE ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : BASE64", SimpleType::BASE64 ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : ANY", SimpleType::ANY ) );
}

TFEATURE( "GrammarParser - spacing around parsing trivial simple value rules" )
{
    TCALL( test_simple_anonymous_value_rule( "rule1 : boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1: boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 :boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1:boolean", SimpleType::BOOLEAN ) );

    TCALL( test_simple_anonymous_value_rule( "rule1 : boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( " rule1 : boolean", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( "rule1 : boolean ", SimpleType::BOOLEAN ) );
    TCALL( test_simple_anonymous_value_rule( " rule1 : boolean ", SimpleType::BOOLEAN ) );
}
