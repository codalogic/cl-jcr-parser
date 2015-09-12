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
    Directive & r_directive = grammar.append_directive();
    Rule & r_rule = ValueRule::make_and_append( grammar );

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

TFEATURETODO( "Verify if we really need Grammar::append_rule()" )

TFEATURE( "Creating various types of Rules in Grammars" )
{
    {
    Grammar grammar;
    ValueRule & r_rule = ValueRule::make_and_append( grammar );
    TTEST( r_rule.is_value_rule() );
    TTEST( ! r_rule.is_array_rule() );
    TTEST( ! r_rule.is_object_rule() );
    TTEST( ! r_rule.is_group_rule() );
    TTEST( ! r_rule.is_ref_rule() );
    TTEST( r_rule.rule_kind() == RuleKind::UNDEFINED_VALUE );
    }

    {
    Grammar grammar;
    ArrayRule & r_rule = ArrayRule::make_and_append( grammar );
    TTEST( ! r_rule.is_value_rule() );
    TTEST( r_rule.is_array_rule() );
    TTEST( ! r_rule.is_object_rule() );
    TTEST( ! r_rule.is_group_rule() );
    TTEST( ! r_rule.is_ref_rule() );
    TTEST( r_rule.rule_kind() == RuleKind::ARRAY );
    }

    {
    Grammar grammar;
    ObjectRule & r_rule = ObjectRule::make_and_append( grammar );
    TTEST( ! r_rule.is_value_rule() );
    TTEST( ! r_rule.is_array_rule() );
    TTEST( r_rule.is_object_rule() );
    TTEST( ! r_rule.is_group_rule() );
    TTEST( ! r_rule.is_ref_rule() );
    TTEST( r_rule.rule_kind() == RuleKind::OBJECT );
    }

    {
    Grammar grammar;
    GroupRule & r_rule = GroupRule::make_and_append( grammar );
    TTEST( ! r_rule.is_value_rule() );
    TTEST( ! r_rule.is_array_rule() );
    TTEST( ! r_rule.is_object_rule() );
    TTEST( r_rule.is_group_rule() );
    TTEST( ! r_rule.is_ref_rule() );
    TTEST( r_rule.rule_kind() == RuleKind::GROUP );
    }

    {
    Grammar grammar;
    RefRule & r_rule = RefRule::make_and_append( grammar );
    TTEST( ! r_rule.is_value_rule() );
    TTEST( ! r_rule.is_array_rule() );
    TTEST( ! r_rule.is_object_rule() );
    TTEST( ! r_rule.is_group_rule() );
    TTEST( r_rule.is_ref_rule() );
    TTEST( r_rule.rule_kind() == RuleKind::REF );
    }
}

TFEATURE( "Creating various types of ValueRule, e.g. SimpleType, EnumType etc." )
{
    {
    Grammar grammar;
    ValueRule & r_value_rule = ValueRule::make_and_append( grammar );
    TTEST( r_value_rule.is_value_rule() );
    TTEST( ! r_value_rule.is_simple_type() );
    TTEST( ! r_value_rule.is_enum_type() );
    TTEST( ! r_value_rule.is_union_type() );
    TTEST( r_value_rule.rule_kind() == RuleKind::UNDEFINED_VALUE );
#ifdef NDEBUG
    // Exceptions are thrown if wrong type is chosen after selection

    // These tests don't in Debug mode because there are also asserts in place.
    bool is_thrown = false;
    try
    {
        is_thrown = false;
        r_value_rule.simple_type();
        TTEST( false );
    }
    catch( BadSimpleTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.enum_type();
        TTEST( false );
    }
    catch( BadEnumTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.union_type();
        TTEST( false );
    }
    catch( BadUnionTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
#endif
    }

    {
    Grammar grammar;
    ValueRule & r_value_rule = ValueRule::make_and_append( grammar );
    r_value_rule.select_simple_type();
    TTEST( r_value_rule.is_simple_type() );
    TTEST( ! r_value_rule.is_enum_type() );
    TTEST( ! r_value_rule.is_union_type() );
    TTEST( r_value_rule.rule_kind() == RuleKind::SIMPLE_VALUE );

#ifdef NDEBUG
    // Exceptions are thrown if wrong type is chosen after selection

    // These tests don't in Debug mode because there are also asserts in place.
    bool is_thrown = false;
    try
    {
        is_thrown = false;
        r_value_rule.simple_type();
    }
    catch( BadSimpleTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( ! is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.enum_type();
        TTEST( false );
    }
    catch( BadEnumTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.union_type();
        TTEST( false );
    }
    catch( BadUnionTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
#endif
    }

    {
    Grammar grammar;
    ValueRule & r_value_rule = ValueRule::make_and_append( grammar );
    r_value_rule.select_enum_type();
    TTEST( ! r_value_rule.is_simple_type() );
    TTEST( r_value_rule.is_enum_type() );
    TTEST( ! r_value_rule.is_union_type() );
    TTEST( r_value_rule.rule_kind() == RuleKind::ENUM_VALUE );

#ifdef NDEBUG
    // Exceptions are thrown if wrong type is chosen after selection

    // These tests don't in Debug mode because there are also asserts in place.
    bool is_thrown = false;
    try
    {
        is_thrown = false;
        r_value_rule.simple_type();
        TTEST( false );
    }
    catch( BadSimpleTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.enum_type();
    }
    catch( BadEnumTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( ! is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.union_type();
        TTEST( false );
    }
    catch( BadUnionTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
#endif
    }

    {
    Grammar grammar;
    ValueRule & r_value_rule = ValueRule::make_and_append( grammar );
    r_value_rule.select_union_type();
    TTEST( ! r_value_rule.is_simple_type() );
    TTEST( ! r_value_rule.is_enum_type() );
    TTEST( r_value_rule.is_union_type() );
    TTEST( r_value_rule.rule_kind() == RuleKind::UNION_VALUE );

#ifdef NDEBUG
    // Exceptions are thrown if wrong type is chosen after selection

    // These tests don't in Debug mode because there are also asserts in place.
    bool is_thrown = false;
    try
    {
        is_thrown = false;
        r_value_rule.simple_type();
        TTEST( false );
    }
    catch( BadSimpleTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.enum_type();
        TTEST( false );
    }
    catch( BadEnumTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( is_thrown );
    try
    {
        is_thrown = false;
        r_value_rule.union_type();
    }
    catch( BadUnionTypeRequest & )
    {
        is_thrown = true;
    }
    TTEST( ! is_thrown );
#endif
    }
}
