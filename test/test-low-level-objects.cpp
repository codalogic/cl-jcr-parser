//----------------------------------------------------------------------------
// Copyright (c) 2015-2017, Codalogic Ltd (http://www.codalogic.com)
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

#include "cl-jcr-parser/parser.h"

using namespace cljcr;

TFEATURE( "ValueConstraint" )
{
    ValueConstraint vc;

    TTEST( vc.is_set() == false );

    TSETUP( vc = "string" );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "string" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.as_pattern() == "string" );   // Regex constraint is stored as a string, with relevant /s
    TTEST( vc.as_modifiers() == "" );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = std::string( "string" ) );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "string" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.as_pattern() == "string" );   // Regex constraint is stored as a string, with relevant /s
    TTEST( vc.as_modifiers() == "" );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = std::string( "/regex/" ) );   // Regex constraint is stored as a string, with relevant /s
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "/regex/" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.as_pattern() == "regex" );
    TTEST( vc.as_modifiers() == "" );

    TSETUP( vc = std::string( "/regex/i" ) );   // Regex constraint is stored as a string, with relevant /s
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "/regex/i" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.as_pattern() == "regex" );
    TTEST( vc.as_modifiers() == "i" );

    TSETUP( vc = true );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_bool() == true );
    TTEST( vc.as_bool() == true );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = false );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_bool() == true );
    TTEST( vc.as_bool() == false );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = (int64)10 );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_int() == true );
    TTEST( vc.as_int() == 10 );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = (uint64)10 );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_uint() == true );
    TTEST( vc.as_uint() == 10 );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = 10.5 );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_float() == true );
    TTEST( vc.as_float() == 10.5 );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );

    TSETUP( vc = 2.5e+3 );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_float() == true );
    TTEST( vc.as_float() == 2500.0 );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );

    TSETUP( vc.clear() );
    TTEST( vc.is_set() == false );
    TTEST( vc.is_string() == false );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );
}

TFEATURE( "MemberName" )
{
    MemberName mn;

    TTEST( mn.is_absent() == true );
    TTEST( mn.is_literal() == false );
    TTEST( mn.is_regex() == false );
    TTEST( mn.name() == "" );

    TSETUP( mn.set_literal( "foo" ) );
    TTEST( mn.is_absent() == false );
    TTEST( mn.is_literal() == true );
    TTEST( mn.is_regex() == false );
    TTEST( mn.name() == "foo" );

    TSETUP( mn.set_regex( "/name*/i" ) );
    TTEST( mn.is_absent() == false );
    TTEST( mn.is_literal() == false );
    TTEST( mn.is_regex() == true );
    TTEST( mn.name() == "/name*/i" );
    TTEST( mn.pattern() == "name*" );
    TTEST( mn.modifiers() == "i" );

    TSETUP( mn.set_regex( "//" ) );
    TTEST( mn.pattern() == "" );
    TTEST( mn.modifiers() == "" );

    TSETUP( mn.set_absent() );
    TTEST( mn.is_absent() == true );
    TTEST( mn.is_literal() == false );
    TTEST( mn.is_regex() == false );
    TTEST( mn.name() == "" );

    TSETUP( mn.set_literal( "foo" ) );
    TTEST( mn.is_literal() == true );
    TSETUP( mn.clear() );
    TTEST( mn.is_absent() == true );
    TTEST( mn.is_literal() == false );
    TTEST( mn.is_regex() == false );
    TTEST( mn.name() == "" );
}

TFEATURE( "TargetRule" )
{
    TargetRule tr;

    TTEST( ! tr.p_rule );
}

TFEATURE( "Rule" )
{
    GrammarSet gs;
    Grammar * p_g = gs.append_grammar();
    Rule r( p_g, 100, 102 );

    TTEST( r.p_grammar == p_g );
    TTEST( ! r.p_parent );
    TTEST( r.line_number == 100 );
    TTEST( r.column_number == 102 );
    TTEST( r.p_rule == &r );
    TTEST( r.p_type == &r );
    TTEST( r.type == Rule::NONE );

    TSETUP( Rule::uniq_ptr pu_rule( new Rule( p_g, 0, 0 ) ) );

    TTEST( ! pu_rule->p_parent );
    TTEST( pu_rule->type == Rule::NONE );

    TSETUP( Rule * p_new_rule = pu_rule.get() );
    TSETUP( Rule * p_appended_rule = r.append_child_rule( pu_rule ) );
    TTEST( p_appended_rule == p_new_rule );
    TTEST( p_appended_rule->p_parent == &r );
}

TFEATURE( "Post-link Rule" )
{
    // We set up values in this test that are inconsistent with a real application.
    // This is so we can verify that the correct instances are being accessed.
    GrammarSet gs;
    Grammar * p_g = gs.append_grammar();
    Rule def( p_g, 100, 102 );
    def.rule_name = "def";
    def.repetition.min = 100;
    def.repetition.max = 101;
    def.annotations.is_root = true;
    def.type = Rule::TARGET_RULE;
    def.child_combiner = Rule::None;
    def.target_rule.rule_name = "rule";

    Rule rule( p_g, 300, 502 );
    rule.rule_name = "rule";
    rule.annotations.is_not = true;
    rule.member_name.set_literal( "rule" );
    rule.type = Rule::TARGET_RULE;
    rule.child_combiner = Rule::None;
    rule.target_rule.rule_name = "type";

    Rule type( p_g, 400, 602 );
    type.rule_name = "type";
    type.annotations.is_unordered = true;
    type.type = Rule::OBJECT;
    type.min = "min";   // Inconsistent with "type = Rule::OBJECT" to aid testing
    type.max = "max";   // Inconsistent with "type = Rule::OBJECT" to aid testing
    type.child_combiner = Rule::Sequence;

    Rule::uniq_ptr pu_child( new Rule( p_g, 700, 802 ) );
    pu_child->type = Rule::INTEGER;
    pu_child->min = (int64)302;
    pu_child->max = (int64)303;
    type.append_child_rule( pu_child );

    // The 'link' operation
    def.target_rule.p_rule = &rule;
    rule.target_rule.p_rule = &type;
    def.p_rule = def.target_rule.p_rule;
    def.p_type = def.p_rule->target_rule.p_rule;
    TTEST( def.p_rule == &rule );
    TTEST( def.p_type == &type );
    def.merge_target_annotations();

    // The tests
    TTEST( def.get_repetition().min == 100 );
    TTEST( def.get_repetition().max == 101 );
    TTEST( def.get_annotations().is_root );
    TTEST( def.get_annotations().is_not );
    TTEST( def.get_annotations().is_unordered );
    TTEST( def.get_member_name().name() == "rule" );
    TTEST( def.is_member_rule() == true );
    TTEST( def.is_type_rule() == false );
    TTEST( def.get_type() == Rule::OBJECT );
    TTEST( def.get_min().as_string() == "min" );
    TTEST( def.get_max().as_string() == "max" );
    TTEST( def.get_child_combiner() == Rule::Sequence );

    TTEST( ! def.get_children().empty() );
    TTEST( def.get_children()[0].type == Rule::INTEGER );
    TTEST( def.get_children()[0].min.as_int() == 302 );
    TTEST( def.get_children()[0].max.as_int() == 303 );
}

TFEATURE( "Grammar" )
{
    GrammarSet gs;
    Grammar * p_g = gs.append_grammar();

    TTEST( p_g->p_grammar_set == &gs );

    TDOC( "Adding and accessing unaliased imports" );
    TSETUP( p_g->add_unaliased_import( "foo" ) );
    TTEST( p_g->unaliased_imports.size() == 1 );
    TSETUP( p_g->add_unaliased_import( "bar" ) );
    TTEST( p_g->unaliased_imports.size() == 2 );
    TTEST( p_g->unaliased_imports[0] == "foo" );
    TTEST( p_g->unaliased_imports[1] == "bar" );

    TDOC( "Adding aliased imports" );
    TTEST( p_g->has_aliased_import( "foo" ) == false );
    TTEST( p_g->add_aliased_import( "foo", "http://foo" ) == true );
    TTEST( p_g->has_aliased_import( "foo" ) == true );
    TTEST( p_g->has_aliased_import( "bar" ) == false );

    TTEST( p_g->add_aliased_import( "bar", "http://bar" ) == true );
    TTEST( p_g->has_aliased_import( "bar" ) == true );
    TTEST( p_g->aliased_imports["foo"] == "http://foo" );
    TTEST( p_g->aliased_imports["bar"] == "http://bar" );

    TDOC( "Adding duplicate aliased imports fails" );
    TTEST( p_g->add_aliased_import( "foo", "http://foo" ) == false );

    TDOC( "Accessing aliased imports for const instances" );
    TSETUP( const Grammar * p_const_g( p_g ) );
    TTEST( p_const_g->get_aliased_import( "foo" ).value() == "http://foo" );
    TSETUP( std::string foo = p_const_g->get_aliased_import( "foo" ) );        // Can use AliasLookupResult cast to string & to assign directly to string
    TTEST( foo == "http://foo" );
    TTEST( p_const_g->get_aliased_import( "bar" ).value() == "http://bar" );

    TTEST( p_const_g->has_aliased_import( "blah" ) == false );
    TTEST( p_const_g->get_aliased_import( "blah" ).is_found() == false );

    TDOC( "Adding rules" );
    TTEST( p_g->rules.size() == 0 );
    TSETUP( Rule::uniq_ptr pu_r( new Rule( p_g, 0, 0 ) ) );
    TSETUP( pu_r->p_parent = pu_r.get() );  // Set p_parent to non-zero value so we can test it's set to 0 later
    TTEST( pu_r->p_parent != 0 );
    TSETUP( Rule * p_unmanaged_rule = pu_r.get() );
    TSETUP( Rule * p_r = p_g->append_rule( pu_r ) );
    TTEST( p_r == p_unmanaged_rule );   // Using append_rule() returns released pointer to rule
    TTEST( ! p_r->p_parent );           // Using append_rule() sets p-parent pointer to null
    TTEST( p_g->rules.size() == 1 );
}

TFEATURE( "Grammar::find_rule()" )
{
    GrammarSet gs;
    Grammar * p_g = gs.append_grammar();
    Rule::uniq_ptr pu_r1( new Rule( p_g, 0, 0 ) );
    pu_r1->rule_name = "r1";
    Rule * p_r1 = p_g->append_rule( pu_r1 ); // pu_r1 releases ownership here
    Rule::uniq_ptr pu_r2( new Rule( p_g, 0, 0 ) );
    pu_r2->rule_name = "r2";
    Rule * p_r2 = p_g->append_rule( pu_r2 ); // pu_r2 releases ownership here

    TTEST( p_g->find_rule( "r1" ) == p_r1 );
    TTEST( p_g->find_rule( "r2" ) == p_r2 );
    TTEST( p_g->find_rule( "r3" ) == 0 );

    const Grammar * p_const_g = p_g;
    TTEST( p_const_g->find_rule( "r1" ) == p_r1 );
    TTEST( p_const_g->find_rule( "r2" ) == p_r2 );
    TTEST( p_const_g->find_rule( "r3" ) == 0 );
}

TFEATURE( "GrammarSet::find_grammar()" )
{
    GrammarSet gs;
    Grammar * p_g1 = gs.append_grammar();
    p_g1->ruleset_id = "g1";
    Grammar * p_g2 = gs.append_grammar();
    p_g2->ruleset_id = "g2";

    TTEST( gs.find_grammar( "g1" ) == p_g1 );
    TTEST( gs.find_grammar( "g2" ) == p_g2 );
    TTEST( gs.find_grammar( "g3" ) == 0 );

    const GrammarSet & r_const_gs = gs;
    TTEST( r_const_gs.find_grammar( "g1" ) == p_g1 );
    TTEST( r_const_gs.find_grammar( "g2" ) == p_g2 );
    TTEST( r_const_gs.find_grammar( "g3" ) == 0 );
}

TFEATURETODO( "Test low level GrammarSet class" );
