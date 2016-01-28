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

struct Repetition
{
    int min;
    int max;

    Repetition() : min( 1 ), max( 1 ) {}
};

struct Annotations
{
    bool reject;
    bool is_unordered;
    bool is_root;

    Annotations() : reject( false ), is_unordered( false ), is_root( false ) {}
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
    void set_absent( const std::string & name ) { m.form = Absent; m.name.clear(); }
    void set_literal( const std::string & name ) { m.form = Literal; m.name = name; }
    void set_regex( const std::string & name ) { m.form = Regex; m.name = name; }

    bool is_absent() { return m.form == Absent; }
    bool is_literal() { return m.form == Literal; }
    bool is_regex() { return m.form == Regex; }
    const std::string & name() const { return m.name; }
};

typedef std::string ConvertibleString; // DEBUG TODO

struct Rule : private detail::NonCopyable
{
    typedef uniq_ptr<Rule>::type uniq_ptr;

    enum Type {
            NONE, TNULL, BOOLEAN, INTEGER, FLOAT,
            STRING_TYPE, STRING_REGEX, STRING_LITERAL,
            URI_TYPE, URI_RANGE, IP4, IP6, FQDN, IDN,
            DATETIME, DATE, TIME,
            EMAIL, PHONE, BASE64, ANY,
            VALUE_CHOICE, OBJECT, ARRAY, GROUP };

    Rule * p_parent;
    std::string rule_name;
    Repetition repetition;
    Annotations annotations;
    MemberName member_name;
    Type type;
    bool has_min_value;
    ConvertibleString min_value;
    bool has_max_value;
    ConvertibleString max_value;
    typedef clutils::ptr_vector< Rule > children_container_t;
    children_container_t children;

    Rule() : p_parent( 0 ), type( NONE ), has_min_value( false ), has_max_value( false ) {}
};

struct Grammar : private detail::NonCopyable
{
    typedef uniq_ptr<Grammar>::type uniq_ptr;
    typedef clutils::ptr_vector< Rule > rule_container_t;

    std::string ruleset_id;
    std::vector< std::string > unaliased_imports;
    std::map< std::string, std::string > aliased_imports;   // Alias -> Ruleset_id
    rule_container_t rules;
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
