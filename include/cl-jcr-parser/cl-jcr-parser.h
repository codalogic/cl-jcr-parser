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
#include <string>
#include <cstdlib>

#if __cplusplus >= 201103L
    #include <cstdint>
#endif

namespace cl { class reader; }

namespace cljcr {

namespace detail {
class NonCopyable   // Avoid requiring inclusion of boost for this
{
private:
    NonCopyable( const NonCopyable & );
    NonCopyable & operator = ( const NonCopyable & );

protected:  // Intended to be inherited
    NonCopyable() {}
};
} // namespace detail

template<typename T>
struct uniq_ptr
{
#if __cplusplus < 201103L
    typedef std::auto_ptr<T> type;
#else
    typedef std::unique_ptr<T> type;
#endif
};

#if __cplusplus < 201103L
    typedef long long int64;
    typedef unsigned long long uint64;
#else
    typedef std::int64_t int64;
    typedef std::uint64_t uint64;
#endif

struct Repetition
{
    int min;
    int max;

    Repetition() : min( 1 ), max( 1 ) {}
};

struct Annotations
{
    bool is_not;
    bool is_unordered;
    bool is_root;

    Annotations() : is_not( false ), is_unordered( false ), is_root( false ) {}
    void clear() { is_not = is_unordered = is_root = false; }
    bool merge( const Annotations & r_rhs )
    {
        is_not = ( is_not || r_rhs.is_not );
        is_unordered = ( is_unordered || r_rhs.is_unordered );
        is_root = ( is_root || r_rhs.is_root );
        return true;
    }
};

class MemberName
{
private:
    enum Form { Absent, Literal, Regex };
    struct Members {
        std::string name;
        Form form;

        Members() : form( Absent ) {}
    } m;

public:
    void set_absent() { m.form = Absent; m.name.clear(); }
    void set_literal( const std::string & name ) { m.form = Literal; m.name = name; }
    void set_regex( const std::string & name ) { m.form = Regex; m.name = name; }

    bool is_absent() const { return m.form == Absent; }
    bool is_literal() const { return m.form == Literal; }
    bool is_regex() const { return m.form == Regex; }
    const std::string & name() const { return m.name; }
};

class ValueConstraint
{
private:
    struct Members {
        enum Form { unset, string_form, bool_form, int_form, uint_form, float_form } form;
        std::string string_value;
        bool bool_value;
        int64 int_value;
        uint64 uint_value;
        double float_value;

        Members() : form( unset ) {}
    } m;

public:
    void clear() { m.form = Members::unset; m.string_value.clear(); }
    ValueConstraint & operator = ( const std::string & r_constraint )
    {
        m.form = Members::string_form;
        m.string_value = r_constraint;
        return *this;
    }
    ValueConstraint & operator = ( const char * p_constraint )
    {
        m.form = Members::string_form;
        m.string_value = p_constraint;
        return *this;
    }
    ValueConstraint & operator = ( bool constraint )
    {
        m.form = Members::bool_form;
        m.bool_value = constraint;
        return *this;
    }
    ValueConstraint & operator = ( int64 constraint )
    {
        m.form = Members::int_form;
        m.int_value = constraint;
        return *this;
    }
    ValueConstraint & operator = ( uint64 constraint )
    {
        m.form = Members::uint_form;
        m.uint_value = constraint;
        return *this;
    }
    ValueConstraint & operator = ( double constraint )
    {
        m.form = Members::float_form;
        m.float_value = constraint;
        return *this;
    }
    bool is_set() const { return m.form != Members::unset; }
    bool is_string() const { return m.form == Members::string_form; }
    bool is_bool() const { return m.form == Members::bool_form; }
    bool is_int() const { return m.form == Members::int_form; }
    bool is_uint() const { return m.form == Members::uint_form; }
    bool is_float() const { return m.form == Members::float_form; }
    const std::string & as_string() const { assert( m.form == Members::string_form ); return m.string_value; }
    bool as_bool() const { assert( m.form == Members::bool_form ); return m.bool_value; }
    int64 as_int() const { assert( m.form == Members::int_form ); return m.int_value; }
    uint64 as_uint() const { assert( m.form == Members::uint_form ); return m.uint_value; }
    double as_float() const { assert( m.form == Members::float_form ); return m.float_value; }
};

struct Rule;

struct TargetRule
{
    std::string rulesetid;
    std::string local_name;
    Rule * p_rule;      // Filled in when 'compiled'

    TargetRule() : p_rule( 0 ) {}
};

struct Rule : private detail::NonCopyable
{
    typedef uniq_ptr<Rule>::type uniq_ptr;

    enum Type {
            NONE, TNULL, BOOLEAN, INTEGER, UINTEGER, DOUBLE, FLOAT,
            STRING_TYPE, STRING_REGEX, STRING_LITERAL,
            IPV4, IPV6, IPADDR, FQDN, IDN,
            URI_TYPE, URI_RANGE, EMAIL, PHONE,
            DATETIME, DATE, TIME,
            HEX, BASE32, BASE32HEX, BASE64, BASE64URL,
            ANY,
            TYPE_CHOICE, OBJECT, ARRAY, GROUP,
            TARGET_RULE };

    Rule * p_parent;
    std::string rule_name;
    Repetition repetition;
    Annotations annotations;
    MemberName member_name;
    Type type;
    ValueConstraint min;
    ValueConstraint max;
    typedef clutils::ptr_vector< Rule > children_container_t;
    children_container_t children;
    enum ChildCombiner { None, Sequence, Choice } child_combiner;
    TargetRule target_rule;

    Rule() : p_parent( 0 ), type( NONE ), child_combiner( None ) {}

    Rule * append_child_rule( uniq_ptr pu_child_rule )
    {
        pu_child_rule->p_parent = this;
        children.push_back( pu_child_rule.get() );
        return pu_child_rule.release();
    }
};

class AliasLookupResult
{
private:
    const std::string * p_s;

public:
    AliasLookupResult() : p_s( 0 ) {}
    AliasLookupResult( const std::string & r_s ) : p_s( &r_s ) {}
    bool is_found() const { return p_s != 0; }
    const std::string & value() const { return *p_s; }
    operator const std::string & () const { return value(); }
};

struct Grammar : private detail::NonCopyable
{
    typedef uniq_ptr<Grammar>::type uniq_ptr;
    typedef clutils::ptr_vector< Rule > rule_container_t;

    std::string ruleset_id;
    std::vector< std::string > unaliased_imports;
    typedef std::map< std::string, std::string > aliased_imports_t;   // Alias -> Ruleset_id
    aliased_imports_t aliased_imports;
    rule_container_t rules;

    void add_unaliased_import( const std::string & r_import )
    {
        unaliased_imports.push_back( r_import );
    }
    bool add_aliased_import( const std::string & r_alias, const std::string & r_import )
    {
        if( has_aliased_import( r_alias ) )
            return false;
        aliased_imports[r_alias] = r_import;
        return true;
    }
    bool has_aliased_import( const std::string & r_alias ) const
    {
        return aliased_imports.find( r_alias ) != aliased_imports.end();
    }
    AliasLookupResult get_aliased_import( const std::string & r_alias ) const
    {
        aliased_imports_t::const_iterator i_alias = aliased_imports.find( r_alias );
        if( i_alias != aliased_imports.end() )
            return AliasLookupResult( i_alias->second );
        return AliasLookupResult();
    }
    Rule * append_rule( Rule::uniq_ptr pu_rule )
    {
        pu_rule->p_parent = 0;
        rules.push_back( pu_rule.get() );
        return pu_rule.release();
    }
};

struct GrammarSet : private detail::NonCopyable
{
private:
    typedef clutils::ptr_vector< Grammar > container_t;
    struct Members {
        container_t grammars;
    } m;

public:
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

class JCRParser : private detail::NonCopyable
{
public:
    enum Status { S_OK, S_UNABLE_TO_OPEN_FILE, S_INTERNAL_ERROR };

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
