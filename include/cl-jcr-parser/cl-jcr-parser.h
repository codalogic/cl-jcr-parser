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
#include <map>

namespace cl { class reader; }

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

class BadSelectorRequest : public std::exception {};

class SimpleType;
class UnionType;
class EnumType;

class BadValueTypeRequest : public BadSelectorRequest {};
class BadSimpleTypeRequest : public BadValueTypeRequest {};
class BadUnionTypeRequest : public BadValueTypeRequest {};
class BadEnumTypeRequest : public BadValueTypeRequest {};

class ValueType
{
public:
    virtual bool is_simple_type() const { return false; }
    virtual const SimpleType & simple_type() const { assert(0); throw BadSimpleTypeRequest(); }
    virtual SimpleType & simple_type() { assert(0); throw BadSimpleTypeRequest(); }

    virtual bool is_union_type() const { return false; }
    virtual const UnionType & union_type() const { assert(0); throw BadUnionTypeRequest(); }
    virtual UnionType & union_type() { assert(0); throw BadUnionTypeRequest(); }

    virtual bool is_enum_type() const { return false; }
    virtual const EnumType & enum_type() const { assert(0); throw BadEnumTypeRequest(); }
    virtual EnumType & enum_type() { assert(0); throw BadEnumTypeRequest(); }
};

class SimpleType : public ValueType
{
public:
    enum Type {
            NONE, TNULL, BOOLEAN, INTEGER, FLOAT, STRING, URI, IP4, IP6,
            FQDN, IDN, DATETIME, DATE, TIME, EMAIL, PHONE, BASE64, ANY };

private:
    struct Members {
        Type type;
        std::map< std::string, std::string > parameters;

        Members() : type( NONE ) {}
    } m;

public:
    virtual bool is_simple_type() const { return true; }
    virtual const SimpleType & simple_type() const { return *this; }
    virtual SimpleType & simple_type() { return *this; }

    void type( Type t ) { m.type = t; }
    Type type() const { return m.type; }

    // TODO: Add parameter accessors
};

class UnionType : public ValueType
{
private:
    typedef clutils::ptr_vector< ValueType > value_type_container_t;
    struct Members {
        value_type_container_t values;
    } m;

    virtual bool is_union_type() const { return true; }
    virtual const UnionType & union_type() const { return *this; }
    virtual UnionType & union_type() { return *this; }
};

class EnumType : public ValueType
{
private:
    struct Members {
    } m;

    virtual bool is_enum_type() const { return true; }
    virtual const EnumType & enum_type() const { return *this; }
    virtual EnumType & enum_type() { return *this; }
};

class BadSimpleTypeAdapter : public BadSelectorRequest {};

class IntegerSimpleTypeAdapter
{
private:
    struct Members {
        const SimpleType & simple_type;

        Members( const SimpleType & simple_type_in ) : simple_type( simple_type_in ) {}
    } m;

public:
    IntegerSimpleTypeAdapter( const SimpleType & simple_type ) : m( simple_type )
    {
        if( m.simple_type.type() != SimpleType::INTEGER )
            throw BadSimpleTypeAdapter();
    }

    bool has_min() const { assert(0); return false; }
    int min() const { assert(0); return 0; }
    bool has_max() const { assert(0); return false; }
    int max() const { assert(0); return 0; }
};

class FloatSimpleTypeAdapter
{
private:
    struct Members {
        const SimpleType & simple_type;

        Members( const SimpleType & simple_type_in ) : simple_type( simple_type_in ) {}
    } m;

public:
    FloatSimpleTypeAdapter( const SimpleType & simple_type ) : m( simple_type )
    {
        if( m.simple_type.type() != SimpleType::FLOAT )
            throw BadSimpleTypeAdapter();
    }

    bool has_min() const { assert(0); return false; }
    double min() const { assert(0); return 0; }
    bool has_max() const { assert(0); return false; }
    double max() const { assert(0); return 0; }
};

class StringSimpleTypeAdapter
{
private:
    struct Members {
        const SimpleType & simple_type;

        Members( const SimpleType & simple_type_in ) : simple_type( simple_type_in ) {}
    } m;

public:
    StringSimpleTypeAdapter( const SimpleType & simple_type ) : m( simple_type )
    {
        if( m.simple_type.type() != SimpleType::STRING )
            throw BadSimpleTypeAdapter();
    }

    bool has_pattern() const { assert(0); return false; }
    const std::string & pattern() const { assert(0); static std::string empty; return empty; }
};

class RefRule;
class ValueRule;
class ObjectRule;
class ArrayRule;

class BadRuleSelectorRequest : public BadSelectorRequest {};
class BadRefRuleRequest : public BadRuleSelectorRequest {};
class BadValueRuleRequest : public BadRuleSelectorRequest {};
class BadObjectRequest : public BadRuleSelectorRequest {};
class BadArrayRequest : public BadRuleSelectorRequest {};

class Rule
{
private:
    struct Members {
        std::string rule_name;
        std::string member_name;
    } m;

public:
    typedef uniq_ptr<Rule>::type uniq_ptr;

    void rule_name( const std::string & name ) { m.rule_name = name; }
    const std::string & rule_name() const { return m.rule_name; }
    void member_name( const std::string & name ) { m.member_name = name; }
    const std::string & member_name() const { return m.member_name; }

    virtual bool is_ref_rule() const { return false; }
    virtual const RefRule & ref_rule() const { assert(0); throw BadRefRuleRequest(); }
    virtual RefRule & ref_rule() { assert(0); throw BadRefRuleRequest(); }

    virtual bool is_value_rule() const { return false; }
    virtual const ValueRule & value_rule() const { assert(0); throw BadValueRuleRequest(); }
    virtual ValueRule & value_rule() { assert(0); throw BadValueRuleRequest(); }

    virtual bool is_object_rule() const { return false; }
    virtual const ObjectRule & object_rule() const { assert(0); throw BadObjectRequest(); }
    virtual ObjectRule & object_rule() { assert(0); throw BadObjectRequest(); }

    virtual bool is_array_rule() const { return false; }
    virtual const ArrayRule & array_rule() const { assert(0); throw BadArrayRequest(); }
    virtual ArrayRule & array_rule() { assert(0); throw BadArrayRequest(); }
};

class RefRule : public Rule
{
private:
    struct Members {
        std::string module;
        std::string local;
    } m;

public:
    virtual bool is_ref_rule() const { return true; }
    virtual const RefRule & ref_rule() const { return *this; }
    virtual RefRule & ref_rule() { return *this; }

    void ref( const std::string & ref ) { m.local = ref; }

    const std::string & module() const { return m.module; }
    const std::string & local() const { return m.local; }
};

class ValueRule : public Rule
{
private:
    struct Members {
        ValueType * p_value_type;

        Members() : p_value_type( 0 ) {}
    } m;

public:
    virtual bool is_value_rule() const { return true; }
    virtual const ValueRule & value_rule() const { return *this; }
    virtual ValueRule & value_rule() { return *this; }

    ValueRule() {}
    ~ValueRule() { delete m.p_value_type; }

    void value_type( ValueType * p_value_type ) { m.p_value_type = p_value_type; }
    const ValueType * value_type() const { return m.p_value_type; }
    ValueType * value_type() { return m.p_value_type; }
};

class ObjectRule : public Rule
{
private:
    typedef clutils::ptr_vector< Rule > rule_container_t;
    struct Members {
        rule_container_t children;
    } m;

    virtual bool is_object_rule() const { return true; }
    virtual const ObjectRule & object_rule() const { return *this; }
    virtual ObjectRule & object_rule() { return *this; }
};

class ArrayRule : public Rule
{
private:
    typedef clutils::ptr_vector< Rule > rule_container_t;
    struct Members {
        rule_container_t children;
    } m;

    virtual bool is_array_rule() const { return true; }
    virtual const ArrayRule & array_rule() const { return *this; }
    virtual ArrayRule & array_rule() { return *this; }
};

class Directive
{
private:
    typedef clutils::ptr_vector< std::string > container_t;
    struct Members {
        std::string directive;
        container_t parts;
    } m;

public:
    typedef uniq_ptr<Directive>::type uniq_ptr;

    void set( const std::string & r_directive );
    const std::string & get() const { return m.directive; }
    size_t size() const { return m.parts.size(); }
    const std::string & get( size_t i ) const  { return m.parts[i]; }
    const std::string & operator [] ( size_t i ) const  { return get( i ); }
};

class BadRuleOrDirectiveRequest : public BadSelectorRequest {};
class BadDirectiveRequest : public BadRuleOrDirectiveRequest {};
class BadRuleRequest : public BadRuleOrDirectiveRequest {};

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
    virtual const Directive & directive() const
    {
        if( ! is_directive() )
            throw BadDirectiveRequest();
        return *m.p_directive;
    }
    virtual Directive & directive()
    {
        if( ! is_directive() )
            throw BadDirectiveRequest();
        return *m.p_directive;
    }
    virtual bool is_rule() const { return m.p_rule != 0; }
    virtual const Rule & rule() const
    {
        if( ! is_rule() )
            throw BadRuleRequest();
        return *m.p_rule;
    }
    virtual Rule & rule()
    {
        if( ! is_rule() )
            throw BadRuleRequest();
        return *m.p_rule;
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

    RuleOrDirective & append( RuleOrDirective * p_rule_or_directive )
    {
        RuleOrDirective::uniq_ptr pu_rule_or_directive( p_rule_or_directive );
        append( pu_rule_or_directive );
        return *p_rule_or_directive;
    }
    void append( RuleOrDirective::uniq_ptr pu_rule_or_directive )
    {
        m.rules_and_directives.push_back( pu_rule_or_directive.get() );
        pu_rule_or_directive.release();
    }
    Directive & append( Directive * p_directive )
    {
        append( RuleOrDirective::make( p_directive ) );
        return *p_directive;
    }
    Rule & append( Rule * p_rule )
    {
        append( RuleOrDirective::make( p_rule ) );
        return *p_rule;
    }
    Directive & append_directive()
    {
        return append( RuleOrDirective::make_directive() ).directive();
    }
    Rule & append_rule()
    {
        return append( RuleOrDirective::make_rule() ).rule();
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
        return back().directive();
    }
    Directive & back_directive()
    {
        return back().directive();
    }
    const Rule & back_rule() const
    {
        return back().rule();
    }
    Rule & back_rule()
    {
        return back().rule();
    }

    typedef container_t::const_iterator const_iterator;
    typedef container_t::iterator iterator;
    const_iterator begin() const { return m.rules_and_directives.begin(); }
    iterator begin() { return m.rules_and_directives.begin(); }
    const_iterator end() const { return m.rules_and_directives.end(); }
    iterator end() { return m.rules_and_directives.end(); }

    size_t size() const { return m.rules_and_directives.size(); }
    const RuleOrDirective & operator [] ( size_t i ) const { return m.rules_and_directives[i]; }
    RuleOrDirective & operator [] ( size_t i ) { return m.rules_and_directives[i]; }
};

class GrammarSet
{
private:
    typedef clutils::ptr_vector< Grammar > container_t;
    struct Members {
        container_t grammars;
    } m;

public:
    GrammarSet() {}
    Grammar & append( Grammar * p_grammar )
    {
        Grammar::uniq_ptr pu_grammar( p_grammar );
        append( pu_grammar );
        return *p_grammar;
    }
    void append( Grammar::uniq_ptr pu_grammar )
    {
        m.grammars.push_back( pu_grammar.get() );
        pu_grammar.release();
    }
    Grammar & append_grammar()
    {
        append( new Grammar() );
        return m.grammars.back();
    }

    typedef container_t::const_iterator const_iterator;
    typedef container_t::iterator iterator;
    const_iterator begin() const { return m.grammars.begin(); }
    iterator begin() { return m.grammars.begin(); }
    const_iterator end() const { return m.grammars.end(); }
    iterator end() { return m.grammars.end(); }

    size_t size() const { return m.grammars.size(); }
    const Grammar & operator [] ( size_t i ) const { return m.grammars[i]; }
    Grammar & operator [] ( size_t i ) { return m.grammars[i]; }
};

class JCRParser
{
public:
    enum Status { S_OK, S_UNABLE_TO_OPEN_FILE, S_EXPECTED_END_OF_RULES };

private:
    struct Members {
        GrammarSet * p_grammar_set;

        Members( GrammarSet * p_grammar_set_in )
            :
            p_grammar_set( p_grammar_set_in )
        {}
    } m;

public:
    JCRParser( GrammarSet * p_grammar_set ) : m( p_grammar_set ) {}
    Status add_grammar( const char * p_file_name );
    Status add_grammar( const std::string & rules );
    Status add_grammar( const char * p_rules, size_t size );
    Status link();

    virtual void error( size_t line, size_t column, Status code, const char * p_message ) {}  // Inherit this class to get error message fed back to you

private:
    Status parse_grammar( cl::reader & reader );
};

}   // namespace cljcr

#endif  // CL_JCR_PARSER
