//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#ifndef CL_JCR_PARSER
#define CL_JCR_PARSER

#include "cl-utils/ptr-vector.h"

#include <cassert>
#include <memory>

namespace cljcr {

template<typename T>
struct uniq_ptr
{
#if __cplusplus < 201103L
    typedef std::auto_ptr<T> type;
#else
    typedef std::unique_ptr<T> type;
#endif
};

class BadRuleOrDirectiveRequest : public std::exception {};
class BadDirectiveRequest : public BadRuleOrDirectiveRequest {};
class BadRuleRequest : public BadRuleOrDirectiveRequest {};

class Rule
{
public:
    typedef uniq_ptr<Rule>::type uniq_ptr;
};

class Directive
{
public:
    typedef uniq_ptr<Directive>::type uniq_ptr;
};

class RuleOrDirective
{
private:
    struct Members {
        Directive * p_directive;
        Rule * p_rule;
        Members() : p_directive( 0 ), p_rule( 0 ) {}
    } m;

public:
    typedef uniq_ptr<RuleOrDirective>::type uniq_ptr;

    static RuleOrDirective * make( Directive * p_directive ) { return new RuleOrDirective( Directive::uniq_ptr( p_directive ) ); }
    static RuleOrDirective * make( Rule * p_rule ) { return new RuleOrDirective( Rule::uniq_ptr( p_rule ) ); }
    static RuleOrDirective * make_directive() { return make( new Directive ); }
    static RuleOrDirective * make_rule() { return make( new Rule ); }

    RuleOrDirective( Directive::uniq_ptr pu_directive_in )
    {
        m.p_directive = pu_directive_in.release();
    }
    RuleOrDirective( Rule::uniq_ptr pu_rule_in )
    {
        m.p_rule = pu_rule_in.release();
    }
    ~RuleOrDirective()
    {
        delete m.p_directive;   // Deleting possibly null pointer is safe...
        delete m.p_rule;
    }

    virtual bool is_directive() const { return m.p_directive != 0; }
    virtual const Directive * directive() const
    {
        if( ! is_directive() )
            throw BadDirectiveRequest();
        return m.p_directive;
    }
    virtual Directive * directive()
    {
        if( ! is_directive() )
            throw BadDirectiveRequest();
        return m.p_directive;
    }
    virtual bool is_rule() const { return m.p_rule != 0; }
    virtual const Rule * rule() const
    {
        if( ! is_rule() )
            throw BadRuleRequest();
        return m.p_rule;
    }
    virtual Rule * rule()
    {
        if( ! is_rule() )
            throw BadRuleRequest();
        return m.p_rule;
    }
};

class Grammar
{
private:
    typedef clutils::ptr_vector< RuleOrDirective > container_t;
    struct Members {
        container_t rules_and_directives;
    } m;

public:
    typedef uniq_ptr<Grammar>::type uniq_ptr;

    ~Grammar() {}

    RuleOrDirective * append( RuleOrDirective * p_rule_or_directive )
    {
        RuleOrDirective::uniq_ptr pu_rule_or_directive( p_rule_or_directive );
        append( pu_rule_or_directive );
        return p_rule_or_directive;
    }
    void append( RuleOrDirective::uniq_ptr pu_rule_or_directive )
    {
        m.rules_and_directives.push_back( pu_rule_or_directive.get() );
        pu_rule_or_directive.release();
    }
    Directive * append( Directive * p_directive )
    {
        append( RuleOrDirective::make( p_directive ) );
        return p_directive;
    }
    Rule * append( Rule * p_rule )
    {
        append( RuleOrDirective::make( p_rule ) );
        return p_rule;
    }
    Directive * append_directive()
    {
        return append( RuleOrDirective::make_directive() )->directive();
    }
    Rule * append_rule()
    {
        return append( RuleOrDirective::make_rule() )->rule();
    }

    const RuleOrDirective & back() const
    {
        return m.rules_and_directives.back();
    }
    RuleOrDirective & back()
    {
        return m.rules_and_directives.back();
    }
    bool back_is_directive() const
    {
        return back().is_directive();
    }
    bool back_is_rule() const
    {
        return back().is_rule();
    }
    const Directive & back_directive() const
    {
        return *back().directive();
    }
    Directive & back_directive()
    {
        return *back().directive();
    }
    const Rule & back_rule() const
    {
        return *back().rule();
    }
    Rule & back_rule()
    {
        return *back().rule();
    }

    typedef container_t::const_iterator const_iterator;
    typedef container_t::iterator iterator;
    const_iterator begin() const { return m.rules_and_directives.begin(); }
    iterator begin() { return m.rules_and_directives.begin(); }
    const_iterator end() const { return m.rules_and_directives.end(); }
    iterator end() { return m.rules_and_directives.end(); }

    const RuleOrDirective & operator [] ( size_t i ) const { return m.rules_and_directives[i]; }
    RuleOrDirective & operator [] ( size_t i ) { return m.rules_and_directives[i]; }
};

class JCRParser
{
public:
    enum Status { S_OK, S_UNABLE_TO_OPEN_FILE };

private:
    struct Members {
        Grammar * p_grammar;
        const char * p_file_name;
        Status status;

        Members( Grammar * p_grammar_in, const char * p_file_name_in )
            :
            p_grammar( p_grammar_in ),
            p_file_name( p_file_name_in ),
            status( S_OK )
        {}
    } m;

public:
    JCRParser( Grammar * p_grammar, const char * p_file_name ) : m( p_grammar, p_file_name ) {}
    Status parse();
    virtual void error( size_t line, const char * p_message );  // Inherit this class to get error message fed back to you
};

}   // namespace cljcr

#endif  // CL_JCR_PARSER
