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

TFEATURE( "RuleOrDirective creation" )
{
    {
    RuleOrDirective::uniq_ptr pu_rule_or_directive( RuleOrDirective::make_rule() );

    TTEST( pu_rule_or_directive->is_rule() == true );
    TTEST( pu_rule_or_directive->is_directive() == false );
    TTEST( pu_rule_or_directive->rule() != 0 );
    try
    {
        pu_rule_or_directive->directive();
        TTEST( false );
    }
    catch( BadDirectiveRequest & )
    {
        TTEST( true );
    }
    }

    {
    RuleOrDirective::uniq_ptr pu_rule_or_directive( RuleOrDirective::make_directive() );

    TTEST( pu_rule_or_directive->is_directive() == true );
    TTEST( pu_rule_or_directive->is_rule() == false );
    TTEST( pu_rule_or_directive->directive() != 0 );
    try
    {
        pu_rule_or_directive->rule();
        TTEST( false );
    }
    catch( BadRuleRequest & )
    {
        TTEST( true );
    }
    }
}

TFEATURE( "Grammar creation / deletion" )
{
    {
    Grammar grammar;
    Directive * p_directive = grammar.append_directive();
    Rule * p_rule = grammar.append_rule();

    Grammar::const_iterator i = grammar.begin(), end = grammar.end();
    TTEST( i != end );
    TTEST( i->is_directive() )
    TTEST( ! (*i).is_rule() )
    ++i;
    TTEST( i != end );
    TTEST( ! i->is_directive() )
    TTEST( i->is_rule() )
    ++i;
    TTEST( i == end );

    TTEST( grammar[0].is_directive() )
    TTEST( ! grammar[0].is_rule() )
    TTEST( ! grammar[1].is_directive() )
    TTEST( grammar[1].is_rule() )
    }

    {
    Grammar grammar;
    grammar.append( new Directive() );
    TTEST( grammar.back_is_directive() );
    TTEST( ! grammar.back_is_rule() );
    grammar.append( new Rule() );
    TTEST( grammar.back_is_rule() );
    TTEST( ! grammar.back_is_directive() );

    Grammar::iterator i = grammar.begin(), end = grammar.end();
    TTEST( i != end );
    TTEST( i->is_directive() )
    TTEST( ! i->is_rule() )
    ++i;
    TTEST( i != end );
    TTEST( ! i->is_directive() )
    TTEST( i->is_rule() )
    ++i;
    TTEST( i == end );

    TTEST( grammar[0].is_directive() )
    TTEST( ! grammar[0].is_rule() )
    TTEST( ! grammar[1].is_directive() )
    TTEST( grammar[1].is_rule() )
    }
}

TFEATURE( "Grammar exception throwing on bad selection" )
{
    {
    Grammar grammar;
    grammar.append( new Directive() );

    try
    {
        grammar.back_directive();
        TTEST( true );
    }
    catch( BadDirectiveRequest & )
    {
        TTEST( false );
    }

    try
    {
        grammar.back_rule();
        TTEST( false );
    }
    catch( BadRuleRequest & )
    {
        TTEST( true );
    }

    try
    {
        grammar.back_rule();
        TTEST( false );
    }
    catch( BadRuleOrDirectiveRequest & )
    {
        TTEST( true );
    }

    grammar.append( new Rule() );
    try
    {
        grammar.back_rule();
        TTEST( true );
    }
    catch( BadRuleRequest & )
    {
        TTEST( false );
    }

    try
    {
        grammar.back_directive();
        TTEST( false );
    }
    catch( BadDirectiveRequest & )
    {
        TTEST( true );
    }
    }
}

TFEATURETODO( "Verify if we really need Grammar::append_directive() & append_rule()" )
