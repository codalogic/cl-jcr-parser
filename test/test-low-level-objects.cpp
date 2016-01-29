//----------------------------------------------------------------------------
// Copyright (c) 2016, Codalogic Ltd (http://www.codalogic.com)
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

TFEATURE( "ValueConstraint" )
{
    ValueConstraint vc;

    TTEST( vc.is_set() == false );

    TSETUP( vc = "true" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "true" );
    TTEST( vc == std::string( "true" ) );
    TTEST( vc.to_string() == "true" );
    TTEST( vc.to_bool() == true );
    TTEST( vc.to_int() == 0 );      // This is undefined behaviour
    TTEST( vc.to_float() == 0 );    // This is undefined behaviour

    TSETUP( vc = "false" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "false" );
    TTEST( vc.to_bool() == false );
    TTEST( vc.to_int() == 0 );      // This is undefined behaviour
    TTEST( vc.to_float() == 0 );    // This is undefined behaviour

    TSETUP( vc = "foo" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "foo" );
    TTEST( vc.to_bool() == false ); // This is undefined behaviour - no attempt made to convert to 'sensible' bool value when non-bool value present
    TTEST( vc.to_int() == 0 );      // This is undefined behaviour
    TTEST( vc.to_float() == 0 );    // This is undefined behaviour

    TSETUP( vc = "10" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "10" );
    TTEST( vc.to_bool() == false ); // This is undefined behaviour - no attempt made to convert to 'sensible' bool value when non-bool value present
    TTEST( vc.to_int() == 10 );
    TTEST( vc.to_float() == 10.0 );

    TSETUP( vc = "10.5" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "10.5" );
    TTEST( vc.to_bool() == false ); // This is undefined behaviour - no attempt made to convert to 'sensible' bool value when non-bool value present
    TTEST( vc.to_int() == 10 );     // This is undefined behaviour
    TTEST( vc.to_float() == 10.5 );

    TSETUP( vc = "2.5e+3" );
    TTEST( vc.is_set() == true );
    TTEST( vc == "2.5e+3" );
    TTEST( vc.to_bool() == false ); // This is undefined behaviour - no attempt made to convert to 'sensible' bool value when non-bool value present
    TTEST( vc.to_int() == 2 );      // This is undefined behaviour
    TTEST( vc.to_float() == 2500.0 );

    TSETUP( vc.clear() );
    TTEST( vc.is_set() == false );
    TTEST( vc == "" );
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

    TSETUP( mn.set_absent() );
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
    Rule r;

    TTEST( ! r.p_parent );
    TTEST( r.type == Rule::NONE );

    TSETUP( Rule::uniq_ptr pu_rule( new Rule ) );

    TTEST( ! pu_rule->p_parent );
    TTEST( pu_rule->type == Rule::NONE );

    TSETUP( Rule * p_new_rule = pu_rule.get() );
    TSETUP( Rule * p_appended_rule = r.append_child_rule( pu_rule ) );
    TTEST( p_appended_rule == p_new_rule );
    TTEST( p_appended_rule->p_parent == &r );
}
