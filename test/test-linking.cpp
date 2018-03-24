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

class RuleMaker // To facilitate making rules for testing
{
    // Usage: Rule * p_rule = RuleMaker( p_grammar ).rule_name( "Foo" ).target_rule_name( "Bar" ).rule();

private:
    Rule * p_rule;

public:
    RuleMaker( Grammar * p_grammar ) : p_rule( p_grammar->append_rule( Rule::uniq_ptr( new Rule( p_grammar, 0, 0 ) ) ) ) {}
    RuleMaker( Rule * p_rule_in ) : p_rule( p_rule_in ) {}  // For an already created rule

    Rule * rule() { return p_rule; }

    RuleMaker & rule_name( const char * p_name ) { p_rule->rule_name = p_name; return *this; }
    RuleMaker & member_name( const char * p_name ) { p_rule->member_name.set_literal( p_name ); return *this; }
    RuleMaker & target_rule_name( const char * p_name ) { p_rule->target_rule.rule_name = p_name; return *this; }
    RuleMaker & target_ruleset_id( const char * p_name ) { p_rule->target_rule.ruleset_id = p_name; return *this; }
};

TFEATURE( "Linking Rule::find_target_rule()" )
{
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";

    Grammar * p_g2 = gs.append_grammar( "<local>" );
    p_g2->add_unaliased_import( "g3" );
    Rule * p_g2r1 = p_g2->append_rule( Rule::uniq_ptr( new Rule( p_g2, 0, 0 ) ) );
    p_g2r1->rule_name = "g2r1";
    Rule * p_g2r2 = p_g2->append_rule( Rule::uniq_ptr( new Rule( p_g2, 0, 0 ) ) );
    p_g2r2->rule_name = "g2r2";

    Grammar * p_g3 = gs.append_grammar( "<local>" );
    p_g3->ruleset_id = "g3";
    // p_g3->add_aliased_import( "ag4", "g4" ); // Not needed - Aliases are mapped at parse time, and target_rule stores resultant ruleset_id
    Rule * p_g3r1 = p_g3->append_rule( Rule::uniq_ptr( new Rule( p_g3, 0, 0 ) ) );
    p_g3r1->rule_name = "g3r1";
    Rule * p_g3r2 = p_g3->append_rule( Rule::uniq_ptr( new Rule( p_g3, 0, 0 ) ) );
    p_g3r2->rule_name = "g3r2";

    Grammar * p_g4 = gs.append_grammar( "<local>" );
    p_g4->ruleset_id = "g4";
    Rule * p_g4r1 = p_g4->append_rule( Rule::uniq_ptr( new Rule( p_g4, 0, 0 ) ) );
    p_g4r1->rule_name = "g4r1";
    Rule * p_g4r2 = p_g4->append_rule( Rule::uniq_ptr( new Rule( p_g4, 0, 0 ) ) );
    p_g4r2->rule_name = "g4r2";

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

GrammarSet::uniq_ptr create_grammar()
{
    GrammarSet::uniq_ptr pu_gs( new GrammarSet );

    Grammar * p_g1 = pu_gs->append_grammar( "<local 1>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";

    Grammar * p_g2 = pu_gs->append_grammar( "<local 2>" );
    p_g2->add_unaliased_import( "g3" );
    Rule * p_g2r1 = p_g2->append_rule( Rule::uniq_ptr( new Rule( p_g2, 0, 0 ) ) );
    p_g2r1->rule_name = "g2r1";
    Rule * p_g2r2 = p_g2->append_rule( Rule::uniq_ptr( new Rule( p_g2, 0, 0 ) ) );
    p_g2r2->rule_name = "g2r2";

    Grammar * p_g3 = pu_gs->append_grammar( "<local 3>" );
    p_g3->ruleset_id = "g3";
    // p_g3->add_aliased_import( "ag4", "g4" ); // Not needed - Aliases are mapped at parse time, and target_rule stores resultant ruleset_id
    Rule * p_g3r1 = p_g3->append_rule( Rule::uniq_ptr( new Rule( p_g3, 0, 0 ) ) );
    p_g3r1->rule_name = "g3r1";
    Rule * p_g3r2 = p_g3->append_rule( Rule::uniq_ptr( new Rule( p_g3, 0, 0 ) ) );
    p_g3r2->rule_name = "g3r2";

    Grammar * p_g4 = pu_gs->append_grammar( "<local 4>" );
    p_g4->ruleset_id = "g4";
    Rule * p_g4r1 = p_g4->append_rule( Rule::uniq_ptr( new Rule( p_g4, 0, 0 ) ) );
    p_g4r1->rule_name = "g4r1";
    Rule * p_g4r2 = p_g4->append_rule( Rule::uniq_ptr( new Rule( p_g4, 0, 0 ) ) );
    p_g4r2->rule_name = "g4r2";

    return pu_gs;
}

TFEATURE( "Global linking - Local ruleset" )
{
    {
    TDOC( "One layer of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r1 );   // Points to self
    TTEST( p_g1->rules[0].p_type == p_g1r2 );
    }
    {
    TDOC( "Two layers of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) == JCRParser::S_OK );
    TTEST( p_g1->rules[0].target_rule.p_rule == p_g1r2 );
    TTEST( p_g1->rules[0].p_rule == p_g1r1 );   // Points to self
    TTEST( p_g1->rules[0].p_type == p_g1r3 );
    }
    {
    TDOC( "Three layers of lookup" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->member_name.set_literal( "mg1r2" );

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->member_name.set_literal( "mg1r2" );
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->member_name.set_literal( "mg1r3" );

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    p_g1r2->member_name.set_literal( "mg1r2" );
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    p_g1r3->member_name.set_literal( "mg1r3" );
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->member_name.set_literal( "mg1r4" );

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

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    p_g1r1->member_name.set_literal( "mg1r1" );
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->member_name.set_literal( "mg1r4" );

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Three layers of lookup - member name on 2nd and 3rd rule" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    p_g1r2->member_name.set_literal( "mg1r2" );
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    p_g1r3->member_name.set_literal( "mg1r3" );
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Three layers of lookup - member name on 3rd and 4th rule" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    p_g1r3->member_name.set_literal( "mg1r3" );
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->member_name.set_literal( "mg1r4" );

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}

TFEATURE( "Global linking - Local ruleset - with illegal loops" )
{
    {
    TDOC( "One layer of lookup - loop to self" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r1";     // Link to self
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Two layers of lookup - loop to top" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r1";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Two layers of lookup - loopback to bottom" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r2";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to top" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->target_rule.rule_name = "g1r1";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to 2nd" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->target_rule.rule_name = "g1r2";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to 3rd" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->target_rule.rule_name = "g1r3";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
    {
    TDOC( "Four layers of lookup - loopback to bottom" );
    GrammarSet gs;

    Grammar * p_g1 = gs.append_grammar( "<local>" );
    Rule * p_g1r1 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r1->rule_name = "g1r1";
    p_g1r1->target_rule.rule_name = "g1r2";
    Rule * p_g1r2 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r2->rule_name = "g1r2";
    p_g1r2->target_rule.rule_name = "g1r3";
    Rule * p_g1r3 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r3->rule_name = "g1r3";
    p_g1r3->target_rule.rule_name = "g1r4";
    Rule * p_g1r4 = p_g1->append_rule( Rule::uniq_ptr( new Rule( p_g1, 0, 0 ) ) );
    p_g1r4->rule_name = "g1r4";
    p_g1r4->target_rule.rule_name = "g1r4";

    JCRParser jp( &gs );
    
    TCRITICALTEST( jp.link( p_g1 ) != JCRParser::S_OK );
    }
}
