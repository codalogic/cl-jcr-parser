//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#include "cl-jcr-parser/cl-jcr-parser.h"

#include "dsl-pa/dsl-pa.h"

#if defined(_MSC_VER)
// Require error when nonstandard extension used :
//      'token' : conversion from 'type' to 'type'
//      e.g. disallow foo( /*const*/ foo & ) for copy constructor
//      and           foo( /*const*/ bar() ) for bar object constructed in-situ
#pragma warning(error: 4239)
#endif

#include <iostream>

namespace cljcr {

//----------------------------------------------------------------------------
//                           Internal detail
//----------------------------------------------------------------------------

namespace { // Anonymous namespace for detail

//----------------------------------------------------------------------------
//                           class QStringParser
//----------------------------------------------------------------------------

class QStringParser
{
    //  ; The defintion of a JSON string, from RFC 4627 s 2
    //  q-string        = %x20-21 / %x23-5B / %x5D-10FFFF / "\" (
    //                     %x22 /      ; "  u+0022
    //                     %x5C /      ; \  u+005C
    //                     %x2F /      ; /  u+002F
    //                     %x62 /      ; BS u+0008
    //                     %x66 /      ; FF u+000C
    //                     %x6E /      ; LF u+000A
    //                     %x72 /      ; CR u+000D
    //                     %x74 /      ; HT u+0009
    //                     ( %x75 4HEXDIG ) ) ; uXXXX u+XXXX

private:
    struct Members
    {
        cl::dsl_pa * p_dsl_pa;
        std::string * p_v;

        Members( cl::dsl_pa * p_dsl_pa_in, std::string * p_v_in )
            : p_dsl_pa( p_dsl_pa_in ), p_v( p_v_in )
        {}
    } m;

    QStringParser( cl::dsl_pa * p_dsl_pa, std::string * p_v )
        : m( p_dsl_pa, p_v )
    {}

    bool read()
    {
        if( ! m.p_dsl_pa->is_get_char( '"' ) )
            return false;

        return read_post_quote();
    }

    bool read_post_quote()
    {
        int c;
        while( (c = m.p_dsl_pa->get()) != '"' )
        {
            if( m.p_dsl_pa->is_current_at_end() )
                return false;
            *m.p_v += c;
        }
        return true;
    }

public:
    static bool read( cl::dsl_pa * p_dsl_pa, std::string * p_v )
    {
        return QStringParser( p_dsl_pa, p_v ).read();
    }

    static bool read_post_quote( cl::dsl_pa * p_dsl_pa, std::string * p_v )
    {
        return QStringParser( p_dsl_pa, p_v ).read_post_quote();
    }
};

//----------------------------------------------------------------------------
//                       class GrammarParserFatalError
//----------------------------------------------------------------------------

class GrammarParserFatalError : public std::exception {};

//----------------------------------------------------------------------------
//                           class GrammarParser
//----------------------------------------------------------------------------

class GrammarParser : public cl::dsl_pa
{
private:
    struct Members {
        JCRParser * p_parent;
        GrammarSet * p_grammar_set;
        Grammar * p_grammar;
        cl::reader & r_reader;
        bool is_errored;
        JCRParser::Status status;

        Rule * p_rule;

        Members(
            JCRParser * p_parent_in,
            cl::reader & r_reader_in,
            Grammar * p_grammar_in )
            :
            p_parent( p_parent_in ),
            p_grammar( p_grammar_in ),
            r_reader( r_reader_in ),
            is_errored( false ),
            status( JCRParser::S_OK ),
            p_rule( 0 )
        {}
    } m;

public:
    GrammarParser( JCRParser * p_parent, cl::reader & r_reader, Grammar * p_grammar );
    bool parse();
    JCRParser::Status status() const { return m.status; }

private:
    #define STAR( what ) bool star_ ## what() { while( what() ) {} return true; }
    #define ONE_STAR( what ) bool one_star_ ## what() { size_t n=0; while( what() ) ++n; return n > 0; }

    bool jcr();
    bool star_comment_or_directive();
    bool star_comment_or_directive_or_rule();
    bool sp_cmt();
    STAR( sp_cmt )
    bool spaces();
    bool comment();
    bool comment_char();
    bool comment_end_char();
    bool directive();
    bool directive_def();
    bool jcr_version_d();
    bool major_version();
    bool minor_version();
    bool ruleset_id_d();
    bool import_d();
    bool ruleset_id();
    bool not_space();
    STAR( not_space )
    bool ruleset_id_alias();
    bool tbd_directive_d();
    bool directive_name();
    bool directive_parameters();
    bool not_eol();
    STAR( not_eol )
    bool eol();
    bool root_rule();
    bool rule();
    bool rule_name();
    bool target_rule_name();
    bool name();
    bool rule_def();
    bool type_rule();
    bool value_rule();
    bool member_rule();
    bool member_name_spec();
    bool type_choice_rule();
    bool type_choice();
    bool type_choice_items();
    bool annotations( Annotations & anno );
    bool annotation_set( Annotations & anno );
    bool reject_annotation();
    bool unordered_annotation();
    bool root_annotation();
    bool tbd_annotation();
    bool annotation_name();
    bool annotation_parameters();
    bool primitive_rule();
    bool primimitive_def();
    bool null_type();
    bool boolean_type();
    bool true_value();
    bool false_value();
    bool string_type();
    bool string_value();
    bool string_range();
    bool float_type();
    bool float_range();
    bool float_min();
    bool float_max();
    bool float_value();
    bool integer_type();
    bool integer_range();
    bool integer_min();
    bool integer_max();
    bool integer_value();
    bool ip4_type();
    bool ip6_type();
    bool fqdn_type();
    bool idn_type();
    bool uri_range();
    bool uri_type();
    bool phone_type();
    bool email_type();
    bool full_date_type();
    bool full_time_type();
    bool date_time_type();
    bool base64_type();
    bool any();
    bool object_rule();
    bool object_items();
    bool object_item();
    bool object_item_types();
    bool object_group();
    bool array_rule();
    bool array_items();
    bool array_item();
    bool array_item_types();
    bool array_group();
    bool group_rule();
    bool group_items();
    bool group_item();
    bool group_item_types();
    bool group_group();
    bool sequence_combiner();
    bool choice_combiner();
    bool repetition( Repetition & reps );
    bool optional_marker();
    bool one_or_more();
    bool zero_or_more();
    bool min_max_repetition();
    bool min_repetition();
    bool max_repetition();
    bool min_repeat();
    bool max_repeat();
    bool specific_repetition();
    bool integer();
    bool p_integer();
    bool float_num();
    bool minus();
    bool plus();
    bool int_num();
    bool digit1_9();
    bool frac();
    bool decimal_point();
    bool exp();
    bool e();
    bool zero();
    bool q_string();
    bool regex();
    bool not_slash();
    bool regex_modifiers();
    bool uri_template();
    bool any_kw();
    bool as_kw();
    bool base64_kw();
    bool boolean_kw();
    bool date_time_kw();
    bool email_kw();
    bool false_kw();
    bool float_kw();
    bool fqdn_kw();
    bool full_date_kw();
    bool full_time_kw();
    bool idn_kw();
    bool import_kw();
    bool integer_kw();
    bool ip4_kw();
    bool ip6_kw();
    bool jcr_version_kw();
    bool null_kw();
    bool phone_kw();
    bool reject_kw();
    bool root_kw();
    bool ruleset_id_kw();
    bool string_kw();
    bool true_kw();
    bool unordered_kw();
    bool uri_dotdot_kw();
    bool uri_kw();
    bool ALPHA();
    bool CR();
    bool DIGIT();
    STAR( DIGIT )
    ONE_STAR( DIGIT )
    bool HEXDIG();
    bool HTAB();
    bool LF();
    bool SP();
    bool WSP();
    STAR( WSP )
    ONE_STAR( WSP )
    bool WSPs();

    // This is a temporary place holder
    bool warning( const char * p_message )
    {
        std::cout << p_message << "\n";
        return true;
    }
    bool error( const char * p_message )
    {
        std::cout << p_message << "\n";
        m.is_errored = true;
        return false;
    }
    bool fatal( const char * p_message )
    {
        std::cout << p_message << "\n";
        m.is_errored = true;
        throw GrammarParserFatalError();
        return false;
    }
    bool recover_to_eol()
    {
        while( is_get_char_in( cl::alphabet_not( cl::alphabet_eol() ) ) )
        {}
        get();  // Get the end of line character
        return true;
    }
};

GrammarParser::GrammarParser(
                        JCRParser * p_parent,
                        cl::reader & r_reader,
                        Grammar * p_grammar )
    :
    cl::dsl_pa( r_reader ),
    m( p_parent, r_reader, p_grammar )
{}

class UnspecifiedRetreat : public std::exception {};
class SurrenderRetreat : public UnspecifiedRetreat {};
class TopLevelRetreat : public UnspecifiedRetreat {};

bool GrammarParser::parse()
{
    if( ! jcr() || m.is_errored )
        m.status = JCRParser::S_INTERNAL_ERROR;
    return m.status == JCRParser::S_OK;
}

bool GrammarParser::jcr()
{
    // jcr() = *( sp_cmt() || directive() ) && [ root_rule() ] && *( sp_cmt() || directive() || rule() )

    try
    {
        return star_comment_or_directive() && optional( root_rule() ) && star_comment_or_directive_or_rule() && is_peek_at_end();
    }
    catch( const GrammarParserFatalError & )
    {
        return false;
    }
}

bool GrammarParser::star_comment_or_directive()
{
    // *( sp_cmt() || directive() )

    while( sp_cmt() || directive() )
    {}

    return true;
}

bool GrammarParser::star_comment_or_directive_or_rule()
{
    // *( sp_cmt() || directive() || rule() )

    while( sp_cmt() || directive() || rule() )
    {}

    return true;
}

bool GrammarParser::sp_cmt()
{
    // sp_cmt() = spaces() || comment()

    return spaces() || comment();
}

bool GrammarParser::spaces()
{
    // spaces() = 1*( WSP() || CR() || LF() )

    int i = 0;
    while( WSP() || CR() || LF() )
        ++i;
    return i > 0;
}

bool GrammarParser::comment()
{
    // comment() = ";" && *( "\;" || comment_char() ) && comment_end_char()

    if( is_get_char( ';' ) )
    {
        while( is_get_char( '\\' ) && get() || comment_char() )
        {}
        return comment_end_char();
    }
    return false;
}

bool is_jcr_comment_char( char c )
{
    // comment_char() = HTAB() / %x20-3A / %x3C-10FFFF

    return c == '\t' || (c >= 0x20 && c <= 0x3a) || c >= 0x3c;
}

bool GrammarParser::comment_char()
{
    // comment_char() = HTAB() / %x20-3A / %x3C-10FFFF

    return is_get_char_in( cl::alphabet_function( is_jcr_comment_char ) );
}

bool GrammarParser::comment_end_char()
{
    // comment_end_char() = CR() || LF() || ";"

    return is_get_char_in( cl::alphabet_char_class( "\r\n;" ) ) || is_peek_at_end();
}

bool GrammarParser::directive()
{
    // directive() = "#" && [ spaces() ] && directive_def() && eol()

    if( is_get_char( '#' ) )
    {
        return optional( WSPs() ) &&
                (directive_def() || error( "Invalid #directive format" ) ) &&
                (eol() || error( "Unexpected additional material in directive" ) || recover_to_eol() );
    }
    return false;
}

bool GrammarParser::directive_def()
{
    // directive_def() = jcr_version_d() || ruleset_id_d() || import_d() || tbd_directive_d()

    cl::locator loc( this );

    return optional_rewind( jcr_version_d() ) ||
            optional_rewind( ruleset_id_d() ) ||
            optional_rewind( import_d() ) ||
            optional_rewind( tbd_directive_d() );
}

bool GrammarParser::jcr_version_d()
{
    // jcr_version_d() = jcr_version_kw() && spaces() && major_version() && "." && minor_version()

    cl::accumulator_deferred major_version_accumulator( this );
    cl::accumulator_deferred minor_version_accumulator( this );

    if( jcr_version_kw() )
    {
        if( WSPs() &&
                major_version_accumulator.select() && major_version() &&
                is_get_char( '.' ) &&
                minor_version_accumulator.select() && minor_version()
                || error( "Bad #jcr-version directive format" ) || recover_to_eol() )
        {
            // TODO...
            std::string major_version_number = major_version_accumulator.get();
            std::string minor_version_number = minor_version_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::major_version()
{
    // major_version() = p_integer()

    return p_integer();
}

bool GrammarParser::minor_version()
{
    // minor_version() = p_integer()

    return p_integer();
}

bool GrammarParser::ruleset_id_d()
{
    // ruleset_id_d() = ruleset_id_kw() && spaces() && ruleset_id()

    if( ruleset_id_kw() )
    {
        cl::accumulator ruleset_id_accumulator( this );

        if( (WSPs() && ruleset_id())
            || error( "Unable to read ruleset-id value" ) || recover_to_eol() )
        {
            // TODO...
            std::string ruleset_id_value = ruleset_id_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::import_d()
{
    // import_d() = import_kw() && spaces() && ruleset_id()
    //                    [ spaces() && as_kw() && spaces() && ruleset_id_alias() ]

    if( import_kw() )
    {
        cl::accumulator_deferred ruleset_id_accumulator( this );
        cl::accumulator_deferred ruleset_id_alias_accumulator( this );

        if( WSPs() &&
            (ruleset_id_accumulator.select() && ruleset_id() || error( "Unable to read ruleset-id in #import" )) &&
            optional(
                WSPs() &&
                as_kw() &&
                (WSPs() || error( "Expected space after 'as' keyword" ) ) &&
                (ruleset_id_alias_accumulator.select() && ruleset_id_alias() || error( "Unable to read alias for imported ruleset-id" ) ) ) )
        {
            // TODO...
            std::string ruleset_id_value = ruleset_id_accumulator.get();
            std::string ruleset_id_alias_value = ruleset_id_alias_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::ruleset_id()
{
    // ruleset_id() = ALPHA() && *not_space()

    return accumulate( cl::alphabet_alpha() ) && star_not_space();
}

bool GrammarParser::not_space()
{
    // not_space() = %x21-10FFFF

    return accumulate( cl::alphabet_not( cl::alphabet_space() ) );
}

bool GrammarParser::ruleset_id_alias()
{
    // ruleset_id_alias() = name()

    return name();
}

bool GrammarParser::tbd_directive_d()
{
    // tbd_directive_d() = directive_name() [ spaces() && directive_parameters() ]

    cl::accumulator tbd_directive_name_accumulator( this );
    cl::accumulator_deferred tbd_directive_parameters_accumulator( this );

    if( directive_name() &&
        optional( WSPs() && tbd_directive_parameters_accumulator.select() && directive_parameters() ) )
    {
        error( (std::string( "Unknown directive: " ) + tbd_directive_name_accumulator.get() +
                ", parameters: " + tbd_directive_parameters_accumulator.get()).c_str() );
        return true;
    }

    return false;
}

bool GrammarParser::directive_name()
{
    // directive_name() = name()

    return name();
}

bool GrammarParser::directive_parameters()
{
    // directive_parameters() = *not_eol()

    return star_not_eol();
}

bool GrammarParser::not_eol()
{
    // not_eol() = HTAB() / %x20-10FFFF

    return accumulate( cl::alphabet_not( cl::alphabet_eol() ) );
}

bool GrammarParser::eol()
{
    // eol() = CR() || LF()

    return is_get_char_in( cl::alphabet_eol() ) || is_peek_at_end();
}

bool GrammarParser::root_rule()
{
    // root_rule() = value_rule() || member_rule() || group_rule()

    cl::locator loc( this );

    return optional_rewind( value_rule() ) ||
            optional_rewind( member_rule() ) ||
            optional_rewind( group_rule() );
}

bool GrammarParser::rule()
{
    // rule() = rule_name() && *sp_cmt() && rule_def()

    cl::locator loc( this );

    cl::accumulator name_accumulator( this );

    if( ! rule_name() )
        return location_top( false );

    star_sp_cmt() && rule_def() || fatal( "Unable to read rule definition" );

    return true;    // We've 'accepted' this path, even if we end up deciding there's a fatal error
}

bool GrammarParser::rule_name()
{
    // rule_name() = name()

    return name();
}

bool GrammarParser::target_rule_name()
{
    // target_rule_name() = [ ruleset_id_alias() "." ] && rule_name()

    // Both ruleset_id_alias() and rule_name() ultimately map to name()
    // so a little juggling of values is required to get parsed values
    // in the right place

    std::string id_alias, rule;

    cl::accumulator first_accumulator( this );
    cl::accumulator_deferred second_accumulator( this );

    if( ruleset_id_alias() )
    {
        if( is_get_char( '.' ) )
        {
            second_accumulator.select();
            if( rule_name() )
            {
                id_alias = first_accumulator.get();
                rule = second_accumulator.get();
                return true;
            }
            else
                return error( "Expected 'rule_name' after 'ruleset_id_alias'" );
        }
        else
        {
            rule = first_accumulator.get();
            return true;
        }
    }

    return false;
}

bool GrammarParser::name()
{
    // name() = ALPHA() && *( ALPHA() || DIGIT() || "_" || "-" )

    if( ALPHA() )
    {
        while( ALPHA() || DIGIT() || accumulate( '_' ) || accumulate( '-' ) )
        {}
        return true;
    }

    return false;
}

bool GrammarParser::rule_def()
{
    // rule_def() = type_rule() || member_rule() || group_rule()

    cl::locator loc( this );

    return optional_rewind( type_rule() ) ||
            optional_rewind( member_rule() ) ||
            optional_rewind( group_rule() );
}

bool GrammarParser::type_rule()
{
    // type_rule() = value_rule() || type_choice_rule() || target_rule_name()

    cl::locator loc( this );

    return optional_rewind( value_rule() ) ||
            optional_rewind( type_choice_rule() ) ||
            optional_rewind( target_rule_name() );
}

bool GrammarParser::value_rule()
{
    // value_rule() = primitive_rule() || array_rule() || object_rule()

    cl::locator loc( this );

    return optional_rewind( primitive_rule() ) ||
            optional_rewind( array_rule() ) ||
            optional_rewind( object_rule() );
}

bool GrammarParser::member_rule()
{
    // member_rule() = annotations() && member_name_spec() && *sp_cmt() && type_rule()

    return false;
}

bool GrammarParser::member_name_spec()
{
    // member_name_spec() = regex() || q_string()

    return false;
}

bool GrammarParser::type_choice_rule()
{
    // type_choice_rule() = ":" && *sp_cmt() && type_choice()

    return false;
}

bool GrammarParser::type_choice()
{
    // type_choice() = annotations() "(" && type_choice_items() && *( choice_combiner() && type_choice_items() ) ")"

    return false;
}

bool GrammarParser::type_choice_items()
{
    // type_choice_items() = *sp_cmt() && ( type_choice() || type_rule() ) && *sp_cmt()

    return false;
}

bool GrammarParser::annotations( Annotations & anno )
{
    // annotations() = *( "@(" && *sp_cmt() && annotation_set() && *sp_cmt() && ")" && *sp_cmt() )

    while( fixed( "@(" ) )
    {
        star_sp_cmt() && annotation_set( anno ) && star_sp_cmt() &&
            (fixed( ")" ) || fatal( "Expected ')' at end of annotation" )) &&
            star_sp_cmt();
    }

    return true;    // Annotations are optional, so we always return 'true' unless we've errored
}

bool GrammarParser::annotation_set( Annotations & anno )
{
    // annotation_set() = reject_annotation() || unordered_annotation() || root_annotation() || tbd_annotation()

    cl::locator loc( this );    // Current annotations don't benefit from optional_rewind(), but maintain the pattern for consistency and possible future proofing

    return optional_rewind( reject_annotation() && set( anno.reject, true ) ) ||
            optional_rewind( unordered_annotation() && set( anno.is_unordered, true ) ) ||
            optional_rewind( root_annotation() && set( anno.is_root, true ) ) ||
            optional_rewind( tbd_annotation() ) ||
            fatal( "Unrecognised annotation format" );     // Getting to fatal() will throw an exception
}

bool GrammarParser::reject_annotation()
{
    // reject_annotation() = reject_kw()

    return reject_kw();
}

bool GrammarParser::unordered_annotation()
{
    // unordered_annotation() = unordered_kw()

    return unordered_kw();
}

bool GrammarParser::root_annotation()
{
    // root_annotation() = root_kw()

    return root_kw();
}

bool GrammarParser::tbd_annotation()
{
    // tbd_annotation() = annotation_name() [ spaces() && annotation_parameters() ]

    cl::accumulator name_accumulator( this );

    annotation_name() && optional( spaces() && name_accumulator.none() && annotation_parameters() );

    fatal( (std::string( "Annotation: '" ) + name_accumulator.get() + "' not supported").c_str() );

    return true;    // We've 'accepted' this path despite having decided to error
}

bool GrammarParser::annotation_name()
{
    // annotation_name() = name()

    return name();
}

bool GrammarParser::annotation_parameters()
{
    // annotation_parameters() = *( spaces() / %x21-28 / %x2A-10FFFF )

    return skip( cl::alphabet_not( cl::alphabet_char( ')' ) ) ) >= 0;
}

bool GrammarParser::primitive_rule()
{
    // primitive_rule() = annotations() ":" && *sp_cmt() && primimitive_def()

    return false;
}

bool GrammarParser::primimitive_def()
{
    // primimitive_def() = null_type() || boolean_type() || true_value() || false_value() || string_type() || string_range() || string_value() || float_type() || float_range() || float_value() || integer_type() || integer_range() || integer_value() || ip4_type() || ip6_type() || fqdn_type() || idn_type() || uri_range() || uri_type() || phone_type() || email_type() || full_date_type() || full_time_type() || date_time_type() || base64_type() || any()

    return false;
}

bool GrammarParser::null_type()
{
    // null_type() = null_kw()

    return false;
}

bool GrammarParser::boolean_type()
{
    // boolean_type() = boolean_kw()

    return false;
}

bool GrammarParser::true_value()
{
    // true_value() = true_kw()

    return false;
}

bool GrammarParser::false_value()
{
    // false_value() = false_kw()

    return false;
}

bool GrammarParser::string_type()
{
    // string_type() = string_kw()

    return false;
}

bool GrammarParser::string_value()
{
    // string_value() = q_string()

    return false;
}

bool GrammarParser::string_range()
{
    // string_range() = regex()

    return false;
}

bool GrammarParser::float_type()
{
    // float_type() = float_kw()

    return false;
}

bool GrammarParser::float_range()
{
    // float_range() = float_min() ".." [ float_max() ] || ".." && float_max()

    return false;
}

bool GrammarParser::float_min()
{
    // float_min() = float()

    return false;
}

bool GrammarParser::float_max()
{
    // float_max() = float()

    return false;
}

bool GrammarParser::float_value()
{
    // float_value() = float()

    return false;
}

bool GrammarParser::integer_type()
{
    // integer_type() = integer_kw()

    return false;
}

bool GrammarParser::integer_range()
{
    // integer_range() = integer_min() ".." [ integer_max() ] || ".." && integer_max()

    return false;
}

bool GrammarParser::integer_min()
{
    // integer_min() = integer()

    return false;
}

bool GrammarParser::integer_max()
{
    // integer_max() = integer()

    return false;
}

bool GrammarParser::integer_value()
{
    // integer_value() = integer()

    return false;
}

bool GrammarParser::ip4_type()
{
    // ip4_type() = ip4_kw()

    return false;
}

bool GrammarParser::ip6_type()
{
    // ip6_type() = ip6_kw()

    return false;
}

bool GrammarParser::fqdn_type()
{
    // fqdn_type() = fqdn_kw()

    return false;
}

bool GrammarParser::idn_type()
{
    // idn_type() = idn_kw()

    return false;
}

bool GrammarParser::uri_range()
{
    // uri_range() = uri_dotdot_kw() && uri_template()

    return false;
}

bool GrammarParser::uri_type()
{
    // uri_type() = uri_kw()

    return false;
}

bool GrammarParser::phone_type()
{
    // phone_type() = phone_kw()

    return false;
}

bool GrammarParser::email_type()
{
    // email_type() = email_kw()

    return false;
}

bool GrammarParser::full_date_type()
{
    // full_date_type() = full_date_kw()

    return false;
}

bool GrammarParser::full_time_type()
{
    // full_time_type() = full_time_kw()

    return false;
}

bool GrammarParser::date_time_type()
{
    // date_time_type() = date_time_kw()

    return false;
}

bool GrammarParser::base64_type()
{
    // base64_type() = base64_kw()

    return false;
}

bool GrammarParser::any()
{
    // any() = any_kw()

    return false;
}

bool GrammarParser::object_rule()
{
    // object_rule() = annotations() [ ":" && *sp_cmt() ] "{" && *sp_cmt() [ object_items() && *sp_cmt() ] "}"

    Annotations annos;
    annotations( annos );

    return false;
}

bool GrammarParser::object_items()
{
    // object_items() = object_item() && (*( sequence_combiner() && object_item() ) || *( choice_combiner() && object_item() ) )

    return false;
}

bool GrammarParser::object_item()
{
    // object_item() = [ repetition() && *sp_cmt() ] && object_item_types()

    return false;
}

bool GrammarParser::object_item_types()
{
    // object_item_types() = member_rule() || target_rule_name() || object_group()

    return false;
}

bool GrammarParser::object_group()
{
    // object_group() = "(" && *sp_cmt() [ object_items() && *sp_cmt() ] ")"

    return false;
}

bool GrammarParser::array_rule()
{
    // array_rule() = annotations() [ ":" && *sp_cmt() ] "[" && *sp_cmt() [ array_items() && *sp_cmt() ] "]"

    return false;
}

bool GrammarParser::array_items()
{
    // array_items() = array_item() && (*( sequence_combiner() && array_item() ) || *( choice_combiner() && array_item() ) )

    return false;
}

bool GrammarParser::array_item()
{
    // array_item() = [ repetition() ] && *sp_cmt() && array_item_types()

    return false;
}

bool GrammarParser::array_item_types()
{
    // array_item_types() = type_rule() || array_group()

    return false;
}

bool GrammarParser::array_group()
{
    // array_group() = "(" && *sp_cmt() [ array_items() && *sp_cmt() ] ")"

    return false;
}

bool GrammarParser::group_rule()
{
    // group_rule() = annotations() "(" && *sp_cmt() [ group_items() && *sp_cmt() ] ")"

    return false;
}

bool GrammarParser::group_items()
{
    // group_items() = group_item() && (*( sequence_combiner() && group_item() ) || *( choice_combiner() && group_item() ) )

    return false;
}

bool GrammarParser::group_item()
{
    // group_item() = [ repetition() ] && *sp_cmt() && group_item_types()

    return false;
}

bool GrammarParser::group_item_types()
{
    // group_item_types() = type_rule() || member_rule() || group_group()

    return false;
}

bool GrammarParser::group_group()
{
    // group_group() = group_rule()

    return false;
}

bool GrammarParser::sequence_combiner()
{
    // sequence_combiner() = *sp_cmt() "," && *sp_cmt()

    return false;
}

bool GrammarParser::choice_combiner()
{
    // choice_combiner() = *sp_cmt() "|" && *sp_cmt()

    return false;
}

bool GrammarParser::repetition( Repetition & reps )
{
    // repetition() = optional_marker() || one_or_more() || min_max_repetition() || min_repetition() || max_repetition() || zero_or_more() || specific_repetition()

    return false;
}

bool GrammarParser::optional_marker()
{
    // optional_marker() = "?"

    return false;
}

bool GrammarParser::one_or_more()
{
    // one_or_more() = "+"

    return false;
}

bool GrammarParser::zero_or_more()
{
    // zero_or_more() = "*"

    return false;
}

bool GrammarParser::min_max_repetition()
{
    // min_max_repetition() = min_repeat() && *sp_cmt() "*" && *sp_cmt() && max_repeat()

    return false;
}

bool GrammarParser::min_repetition()
{
    // min_repetition() = min_repeat() && *sp_cmt() "*"

    return false;
}

bool GrammarParser::max_repetition()
{
    // max_repetition() = "*" && *sp_cmt() && max_repeat()

    return false;
}

bool GrammarParser::min_repeat()
{
    // min_repeat() = p_integer()

    return false;
}

bool GrammarParser::max_repeat()
{
    // max_repeat() = p_integer()

    return false;
}

bool GrammarParser::specific_repetition()
{
    // specific_repetition() = p_integer()

    return false;
}

bool GrammarParser::integer()
{
    // integer() = ["-"] && 1*DIGIT()

    return false;
}

bool GrammarParser::p_integer()
{
    // p_integer() = 1*DIGIT()

    return one_star_DIGIT();
}

bool GrammarParser::float_num()
{
    // float() = [ minus() ] && int() && frac() [ exp() ]

    return false;
}

bool GrammarParser::minus()
{
    // minus() = %x2D                          ; -

    return false;
}

bool GrammarParser::plus()
{
    // plus() = %x2B                          ; +

    return false;
}

bool GrammarParser::int_num()
{
    // int() = zero() || ( digit1_9() && *DIGIT() )

    return false;
}

bool GrammarParser::digit1_9()
{
    // digit1_9() = %x31-39                       ; 1-9

    return false;
}

bool GrammarParser::frac()
{
    // frac() = decimal_point() && 1*DIGIT()

    return false;
}

bool GrammarParser::decimal_point()
{
    // decimal_point() = %x2E                          ; .

    return false;
}

bool GrammarParser::exp()
{
    // exp() = e() [ minus() || plus() ] && 1*DIGIT()

    return false;
}

bool GrammarParser::e()
{
    // e() = %x65 / %x45                   ; e() && E

    return false;
}

bool GrammarParser::zero()
{
    // zero() = %x30                          ; 0

    return false;
}

bool GrammarParser::q_string()
{
    // q_string() = quotation_mark() && *char() && quotation_mark()

    return false;
}
bool GrammarParser::regex()
{
    // regex() = "/" && *( escape() "/" || not_slash() ) "/" [ regex_modifiers() ]

    return false;
}

bool GrammarParser::not_slash()
{
    // not_slash() = HTAB() || CR() || LF() / %x20-2E / %x30-10FFFF

    return false;
}

bool GrammarParser::regex_modifiers()
{
    // regex_modifiers() = *( "i" || "s" || "x" )

    return false;
}

bool GrammarParser::uri_template()
{
    // uri_template() = 1*ALPHA() ":" && not_space()

    return false;
}

bool GrammarParser::any_kw()
{
    // any_kw() = %x61.6E.79                      ; "any"

    return fixed( "any" );
}

bool GrammarParser::as_kw()
{
    // as_kw() = %x61.73                         ; "as"

    return fixed( "as" );
}

bool GrammarParser::base64_kw()
{
    // base64_kw() = %x62.61.73.65.36.34             ; "base64"

    return fixed( "base64" );
}

bool GrammarParser::boolean_kw()
{
    // boolean_kw() = %x62.6F.6F.6C.65.61.6E          ; "boolean"

    return fixed( "boolean" );
}

bool GrammarParser::date_time_kw()
{
    // date_time_kw() = %x64.61.74.65.2D.74.69.6D.65    ; "date-time"

    return fixed( "date-time" );
}

bool GrammarParser::email_kw()
{
    // email_kw() = %x65.6D.61.69.6C                ; "email"

    return fixed( "email" );
}

bool GrammarParser::false_kw()
{
    // false_kw() = %x66.61.6C.73.65                ; "false"

    return fixed( "false" );
}

bool GrammarParser::float_kw()
{
    // float_kw() = %x66.6C.6F.61.74                ; "float"

    return fixed( "float" );
}

bool GrammarParser::fqdn_kw()
{
    // fqdn_kw() = %x66.71.64.6E                   ; "fqdn"

    return fixed( "fqdn" );
}

bool GrammarParser::full_date_kw()
{
    // full_date_kw() = %x66.75.6C.6C.2D.64.61.74.65    ; "full-date"

    return fixed( "full-date" );
}

bool GrammarParser::full_time_kw()
{
    // full_time_kw() = %x66.75.6C.6C.2D.74.69.6D.65    ; "full-time"

    return fixed( "full-time" );
}

bool GrammarParser::idn_kw()
{
    // idn_kw() = %x69.64.6E                      ; "idn"

    return fixed( "idn" );
}

bool GrammarParser::import_kw()
{
    // import_kw() = %x69.6D.70.6F.72.74             ; "import"

    return fixed( "import" );
}

bool GrammarParser::integer_kw()
{
    // integer_kw() = %x69.6E.74.65.67.65.72          ; "integer"

    return fixed( "integer" );
}

bool GrammarParser::ip4_kw()
{
    // ip4_kw() = %x69.70.34                      ; "ip4"

    return fixed( "ip4" );
}

bool GrammarParser::ip6_kw()
{
    // ip6_kw() = %x69.70.36                      ; "ip6"

    return fixed( "ip6" );
}

bool GrammarParser::jcr_version_kw()
{
    // jcr_version_kw() = %x6A.63.72.2D.76.65.72.73.69.6F.6E ; "jcr-version"

    return fixed( "jcr-version" );
}

bool GrammarParser::null_kw()
{
    // null_kw() = %x6E.75.6C.6C                   ; "null"

    return fixed( "null" );
}

bool GrammarParser::phone_kw()
{
    // phone_kw() = %x70.68.6F.6E.65                ; "phone"

    return fixed( "phone" );
}

bool GrammarParser::reject_kw()
{
    // reject_kw() = %x72.65.6A.65.63.74             ; "reject"

    return fixed( "reject" );
}

bool GrammarParser::root_kw()
{
    // root_kw() = %x72.6F.6F.74                   ; "root"

    return fixed( "root" );
}

bool GrammarParser::ruleset_id_kw()
{
    // ruleset_id_kw() = %x72.75.6C.65.73.65.74.2D.69.64 ; "ruleset-id"

    return fixed( "ruleset-id" );
}

bool GrammarParser::string_kw()
{
    // string_kw() = %x73.74.72.69.6E.67             ; "string"

    return fixed( "string" );
}

bool GrammarParser::true_kw()
{
    // true_kw() = %x74.72.75.65                   ; "true"

    return fixed( "true" );
}

bool GrammarParser::unordered_kw()
{
    // unordered_kw() = %x75.6E.6F.72.64.65.72.65.64    ; "unordered"

    return fixed( "unordered" );
}

bool GrammarParser::uri_dotdot_kw()
{
    // uri_dotdot_kw() = %x75.72.69.2E.2E                ; "uri.."

    return fixed( "uri.." );
}

bool GrammarParser::uri_kw()
{
    // uri_kw() = %x75.72.69                      ; "uri"

    return fixed( "uri" );
}

bool GrammarParser::ALPHA()
{
    // ALPHA() = %x41-5A / %x61-7A   ; A-Z / a-z

    return accumulate( cl::alphabet_alpha() );
}

bool GrammarParser::CR()
{
    // CR() = %x0D         ; carriage return

    return is_get_char( '\r' );
}

bool GrammarParser::DIGIT()
{
    // DIGIT() = %x30-39      ; 0-9

    return accumulate( cl::alphabet_digit() );
}

bool GrammarParser::HEXDIG()
{
    // HEXDIG() = DIGIT() || "A" || "B" || "C" || "D" || "E" || "F"

    return accumulate( cl::alphabet_hex() );
}

bool GrammarParser::HTAB()
{
    // HTAB() = %x09         ; horizontal tab

    return is_get_char( '\t' );
}

bool GrammarParser::LF()
{
    // LF() = %x0A         ; linefeed

    return is_get_char( '\x0a' );
}

bool GrammarParser::SP()
{
    // SP() = %x20         ; space

    return is_get_char( ' ' );
}

bool GrammarParser::WSP()
{
    // WSP() = SP() || HTAB()    ; white space

    return SP() || HTAB();
}

bool GrammarParser::WSPs()
{
    // WSPs() = 1*WSP()

    return one_star_WSP();
}

} // End of Anonymous namespace

//----------------------------------------------------------------------------
//                           class JCRParser
//----------------------------------------------------------------------------

JCRParser::Status JCRParser::add_grammar( const char * p_file_name )
{
    cl::reader_file reader( p_file_name );
    if( ! reader.is_open() )
        return S_UNABLE_TO_OPEN_FILE;

    return parse_grammar( reader );
}

JCRParser::Status JCRParser::add_grammar( const std::string & rules )
{
    cl::reader_string reader( rules );

    return parse_grammar( reader );
}

JCRParser::Status JCRParser::add_grammar( const char * p_rules, size_t size )
{
    cl::reader_mem_buf reader( p_rules, size );

    return parse_grammar( reader );
}

JCRParser::Status JCRParser::link()
{
    return S_OK;
}

JCRParser::Status JCRParser::parse_grammar( cl::reader & reader )
{
    GrammarParser parser( this, reader, &m.p_grammar_set->append_grammar() );

    parser.parse();

    return parser.status();
}

}   // namespace cljcr
