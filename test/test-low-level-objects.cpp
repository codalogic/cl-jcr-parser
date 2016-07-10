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

    TSETUP( vc = "string" );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "string" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

    TSETUP( vc = std::string( "string" ) );
    TTEST( vc.is_set() == true );
    TTEST( vc.is_string() == true );
    TTEST( vc.as_string() == "string" );
    TTEST( vc.as_string() != "other" );
    TTEST( vc.is_bool() == false );
    TTEST( vc.is_int() == false );
    TTEST( vc.is_uint() == false );
    TTEST( vc.is_float() == false );

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

TFEATURE( "Grammar" )
{
    Grammar g;

    TDOC( "Adding and accessing unaliased imports" );
    TSETUP( g.add_unaliased_import( "foo" ) );
    TTEST( g.unaliased_imports.size() == 1 );
    TSETUP( g.add_unaliased_import( "bar" ) );
    TTEST( g.unaliased_imports.size() == 2 );
    TTEST( g.unaliased_imports[0] == "foo" );
    TTEST( g.unaliased_imports[1] == "bar" );

    TDOC( "Adding aliased imports" );
    TTEST( g.has_aliased_import( "foo" ) == false );
    TTEST( g.add_aliased_import( "foo", "http://foo" ) == true );
    TTEST( g.has_aliased_import( "foo" ) == true );
    TTEST( g.has_aliased_import( "bar" ) == false );

    TTEST( g.add_aliased_import( "bar", "http://bar" ) == true );
    TTEST( g.has_aliased_import( "bar" ) == true );
    TTEST( g.aliased_imports["foo"] == "http://foo" );
    TTEST( g.aliased_imports["bar"] == "http://bar" );

    TDOC( "Adding duplicate aliased imports fails" );
    TTEST( g.add_aliased_import( "foo", "http://foo" ) == false );

    TDOC( "Accessing aliased imports for const instances" );
    TSETUP( const Grammar & r_g( g ) );
    TTEST( r_g.get_aliased_import( "foo" ).value() == "http://foo" );
    TSETUP( std::string foo = r_g.get_aliased_import( "foo" ) );        // Can use AliasLookupResult cast to string & to assign directly to string
    TTEST( foo == "http://foo" );
    TTEST( r_g.get_aliased_import( "bar" ).value() == "http://bar" );

    TTEST( r_g.has_aliased_import( "blah" ) == false );
    TTEST( r_g.get_aliased_import( "blah" ).is_found() == false );

    TDOC( "Adding rules" );
    TTEST( g.rules.size() == 0 );
    TSETUP( Rule::uniq_ptr pu_r( new Rule ) );
    TSETUP( pu_r->p_parent = pu_r.get() );  // Set p_parent to non-zero value so we can test it's set to 0 later
    TTEST( pu_r->p_parent != 0 );
    TSETUP( Rule * p_unmanaged_rule = pu_r.get() );
    TSETUP( Rule * p_r = g.append_rule( pu_r ) );
    TTEST( p_r == p_unmanaged_rule );   // Using append_rule() returns released pointer to rule
    TTEST( ! p_r->p_parent );           // Using append_rule() sets p-parent pointer to null
    TTEST( g.rules.size() == 1 );
}

TFEATURETODO( "Test low level GrammarSet class" );
