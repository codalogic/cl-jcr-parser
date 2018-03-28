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

class GrammarModifier // To facilitate modifying already created grammars
{
    // Usage: GrammarModifier( p_grammar ).id( "Foo" ).import( "Bar" ).import( "Baz" );

private:
    Grammar * p_grammar;

public:
    GrammarModifier( Grammar * p_grammar_in ) : p_grammar( p_grammar_in ) {}  // For an already created grammar

    Grammar * grammar() { return p_grammar; }
    operator Grammar * () { return grammar(); }

    GrammarModifier & ruleset_id( const char * p_name ) { p_grammar->ruleset_id = p_name; return *this; }
    GrammarModifier & unaliased_import( const char * p_name ) { p_grammar->add_unaliased_import( p_name ); return *this; }
};

class GrammarMaker : public GrammarModifier // To facilitate making grammars for testing
{
    // Usage: Grammar * p_grammar = GrammarMaker( p_grammar_set ).id( "Foo" ).import( "Bar" ).import( "Baz" );

public:
    GrammarMaker( GrammarSet & r_grammar_set ) : GrammarModifier( r_grammar_set.append_grammar( "<locally generated test>" ) ) {}
};

class RuleModifier // To facilitate modifying already created rules
{
    // Usage: RuleModifier( p_rule ).rule_name( "Foo" ).target_rule_name( "Bar" );

private:
    Rule * p_rule;

public:
    RuleModifier( Rule * p_rule_in ) : p_rule( p_rule_in ) {}

    Rule * rule() { return p_rule; }
    operator Rule * () { return rule(); }

    RuleModifier & rule_name( const char * p_name ) { p_rule->rule_name = p_name; return *this; }
    RuleModifier & member_name( const char * p_name ) { p_rule->member_name.set_literal( p_name ); return *this; }
    RuleModifier & target_rule_name( const char * p_name ) { p_rule->target_rule.rule_name = p_name; return *this; }
    RuleModifier & target_ruleset_id( const char * p_name ) { p_rule->target_rule.ruleset_id = p_name; return *this; }
};

class RuleMaker : public RuleModifier // To facilitate making rules for testing
{
    // Usage: Rule * p_rule = RuleMaker( p_grammar ).rule_name( "Foo" ).target_rule_name( "Bar" );

public:
    RuleMaker( Grammar * p_grammar ) : RuleModifier( p_grammar->append_rule( Rule::uniq_ptr( new Rule( p_grammar, 0, 0 ) ) ) ) {}
    RuleMaker( Rule * p_parent_rule ) : RuleModifier( p_parent_rule->append_child_rule( Rule::uniq_ptr( new Rule( p_parent_rule->p_grammar, 0, 0 ) ) ) ) {}
};

TFEATURE( "Linking Rule::find_target_rule()" )
{
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );

    Grammar * p_g2 = GrammarMaker( gs ).unaliased_import( "g3" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" );
    Rule * p_g2r2 = RuleMaker( p_g2 ).rule_name( "g2r2" );

    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );
    // p_g3->add_aliased_import( "ag4", "g4" ); // Not needed - Aliases are mapped at parse time, and target_rule stores resultant ruleset_id
    Rule * p_g3r1 = RuleMaker( p_g3 ).rule_name( "g3r1" );
    Rule * p_g3r2 = RuleMaker( p_g3 ).rule_name( "g3r2" );

    Grammar * p_g4 = GrammarMaker( gs ).ruleset_id( "g4" );
    Rule * p_g4r1 = RuleMaker( p_g4 ).rule_name( "g4r1" );
    Rule * p_g4r2 = RuleMaker( p_g4 ).rule_name( "g4r2" );

    // Test no imports case
    TSETUP( p_g1r1->target_rule.rule_name = "g1r2" );
    TTEST( p_g1r1->find_target_rule() == p_g1r2 );
    TTEST( p_g1r1->target_rule.p_rule == p_g1r2 );  // Check also stores result in target_rule.p_rule

    // Test unaliased imports case
    TSETUP( p_g2r1->target_rule.rule_name = "g3r2" );
    TTEST( p_g2r1->find_target_rule() == p_g3r2 );
    TTEST( p_g2r1->target_rule.p_rule == p_g3r2 );  // Check also stores result in target_rule.p_rule

    // Test aliased imports case
    TSETUP( p_g3r1->target_rule.rule_name = "g4r2" );
    TSETUP( p_g3r1->target_rule.ruleset_id = "g4" );
    TTEST( p_g3r1->find_target_rule() == p_g4r2 );
    TTEST( p_g3r1->target_rule.p_rule == p_g4r2 );  // Check also stores result in target_rule.p_rule

    // Test behaviour when target rule not named
    TTEST( p_g3r2->find_target_rule() == p_g3r2 );
    TTEST( p_g3r2->target_rule.p_rule == p_g3r2 );  // Check also stores result in target_rule.p_rule

    // Test const case
    TSETUP( p_g1r2->target_rule.rule_name = "g1r1" );
    const Rule * p_const_g1r2 = p_g1r2;
    TTEST( p_const_g1r2->find_target_rule() == p_g1r1 );
    TTEST( p_const_g1r2->target_rule.p_rule == 0 ); // Const instance can't set target_rule.p_rule
}

TFEATURE( "Global linking - Check for duplicate rules" )
{
    {
    TDOC( "No duplication" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    }
    {
    TDOC( "Bottom name duplicates top name - 2 rules" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Bottom name duplicates top name - 4 rules" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "3rd name duplicates 2nd name" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Bottom name duplicates 2nd name" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Bottom name duplicates 3rd name" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Global linking - Local ruleset" )
{
    {
    TDOC( "One layer of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r1 );   // Points to self
    TTEST( p_g1->rules[0].p_type == p_g1r2 );
    }
    {
    TDOC( "Two layers of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r1 );   // Points to self
    TTEST( p_g1->rules[0].p_type == p_g1r3 );
    }
    {
    TDOC( "Three layers of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r1 );   // Points to self
    TTEST( p_g1->rules[0].p_type == p_g1r4 );
    }
}

TFEATURE( "Global linking - Local ruleset - with member rule" )
{
    {
    TDOC( "One layer of lookup - member rule @ 2nd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).                           member_name( "mg1r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_type == p_g1r2 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r2" );
    }
    {
    TDOC( "Two layers of lookup - member rule @ 2nd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" ).member_name( "mg1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_type == p_g1r3 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r2" );
    }
    {
    TDOC( "Two layers of lookup - member rule @ 3rd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).                           member_name( "mg1r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r3 );
    TTEST( p_g1->rules[0].p_type == p_g1r3 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r3" );
    }
    {
    TDOC( "Three layers of lookup - member rule @ 2nd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" ).member_name( "mg1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_type == p_g1r4 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r2" );
    }
    {
    TDOC( "Three layers of lookup - member rule @ 3rd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" ).member_name( "mg1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r3 );
    TTEST( p_g1->rules[0].p_type == p_g1r4 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r3" );
    }
    {
    TDOC( "Three layers of lookup - member rule @ 4th rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).                           member_name( "mg1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r4 );
    TTEST( p_g1->rules[0].p_type == p_g1r4 );
    TTEST( p_g1->rules[0].get_member_name().name() == "mg1r4" );
    }
}

TFEATURE( "Global linking - Local ruleset - with illegal multiple member rules" )
{
    {
    TDOC( "Three layers of lookup - member name on 1st and 4th rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" ).member_name( "mg1r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).                           member_name( "mg1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Three layers of lookup - member name on 2nd and 3rd rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" ).member_name( "mg1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" ).member_name( "mg1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Three layers of lookup - member name on 3rd and 4th rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" ).member_name( "mg1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).                           member_name( "mg1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Three layers of lookup - member name on 2nd and 4th rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" ).member_name( "mg1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).                           member_name( "mg1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Global linking - Local ruleset - with illegal loops" )
{
    {
    TDOC( "One layer of lookup - loop to self" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Two layers of lookup - loop to top" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Two layers of lookup - loopback to bottom" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to top" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).target_rule_name( "g1r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to 2nd" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).target_rule_name( "g1r2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to 3rd" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).target_rule_name( "g1r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to bottom" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g1r2" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g1r3" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "g1r4" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).target_rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Global link - to undefined rule names" )
{
    {
    TDOC( "First rule linking to undefined rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "undefined" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "3rd rule linking to undefined rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).target_rule_name( "undefined" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Multiple grammar linking - Check for duplicately (or multiply) named grammar ruleset-ids" )
{
    {
    TDOC( "Mustn't complain about no grammars" );
    GrammarSet gs;

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    }
    {
    TDOC( "Mustn't complain about a single grammar" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    }
    {
    TDOC( "Multiple annoymous ruleset-ids is OK - 2 grammars" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Grammar * p_g2 = GrammarMaker( gs );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    }
    {
    TDOC( "Multiple annoymous ruleset-ids is OK - 3 grammars" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Grammar * p_g2 = GrammarMaker( gs );
    Grammar * p_g3 = GrammarMaker( gs );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    }
    {
    TDOC( "Grammars with different ruleset-ids is OK" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    }
    {
    TDOC( "Must error with multiple grammars with same ruleset-id - test 1" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).ruleset_id( "g1" );
    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g1" );
    Grammar * p_g3 = GrammarMaker( gs );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() != JCRParser::S_OK );
    }
    {
    TDOC( "Must error with multiple grammars with same ruleset-id - test 2" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).ruleset_id( "g1" );
    Grammar * p_g2 = GrammarMaker( gs );
    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() != JCRParser::S_OK );
    }
    {
    TDOC( "Must error with multiple grammars with same ruleset-id - test 3" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g2" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() != JCRParser::S_OK );
    }
}

TFEATURE( "Multiple grammar linking - global rule linking" )
{
    {
    TDOC( "Unaliased imports - first ruleset doing linking" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).unaliased_import( "g2" ).unaliased_import( "g3" );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g2r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_rule_name( "g3r1" );
    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" );
    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );
    Rule * p_g3r1 = RuleMaker( p_g3 ).rule_name( "g3r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    TTEST( p_g1r1->target_rule.p_rule == p_g2r1 );
    TTEST( p_g1r1->p_rule == p_g1r1 );
    TTEST( p_g1r1->p_type == p_g2r1 );

    TTEST( p_g1r2->target_rule.p_rule == p_g3r1 );
    TTEST( p_g1r2->p_rule == p_g1r2 );
    TTEST( p_g1r2->p_type == p_g3r1 );
    }
    {
    TDOC( "Unaliased imports - second ruleset doing linking" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).ruleset_id( "g1" );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );

    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" ).unaliased_import( "g1" ).unaliased_import( "g3" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" ).target_rule_name( "g1r1" );
    Rule * p_g2r2 = RuleMaker( p_g2 ).rule_name( "g2r2" ).target_rule_name( "g3r1" );

    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );
    Rule * p_g3r1 = RuleMaker( p_g3 ).rule_name( "g3r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    TTEST( p_g2r1->target_rule.p_rule == p_g1r1 );
    TTEST( p_g2r1->p_rule == p_g2r1 );
    TTEST( p_g2r1->p_type == p_g1r1 );

    TTEST( p_g2r2->target_rule.p_rule == p_g3r1 );
    TTEST( p_g2r2->p_rule == p_g2r2 );
    TTEST( p_g2r2->p_type == p_g3r1 );
    }
    {
    TDOC( "Must fail to link to rule without a suitable import" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).ruleset_id( "g1" );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_rule_name( "g2r1" );

    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" ).target_rule_name( "g1r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() != JCRParser::S_OK );
    }
    {
    TDOC( "Aliased imports - first ruleset doing linking" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" ).target_ruleset_id( "g2" ).target_rule_name( "g2r1" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" ).target_ruleset_id( "g3" ).target_rule_name( "g3r1" );
    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" );
    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );
    Rule * p_g3r1 = RuleMaker( p_g3 ).rule_name( "g3r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    TTEST( p_g1r1->target_rule.p_rule == p_g2r1 );
    TTEST( p_g1r1->p_rule == p_g1r1 );
    TTEST( p_g1r1->p_type == p_g2r1 );

    TTEST( p_g1r2->target_rule.p_rule == p_g3r1 );
    TTEST( p_g1r2->p_rule == p_g1r2 );
    TTEST( p_g1r2->p_type == p_g3r1 );
    }
    {
    TDOC( "Aliased imports - second ruleset doing linking" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).ruleset_id( "g1" );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );

    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" ).target_ruleset_id( "g1" ).target_rule_name( "g1r1" );
    Rule * p_g2r2 = RuleMaker( p_g2 ).rule_name( "g2r2" ).target_ruleset_id( "g3" ).target_rule_name( "g3r1" );

    Grammar * p_g3 = GrammarMaker( gs ).ruleset_id( "g3" );
    Rule * p_g3r1 = RuleMaker( p_g3 ).rule_name( "g3r1" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link() == JCRParser::S_OK );
    TTEST( p_g2r1->target_rule.p_rule == p_g1r1 );
    TTEST( p_g2r1->p_rule == p_g2r1 );
    TTEST( p_g2r1->p_type == p_g1r1 );

    TTEST( p_g2r2->target_rule.p_rule == p_g3r1 );
    TTEST( p_g2r2->p_rule == p_g2r2 );
    TTEST( p_g2r2->p_type == p_g3r1 );
    }
}

TFEATURE( "Child linking - single grammar" )
{
    {
    TDOC( "One child, one layer of nesting" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 ).target_rule_name( "g1r3" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c1->target_rule.p_rule == p_g1r3 );
    TTEST( p_g1r1c1->p_rule == p_g1r1c1 );   // Points to self
    TTEST( p_g1r1c1->p_type == p_g1r3 );
    }
    {
    TDOC( "Three children, one layer of nesting" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c2 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c3 = RuleMaker( p_g1r1 ).target_rule_name( "g1r3" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c3->target_rule.p_rule == p_g1r3 );
    TTEST( p_g1r1c3->p_rule == p_g1r1c3 );   // Points to self
    TTEST( p_g1r1c3->p_type == p_g1r3 );
    }
    {
    TDOC( "One child, two layers of nesting" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c1c1 = RuleMaker( p_g1r1c1 ).target_rule_name( "g1r3" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c1c1->target_rule.p_rule == p_g1r3 );
    TTEST( p_g1r1c1c1->p_rule == p_g1r1c1c1 );   // Points to self
    TTEST( p_g1r1c1c1->p_type == p_g1r3 );
    }
    {
    TDOC( "Three children, one layer of nesting" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c2 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c3 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c3c1 = RuleMaker( p_g1r1c3 ).target_rule_name( "g1r3" );
            Rule * p_g1r1c3c2 = RuleMaker( p_g1r1c3 ).target_rule_name( "g1r4" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).target_rule_name( "g1r5" );
    Rule * p_g1r5 = RuleMaker( p_g1 ).rule_name( "g1r5" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c3c1->target_rule.p_rule == p_g1r3 );
    TTEST( p_g1r1c3c1->p_rule == p_g1r1c3c1 );   // Points to self
    TTEST( p_g1r1c3c1->p_type == p_g1r3 );
    TTEST( p_g1r1c3c2->target_rule.p_rule == p_g1r4 );
    TTEST( p_g1r1c3c2->p_rule == p_g1r1c3c2 );   // Points to self
    TTEST( p_g1r1c3c2->p_type == p_g1r5 );
    }
}

TFEATURE( "Child linking - single grammar - with member names" )
{
    {
    TDOC( "Three children, one layer of nesting" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c2 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c3 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c3c1 = RuleMaker( p_g1r1c3 ).target_rule_name( "g1r3" );
            Rule * p_g1r1c3c2 = RuleMaker( p_g1r1c3 ).target_rule_name( "g1r4" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).member_name( "mg1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).member_name( "mg1r4" ).target_rule_name( "g1r5" );
    Rule * p_g1r5 = RuleMaker( p_g1 ).rule_name( "g1r5" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c3c1->target_rule.p_rule == p_g1r3 );
    TTEST( p_g1r1c3c1->p_rule == p_g1r3 );
    TTEST( p_g1r1c3c1->p_type == p_g1r3 );
    TTEST( p_g1r1c3c2->target_rule.p_rule == p_g1r4 );
    TTEST( p_g1r1c3c2->p_rule == p_g1r4 );
    TTEST( p_g1r1c3c2->p_type == p_g1r5 );
    }
    {
    TDOC( "Must error if child is a member rule, and also links to a member rule" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c2 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c3 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c3c1 = RuleMaker( p_g1r1c3 ).member_name( "mg1r1c3" ).target_rule_name( "g1r3" );
            Rule * p_g1r1c3c2 = RuleMaker( p_g1r1c3 );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" ).member_name( "mg1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Must error if child links to rules that have multiple member names" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c2 = RuleMaker( p_g1r1 );
        Rule * p_g1r1c3 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c3c1 = RuleMaker( p_g1r1c3 );
            Rule * p_g1r1c3c2 = RuleMaker( p_g1r1c3 ).target_rule_name( "g1r4" );
    Rule * p_g1r2 = RuleMaker( p_g1 ).rule_name( "g1r2" );
    Rule * p_g1r3 = RuleMaker( p_g1 ).rule_name( "g1r3" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" ).member_name( "mg1r4" ).target_rule_name( "g1r5" );
    Rule * p_g1r5 = RuleMaker( p_g1 ).rule_name( "g1r5" ).member_name( "mg1r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Child linking - multiple grammars" )
{
    {
    TDOC( "Unaliased import" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs ).unaliased_import( "g2" );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c1c1 = RuleMaker( p_g1r1c1 ).target_rule_name( "g2r2" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" );
    Rule * p_g2r2 = RuleMaker( p_g2 ).rule_name( "g2r2" );
    Rule * p_g2r3 = RuleMaker( p_g2 ).rule_name( "g2r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c1c1->target_rule.p_rule == p_g2r2 );
    TTEST( p_g1r1c1c1->p_rule == p_g1r1c1c1 );   // Points to self
    TTEST( p_g1r1c1c1->p_type == p_g2r2 );
    }
    {
    TDOC( "Aliased import" );
    GrammarSet gs;

    Grammar * p_g1 = GrammarMaker( gs );
    Rule * p_g1r1 = RuleMaker( p_g1 ).rule_name( "g1r1" );
        Rule * p_g1r1c1 = RuleMaker( p_g1r1 );
            Rule * p_g1r1c1c1 = RuleMaker( p_g1r1c1 ).target_ruleset_id( "g2" ).target_rule_name( "g2r2" );
    Rule * p_g1r4 = RuleMaker( p_g1 ).rule_name( "g1r4" );

    Grammar * p_g2 = GrammarMaker( gs ).ruleset_id( "g2" );
    Rule * p_g2r1 = RuleMaker( p_g2 ).rule_name( "g2r1" );
    Rule * p_g2r2 = RuleMaker( p_g2 ).rule_name( "g2r2" );
    Rule * p_g2r3 = RuleMaker( p_g2 ).rule_name( "g2r3" );

    JCRParser jp( &gs );

    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1r1c1c1->target_rule.p_rule == p_g2r2 );
    TTEST( p_g1r1c1c1->p_rule == p_g1r1c1c1 );   // Points to self
    TTEST( p_g1r1c1c1->p_type == p_g2r2 );
    }
}
