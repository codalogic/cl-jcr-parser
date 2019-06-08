//----------------------------------------------------------------------------
// Copyright (c) 2015-2018, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#ifndef CL_JCR_PARSER__PARSER
#define CL_JCR_PARSER__PARSER

#include "cl-utils/ptr-vector.h"

#include <cassert>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <iostream>

#if __cplusplus >= 201103L
    #include <cstdint>
#endif

namespace cl { class reader; }

namespace cljcr {

//----------------------------------------------------------------------------
//                          Utility classes
//----------------------------------------------------------------------------

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

class Severity
{
public:
    enum Type { WARNING, ERROR, FATAL };

private:
    Type value;

public:
    Severity() : value( WARNING ) {}
    Severity( const Severity & r_rhs ) : value( r_rhs.value ) {}
    Severity & operator = ( const Severity & r_rhs ) { value = r_rhs.value; return *this; }

    Severity( Severity::Type e ) : value( e ) {}
    Severity & operator = ( Severity::Type e ) { value = e; return *this; }

    bool operator == ( const Severity & r_rhs ) const { return value == r_rhs.value; }
    bool operator == ( Severity::Type e ) const { return value == e; }
    bool operator != ( const Severity & r_rhs ) const { return value != r_rhs.value; }
    bool operator != ( Severity::Type e ) const { return value != e; }

    Severity::Type to_enum() const { return value; }
    operator Severity::Type () const { return to_enum(); }

    const char * to_s() const
    {
        switch( value )
        {
            case WARNING: return "Warning";
            case ERROR: return "Error";
            case FATAL: return "Fatal error";
        }
        return "<Error: invalid enum recorded>";
    }
    operator const char * () { return to_s(); }
};

inline std::ostream & operator << ( std::ostream & r_os, const Severity & r_v ) { r_os << r_v.to_s(); return r_os; }

//----------------------------------------------------------------------------
//                   Classes representing JCR constructs
//----------------------------------------------------------------------------

struct Rule;

struct TargetRule
{
    std::string ruleset_id;
    std::string rule_name;
    Rule * p_rule;      // Filled in when 'compiled'

    TargetRule() : p_rule( 0 ) {}
    void clear() { ruleset_id.empty(); rule_name.empty(); }
};

struct Repetition
{
    int min;
    int max;
    int step;

    Repetition() : min( 1 ), max( 1 ), step( 1 ) {}
};

struct Annotations
{
    bool is_not;
    bool is_unordered;
    bool is_root;
    bool is_exclude_min;
    bool is_exclude_max;
    bool is_defaulted;
    std::string default_value;
    std::string format;
    bool is_choice;
    std::vector<TargetRule> augments;

    Annotations() { clear(); }
    void clear() { is_not = is_unordered = is_root = is_exclude_min = is_exclude_max = is_defaulted = false; }
    bool merge( const Annotations & r_rhs )
    {
        is_not = ( is_not || r_rhs.is_not );
        is_unordered = ( is_unordered || r_rhs.is_unordered );
        is_root = ( is_root || r_rhs.is_root );
        is_exclude_min = ( is_exclude_min || r_rhs.is_exclude_min );
        is_exclude_max = ( is_exclude_max || r_rhs.is_exclude_max );
        is_defaulted = ( is_defaulted || r_rhs.is_defaulted );
        if( r_rhs.is_defaulted )
            default_value = r_rhs.default_value;
        if( format.empty() && ! r_rhs.format.empty() )
            format = r_rhs.format;
        if( augments.empty() && ! r_rhs.augments.empty() )
            augments = r_rhs.augments;
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
    void clear() { m = Members(); }
    void set_absent() { m.form = Absent; m.name.clear(); }
    void set_literal( const std::string & name ) { m.form = Literal; m.name = name; }
    void set_regex( const std::string & name ) { m.form = Regex; m.name = name; }

    bool is_absent() const { return m.form == Absent; }
    bool is_literal() const { return m.form == Literal; }
    bool is_regex() const { return m.form == Regex; }
    const std::string & name() const { return m.name; } // For regex form, name() will include full pattern, e.g. /p\d+/i
    std::string pattern() const;
    std::string modifiers() const;
};

std::ostream & operator << ( std::ostream & r_os, const MemberName & r_mn );

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
    std::string as_pattern() const;
    std::string as_modifiers() const;
    bool as_bool() const { assert( m.form == Members::bool_form ); return m.bool_value; }
    int64 as_int() const { assert( m.form == Members::int_form ); return m.int_value; }
    uint64 as_uint() const { assert( m.form == Members::uint_form ); return m.uint_value; }
    double as_float() const { assert( m.form == Members::float_form ); return m.float_value; }
};

std::ostream & operator << ( std::ostream & r_os, const TargetRule & r_tr );

struct Grammar;

struct Rule : private detail::NonCopyable
{
    typedef uniq_ptr< Rule >::type uniq_ptr;

    enum Type {
            NONE, TNULL, BOOLEAN, INTEGER, UINTEGER, DOUBLE, FLOAT,
            STRING_TYPE, STRING_REGEX, STRING_LITERAL,
            IPV4, IPV6, IPADDR, FQDN, IDN,
            URI_TYPE, URI_RANGE, EMAIL, PHONE,
            DATETIME, DATE, TIME,
            HEX, BASE32, BASE32HEX, BASE64, BASE64URL,
            ANY,
            TYPE_CHOICE,
            OBJECT, OBJECT_GROUP, ARRAY, ARRAY_GROUP, GROUP, GROUP_GROUP,
            TARGET_RULE };

    Grammar * p_grammar;
    Rule * p_parent;
    int line_number;
    int column_number;
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
    Rule * p_rule;
    Rule * p_type;

    Rule( Grammar * p_grammar_in, int line_number_in, int column_number_in )
        :
        p_grammar( p_grammar_in ),
        p_parent( 0 ),
        line_number( line_number_in ),
        column_number( column_number_in ),
        type( NONE ),
        child_combiner( None )
    {
        p_rule = p_type = this;
    }

    Rule * append_child_rule( uniq_ptr pu_child_rule )
    {
        pu_child_rule->p_parent = this;
        children.push_back( pu_child_rule.get() );
        return pu_child_rule.release();
    }

    void merge_target_annotations()
    {
        // Call after p_rule and p_type have been linked
        if( p_rule != this )
            annotations.merge( p_rule->annotations );
        if( p_type != this && p_type != p_rule )
            annotations.merge( p_type->annotations );
    }

    // find_target_rule() only resolves a rule to its named target
    // (if present). Bear in mind, however, that rules can link to rules
    // and then onto other rules, so find_target_rule() isn't the end of the
    // story.
    const Rule * find_target_rule() const;
    Rule * find_target_rule()
    {
        target_rule.p_rule = const_cast< Rule * >( static_cast< const Rule & >(*this).find_target_rule() );
        return target_rule.p_rule;
    }

    // These method access the relevant Rule after the linking operation
    const char * get_rule_name() const { return ! rule_name.empty() ? rule_name.c_str() : "<annoymous>"; }
    const Repetition & get_repetition() const { return repetition; }
    const Annotations & get_annotations() const { return annotations; }
    const MemberName & get_member_name() const { return p_rule->member_name; }
    bool is_member_rule() const { return ! get_member_name().is_absent(); }
    bool is_type_rule() const { return get_member_name().is_absent(); }
    const Type & get_type() const { return p_type->type; }
    const ValueConstraint & get_min() const { return p_type->min; }
    const ValueConstraint & get_max() const { return p_type->max; }
    const children_container_t & get_children() const { return p_type->children; }
    const ChildCombiner & get_child_combiner() const { return p_type->child_combiner; }
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

struct GrammarSet;

struct Grammar : private detail::NonCopyable
{
    typedef uniq_ptr< Grammar >::type uniq_ptr;
    typedef clutils::ptr_vector< Rule > rule_container_t;

    GrammarSet * p_grammar_set;
    std::string jcr_source;
    std::string ruleset_id;
    std::vector< std::string > unaliased_imports;
    typedef std::string ruleset_id_alias_t;
    typedef std::string ruleset_id_t;
    typedef std::map< ruleset_id_alias_t, ruleset_id_t > aliased_imports_t;   // Alias -> Ruleset_id
    aliased_imports_t aliased_imports;
    rule_container_t rules;

    Grammar( GrammarSet * p_grammar_set_in, std::string jcr_source_in )
        : p_grammar_set( p_grammar_set_in ), jcr_source( jcr_source_in )
    {}

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
    const Rule * find_rule( const std::string & r_sought_rule_name ) const
    {
        for( size_t i=0; i<rules.size(); ++i )
            if( rules[i].rule_name == r_sought_rule_name )
                return & rules[i];
        return 0;
    }
    Rule * find_rule( const std::string & r_sought_rule_name )
    {
        return const_cast< Rule * >( static_cast< const Grammar & >(*this).find_rule( r_sought_rule_name ) );
    }
};

struct GrammarSet : private detail::NonCopyable
{
public:
    typedef uniq_ptr< GrammarSet >::type uniq_ptr;

private:
    typedef clutils::ptr_vector< Grammar > container_t;
    struct Members {
        container_t grammars;
        size_t error_count;
        size_t warning_count;
        Members() : error_count( 0 ), warning_count( 0 ) {}
    } m;

public:
    Grammar * append( Grammar::uniq_ptr pu_grammar )
    {
        m.grammars.push_back( pu_grammar.get() );
        return pu_grammar.release();
    }
    Grammar * append_grammar( const std::string & jcr_source )
    {
        return append( Grammar::uniq_ptr( new Grammar( this, jcr_source ) ) );
    }

    void inc_error_count() { ++m.error_count; }
    void inc_warning_count() { ++m.warning_count; }
    size_t error_count() const { return m.error_count; }
    size_t warning_count() const { return m.warning_count; }

    typedef container_t::const_iterator const_iterator;
    typedef container_t::iterator iterator;
    const_iterator begin() const { return m.grammars.begin(); }
    iterator begin() { return m.grammars.begin(); }
    const_iterator end() const { return m.grammars.end(); }
    iterator end() { return m.grammars.end(); }

    size_t size() const { return m.grammars.size(); }
    const Grammar & operator [] ( size_t i ) const { return m.grammars[i]; }
    Grammar & operator [] ( size_t i ) { return m.grammars[i]; }

    const Grammar * find_grammar( const std::string & r_sought_ruleset_id ) const
    {
        for( size_t i=0; i<m.grammars.size(); ++i )
            if( ! m.grammars[i].ruleset_id.empty() &&      // Can't find an unnamed grammar
                    m.grammars[i].ruleset_id == r_sought_ruleset_id )
                return & m.grammars[i];
        return 0;
    }
    Grammar * find_grammar( const std::string & r_sought_ruleset_id )
    {
        return const_cast< Grammar * >( static_cast< const GrammarSet & >(*this).find_grammar( r_sought_ruleset_id ) );
    }
};

class JCRParser : private detail::NonCopyable
{
public:
    enum Status { S_OK, S_UNABLE_TO_OPEN_FILE, S_ERROR, S_INTERNAL_ERROR };

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
    GrammarSet * grammar_set() const { return m.p_grammar_set; }
    Status add_grammar( const char * p_file_name );
    Status add_grammar( const std::string & rules );
    Status add_grammar( const char * p_rules, size_t size );
    Status link();
    Status link( Grammar * p_grammar );

    virtual void report( const std::string & source, size_t line, size_t column, Severity severity, const char * p_message )  // Inherit this class to get error message fed back to you
    {
        (void)source; (void)line; (void)column; (void)severity; (void)p_message; // Mark parameters as unused
    }
    void report( const std::string & source, size_t line, Severity severity, const char * p_message )  // Inherit this class to get error message fed back to you
    {
        report( source, line, ~0U, severity, p_message );
    }
    void report( const std::string & source, Severity severity, const char * p_message )  // Inherit this class to get error message fed back to you
    {
        report( source, ~0U, ~0U, severity, p_message );
    }

private:
    Status parse_grammar( cl::reader & reader, const std::string & jcr_source );
};

class JCRParserWithReporter : public JCRParser
{
private:
    std::set< std::string > reported_messages;

public:
    JCRParserWithReporter( GrammarSet * p_grammar_set ) : JCRParser( p_grammar_set ) {}
    virtual void report( const std::string & source, size_t line, size_t column, Severity severity, const char * p_message );
};

}   // namespace cljcr

#endif  // CL_JCR_PARSER__PARSER
