//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

// Implements jcr-abnf - 2016-07-11T15-33

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
#include <cassert>

namespace cljcr {

//----------------------------------------------------------------------------
//                           Internal detail
//----------------------------------------------------------------------------

namespace { // Anonymous namespace for detail

//----------------------------------------------------------------------------
//                           Standalone utility functions
//----------------------------------------------------------------------------

bool is_supported_jcr_version( const std::string & major, const std::string & minor )
{
    return major == "0" && minor == "6";
}

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

    class RuleStackLogger
    {
    // Use this class to keep track of which Rule we're logging to in a stack-like manner
    private:
        GrammarParser * p_grammar_parser;
        Rule * p_prev_rule;
    public:
        RuleStackLogger( GrammarParser * p_grammar_parser_in, Rule::uniq_ptr & pu_rule )
            : p_grammar_parser( p_grammar_parser_in )
        {
            p_prev_rule = p_grammar_parser->m.p_rule;
            if( p_prev_rule )
                pu_rule->p_parent = p_prev_rule;
            p_grammar_parser->m.p_rule = pu_rule.get();
        }
        ~RuleStackLogger()
        {
            p_grammar_parser->m.p_rule = p_prev_rule;
        }
    };

public:
    GrammarParser( JCRParser * p_parent, cl::reader & r_reader, Grammar * p_grammar );
    bool parse();
    JCRParser::Status status() const { return m.status; }

private:
    #define STAR( what ) bool star_ ## what() { while( what() ) {} return true; }
    #define ONE_STAR( what ) bool one_star_ ## what() { size_t n=0; while( what() ) ++n; return n > 0; }

    bool jcr();
    bool sp_cmt();
    STAR( sp_cmt )
    ONE_STAR( sp_cmt )
    bool spaces();
    bool comment();
    bool comment_char();
    bool comment_end_char();
    bool directive();
    bool one_line_directive();
    bool multi_line_directive();
    bool directive_def();
    bool jcr_version_d();
    bool DSPs();
    bool major_version();
    bool minor_version();
    bool ruleset_id_d();
    bool import_d();
    bool ruleset_id();
    bool not_space();
    STAR( not_space )
    ONE_STAR( not_space )
    bool ruleset_id_alias();
    bool one_line_tbd_directive_d();
    bool directive_name();
    bool one_line_directive_parameters();
    bool not_eol();
    STAR( not_eol )
    bool eol();
    bool multi_line_tbd_directive_d();
    bool multi_line_directive_parameters();
    bool multi_line_parameters();
    bool not_multi_line_special();
    bool root_rule();
    bool rule();
    bool rule_name();
    bool target_rule_name();
    bool name();
    bool rule_def();
    bool type_designator();
    bool rule_def_type_rule();
    bool value_rule();
    bool member_rule();
    bool member_name_spec();
    bool type_rule();
    bool explicit_type_choice();
    bool type_choice();
    bool type_choice_items();
    bool annotations( Annotations & );
    bool annotation_set( Annotations & );
    bool not_annotation( Annotations & );
    bool unordered_annotation( Annotations & );
    bool root_annotation( Annotations & );
    bool tbd_annotation();
    bool annotation_name();
    bool annotation_parameters();
    bool primitive_rule();
    bool primitive_def();
    bool null_type();
    bool boolean_type();
    bool true_value();
    bool false_value();
    bool string_type();
    bool string_value();
    bool string_range();
    bool double_type();
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
    bool sized_int_type();
    bool sized_uint_type();
    bool ipv4_type();
    bool ipv6_type();
    bool ipaddr_type();
    bool fqdn_type();
    bool idn_type();
    bool uri_range();
    bool uri_type();
    bool phone_type();
    bool email_type();
    bool datetime_type();
    bool date_type();
    bool time_type();
    bool hex_type();
    bool base32hex_type();
    bool base32_type();
    bool base64url_type();
    bool base64_type();
    bool any();
    bool object_rule();
    bool object_items();
    bool star_sequence_combiner_and_object_item();
    bool star_choice_combiner_and_object_item();
    bool object_item();
    bool object_item_types();
    bool object_group();
    bool array_rule();
    bool array_items();
    bool star_sequence_combiner_and_array_item();
    bool star_choice_combiner_and_array_item();
    bool array_item();
    bool array_item_types();
    bool array_group();
    bool group_rule();
    bool group_items();
    bool star_sequence_combiner_and_group_item();
    bool star_choice_combiner_and_group_item();
    bool group_item();
    bool group_item_types();
    bool group_group();
    bool sequence_combiner();
    bool choice_combiner();
    bool repetition();
    bool optional_marker();
    bool one_or_more();
    bool zero_or_more();
    bool min_max_repetition();
    bool min_repetition();
    bool max_repetition();
    bool min_repeat();
    bool max_repeat();
    bool specific_repetition();
    bool repetition_step();
    bool integer();
    bool non_neg_integer();
    bool pos_integer();
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
    bool q_string_as_utf8();
    bool q_string();
    bool qs_char();
    STAR( qs_char )
    bool escape();
    bool quotation_mark();
    bool unescaped();
    bool escaped_code();
    bool u();
    bool four_HEXDIG();
    bool regex();
    bool not_slash();
    bool regex_modifiers();
    bool uri_template();
    bool any_kw();
    bool as_kw();
    bool base32_kw();
    bool base32hex_kw();
    bool base64_kw();
    bool base64url_kw();
    bool boolean_kw();
    bool date_kw();
    bool datetime_kw();
    bool double_kw();
    bool email_kw();
    bool false_kw();
    bool float_kw();
    bool fqdn_kw();
    bool hex_kw();
    bool idn_kw();
    bool import_kw();
    bool int_kw();
    bool integer_kw();
    bool ipaddr_kw();
    bool ipv4_kw();
    bool ipv6_kw();
    bool jcr_version_kw();
    bool not_kw();
    bool null_kw();
    bool phone_kw();
    bool root_kw();
    bool ruleset_id_kw();
    bool string_kw();
    bool time_kw();
    bool true_kw();
    bool type_kw();
    bool uint_kw();
    bool unordered_kw();
    bool uri_dotdot_kw();
    bool uri_kw();
    bool ALPHA();
    ONE_STAR( ALPHA )
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

    // These are temporary place holders
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
    bool recover_to_eol( bool ret = true )
    {
        while( is_get_char_in( cl::alphabet_not( cl::alphabet_eol() ) ) )
        {}
        get();  // Get the end of line character
        return ret;
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
    /* ABNF: 
    jcr              = *( sp-cmt / directive / root-rule / rule )
    */
    // *( sp_cmt() || directive() || root_rule() || rule() )

    try
    {
        while( sp_cmt() || directive() || root_rule() || rule() )
        {}
        return is_peek_at_end() || fatal( "Unexpected input" );
    }
    catch( const GrammarParserFatalError & )
    {
        return false;
    }

    return false;
}

bool GrammarParser::sp_cmt()
{
    /* ABNF: 
    sp-cmt           = spaces / comment
    */
    // spaces() || comment()

    return spaces() || comment();
}

bool GrammarParser::spaces()
{
    /* ABNF: 
    spaces           = 1*( WSP / CR / LF )
    */
    // 1*( WSP() || CR() || LF() )

    int i = 0;
    while( WSP() || CR() || LF() )
        ++i;
    return i > 0;
}

bool GrammarParser::comment()
{
    /* ABNF: 
    comment          = ";" *( "\;" / comment-char ) comment-end-char
    */
    // ";" && *( "\;" || comment_char() ) && comment_end_char()

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
    /* ABNF: 
    comment-char     = HTAB / %x20-3A / %x3C-10FFFF
    */
    // HTAB() / %x20-3A / %x3C-10FFFF

    return is_get_char_in( cl::alphabet_function( is_jcr_comment_char ) );
}

bool GrammarParser::comment_end_char()
{
    /* ABNF: 
    comment-end-char = CR / LF / ";"
    */
    // CR() || LF() || ";"

    return is_get_char_in( cl::alphabet_char_class( "\r\n;" ) ) || is_peek_at_end();
}

bool GrammarParser::directive()
{
    /* ABNF: 
    directive        = "#" (one-line-directive / multi-line-directive)
    */
    // "#" && (one_line_directive() || multi_line_directive())

    if( is_get_char( '#' ) )
    {
        cl::locator loc( this );

        (one_line_directive() || multi_line_directive() || error( "Invalid #directive format" ) );

        return true;
    }

    return false;
}

bool GrammarParser::one_line_directive()
{
    /* ABNF: 
    one-line-directive = [ spaces ] 
                   (directive-def / one-line-tbd-directive-d) *WSP eol
    */
    // [ spaces() ] && (directive_def() || one_line_tbd_directive_d()) && *WSP() && eol()

    cl::locator loc( this );

    if( star_WSP() && (directive_def() || one_line_tbd_directive_d()) )
    {
        // Use is_peek_at_end() to allow ruleset to end with a directive that doesn't have newline at end
        star_WSP() && (eol() || is_peek_at_end()) || error( "Unexpected additional material in directive" );

        return true;
    }

    location_top();

    return false;
}

bool GrammarParser::multi_line_directive()
{
    /* ABNF: 
    multi-line-directive = "{" *sp-cmt
                   (directive-def / multi-line-tbd-directive-d) *sp-cmt "}"
    */
    // "{" && *sp_cmt() && (directive_def() || multi_line_tbd_directive_d()) && *sp_cmt() "}"

    if( is_get_char( '{' ) )
    {
        star_sp_cmt() && (directive_def() || multi_line_tbd_directive_d()) &&
            star_sp_cmt() && is_get_char( '}' ) || error( "Invalid multi-line #{directive} format" );
        return true;
    }

    return false;
}

bool GrammarParser::directive_def()
{
    /* ABNF: 
    directive-def    = jcr-version-d / ruleset-id-d / import-d
    */
    // jcr_version_d() || ruleset_id_d() || import_d()

    cl::locator loc( this );

    return optional_rewind( jcr_version_d() ) ||
            optional_rewind( ruleset_id_d() ) ||
            optional_rewind( import_d() );
}

bool GrammarParser::jcr_version_d()
{
    /* ABNF: 
    jcr-version-d    = jcr-version-kw spaces major-version "." minor-version
    */
    // jcr_version_kw() && spaces() && major_version() && "." && minor_version()

    cl::accumulator_deferred major_version_accumulator( this );
    cl::accumulator_deferred minor_version_accumulator( this );

    if( jcr_version_kw() )
    {
        if( (DSPs() || error( "Expected WSP tokens after 'jcr-version' keyword")) &&
                major_version_accumulator.select() && major_version() &&
                is_get_char( '.' ) &&
                minor_version_accumulator.select() && minor_version()
                || error( "Bad #jcr-version directive format" ) || recover_to_eol( false ) )
        {
            std::string major_number = major_version_accumulator.get();
            std::string minor_number = minor_version_accumulator.get();

            if( ! is_supported_jcr_version( major_number, minor_number ) )
                error( (std::string( "Unsupported JCR version: " ) + major_number + "." + minor_number).c_str() );
        }

        return true;
    }

    return false;
}

bool GrammarParser::DSPs()  // "Directive spaces" - May later include a flag to test if in one-line or multi-line directive
{
    return spaces();
}

bool GrammarParser::major_version()
{
    /* ABNF: 
    major-version    = non-neg-integer
    */
    // non_neg_integer()

    return non_neg_integer();
}

bool GrammarParser::minor_version()
{
    /* ABNF: 
    minor-version    = non-neg-integer
    */
    // non_neg_integer()

    return non_neg_integer();
}

bool GrammarParser::ruleset_id_d()
{
    /* ABNF: 
    ruleset-id-d     = ruleset-id-kw spaces ruleset-id
    */
    // ruleset_id_kw() && spaces() && ruleset_id()

    if( ruleset_id_kw() )
    {
        cl::accumulator ruleset_id_accumulator( this );

        if( (DSPs() && ruleset_id())
            || error( "Unable to read ruleset-id value" ) || recover_to_eol() )
        {
            m.p_grammar->ruleset_id = ruleset_id_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::import_d()
{
    /* ABNF: 
    import-d         = import-kw spaces ruleset-id
                   [ spaces as-kw spaces ruleset-id-alias ]
    */
    // import_kw() && spaces() && ruleset_id()
    //                    [ spaces() && as_kw() && spaces() && ruleset_id_alias() ]

    if( import_kw() )
    {
        cl::accumulator_deferred ruleset_id_accumulator( this );
        cl::accumulator_deferred ruleset_id_alias_accumulator( this );

        if( DSPs() &&
            (ruleset_id_accumulator.select() && ruleset_id() || error( "Unable to read ruleset-id in #import" )) &&
            optional(
                DSPs() &&
                as_kw() &&
                (DSPs() || error( "Expected space after 'as' keyword" ) ) &&
                (ruleset_id_alias_accumulator.select() && ruleset_id_alias() || error( "Unable to read alias for imported ruleset-id" ) ) ) )
        {
            std::string ruleset_id = ruleset_id_accumulator.get();
            std::string ruleset_id_alias = ruleset_id_alias_accumulator.get();
            if( ruleset_id_alias.empty() )
                m.p_grammar->add_unaliased_import( ruleset_id );
            else
                m.p_grammar->add_aliased_import( ruleset_id_alias, ruleset_id );
        }

        return true;
    }

    return false;
}

bool GrammarParser::ruleset_id()
{
    /* ABNF: 
    ruleset-id       = ALPHA *not-space
    */
    // ALPHA() && *not_space()

    return accumulate( cl::alphabet_alpha() ) && star_not_space();
}

bool GrammarParser::not_space()
{
    /* ABNF: 
    not-space        = %x21-10FFFF
    */
    // %x21-10FFFF

    return accumulate( cl::alphabet_not( cl::alphabet_space() ) );
}

bool GrammarParser::ruleset_id_alias()
{
    /* ABNF: 
    ruleset-id-alias = name
    */
    // name()

    return name();
}

bool GrammarParser::one_line_tbd_directive_d()
{
    /* ABNF: 
    one-line-tbd-directive-d = directive-name [ WSP one-line-directive-parameters ]
    */
    // directive_name() [ WSP() && one_line_directive_parameters() ]

    cl::accumulator tbd_directive_name_accumulator( this );
    cl::accumulator_deferred tbd_directive_parameters_accumulator( this );

    if( directive_name() &&
        optional( DSPs() && tbd_directive_parameters_accumulator.select() && one_line_directive_parameters() ) )
    {
        warning( (std::string( "Unknown directive: " ) + tbd_directive_name_accumulator.get() +
                ", parameters: " + tbd_directive_parameters_accumulator.get()).c_str() );
        return true;
    }

    return false;
}

bool GrammarParser::directive_name()
{
    /* ABNF: 
    directive-name   = name
    */
    // name()

    return name();
}

bool GrammarParser::one_line_directive_parameters()
{
    /* ABNF: 
    one-line-directive-parameters = *not-eol
    */
    // *not_eol()

    return star_not_eol();
}

bool GrammarParser::not_eol()
{
    /* ABNF: 
    not-eol          = HTAB / %x20-10FFFF
    */
    // HTAB() / %x20-10FFFF

    return accumulate( cl::alphabet_not( cl::alphabet_eol() ) );
}

bool GrammarParser::eol()
{
    /* ABNF: 
    eol              = CR / LF
    */
    // CR() || LF()

    return is_get_char_in( cl::alphabet_eol() );
}

bool GrammarParser::multi_line_tbd_directive_d()
{
    /* ABNF: 
    multi-line-tbd-directive-d = directive-name
                   [ spaces multi-line-directive-parameters ]
    */
    // directive_name()
    //                    [ spaces() && multi_line_directive_parameters() ]

    cl::accumulator tbd_directive_name_accumulator( this );
    cl::accumulator_deferred tbd_directive_parameters_accumulator( this );

    if( directive_name() &&
        optional( spaces() && tbd_directive_parameters_accumulator.select() && multi_line_directive_parameters() ) )
    {
        warning( (std::string( "Unknown directive: " ) + tbd_directive_name_accumulator.get()).c_str() );
        return true;
    }

    return false;
}

bool GrammarParser::multi_line_directive_parameters()
{
    /* ABNF: 
    multi-line-directive-parameters = multi-line-parameters
    */
    // multi_line_parameters()

    return multi_line_parameters();
}

bool GrammarParser::multi_line_parameters()
{
    /* ABNF: 
    multi-line-parameters = *(comment / q-string / regex /
                   not-multi-line-special)
    */
    // *(comment() || q_string() || regex() || not_multi_line_special())

    while( comment() || q_string() || regex() || not_multi_line_special() )
    {}

    return true;
}

cl::alphabet_char_class not_dquote_or_slash_or_semicolon_or_right_brace( "^\"/;}" );

bool GrammarParser::not_multi_line_special()
{
    /* ABNF: 
    not-multi-line-special = spaces / %x21 / %x23-2E / %x30-3A / %x3C-7C /
                   %x7E-10FFFF ; not ", /, ; or }
    */
    // spaces() / %x21 / %x23-2E / %x30-3A / %x3C-7C /
    //                    %x7E-10FFFF ; not ", /, ; or }

    return skip( not_dquote_or_slash_or_semicolon_or_right_brace ) > 0;
}

bool GrammarParser::root_rule()
{
    /* ABNF: 
    root-rule        = value-rule / group-rule
    */
    // value_rule() || group_rule()

    cl::locator loc( this );

    Rule::uniq_ptr pu_rule( new Rule );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( optional_rewind( value_rule() ) || optional_rewind( group_rule() ) )
    {
        m.p_rule->annotations.is_root = true;

        m.p_grammar->append_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::rule()
{
    /* ABNF: 
    rule             = annotations "$" rule-name *sp-cmt "=" *sp-cmt rule-def
    */
    // annotations() && "$" && rule_name() && *sp_cmt() && "=" && *sp_cmt() && rule_def()

    Annotations rule_annotations;

    if( annotations( rule_annotations ) && is_get_char( '$' ) )
    {
        cl::accumulator name_accumulator( this );

        Rule::uniq_ptr pu_rule( new Rule );
        RuleStackLogger rule_stack_logger( this, pu_rule );

        rule_name() && star_sp_cmt() && is_get_char( '=' ) && star_sp_cmt() && rule_def() || fatal( "Unable to read rule definition" );

        m.p_rule->rule_name = name_accumulator.get();
        m.p_rule->annotations.merge( rule_annotations );

        m.p_grammar->append_rule( pu_rule );

        return true;    // We've 'accepted' this path, even if we end up deciding there's a fatal error
    }

    return false;
}

bool GrammarParser::rule_name()
{
    /* ABNF: 
    rule-name        = name
    */
    // name()

    return name();
}

bool GrammarParser::target_rule_name()
{
    /* ABNF: 
    target-rule-name = annotations "$" [ ruleset-id-alias "." ] rule-name
    */
    // annotations() "$" [ ruleset_id_alias() "." ] && rule_name()

    // Both ruleset_id_alias() and rule_name() ultimately map to name()
    // so a little juggling of values is required to get parsed values
    // in the right place

    Annotations target_rule_name_annotations;

    if( annotations( target_rule_name_annotations ) && is_get_char( '$' ) )
    {
        cl::accumulator first_accumulator( this );
        cl::accumulator_deferred second_accumulator( this );

        ruleset_id_alias() || error( "Expected 'target_rule_name' after '$'" );

        if( is_get_char( '.' ) )
        {
            second_accumulator.select();
            if( rule_name() )
            {
                AliasLookupResult alias_lookup_result( m.p_grammar->get_aliased_import( first_accumulator.get() ) );
                if( ! alias_lookup_result.is_found() )
                    return fatal( (std::string( "Unknown alias in target rule: " ) + first_accumulator.get() ).c_str() );

                m.p_rule->target_rule.rulesetid = alias_lookup_result;
                m.p_rule->target_rule.local_name = second_accumulator.get();
            }
            else
                return error( "Expected 'rule_name' after 'rulesetid_alias'" );
        }
        else
        {
            m.p_rule->target_rule.local_name = first_accumulator.get();
        }

        m.p_rule->type = Rule::TARGET_RULE;
        m.p_rule->annotations.merge( target_rule_name_annotations );

        return true;
    }

    return false;
}

bool GrammarParser::name()
{
    /* ABNF: 
    name             = ALPHA *( ALPHA / DIGIT / "-" / "-" )
    */
    // ALPHA() && *( ALPHA() || DIGIT() || "-" || "-" )

    // ALPHA() && *( ALPHA() || DIGIT() || "_" || "-" )

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
    /* ABNF: 
    rule-def         = member-rule / type-designator rule-def-type-rule /
                   array-rule / object-rule / group-rule / target-rule-name
    */
    // member_rule() || type_designator() && rule_def_type_rule() || array_rule() || object_rule() || group_rule() || target_rule_name()

    cl::locator loc( this );

    return optional_rewind( member_rule() ) ||
            optional_rewind( type_designator() && rule_def_type_rule() ) ||
            optional_rewind( array_rule() ) ||
            optional_rewind( object_rule() ) ||
            optional_rewind( group_rule() ) ||
            optional_rewind( target_rule_name() );
}

bool GrammarParser::type_designator()
{
    /* ABNF: 
    type-designator  = "type" 1*sp-cmt / ":" *sp-cmt
    */
    // "type" && 1*sp_cmt() || ":" && *sp_cmt()

    cl::locator loc( this );

    return optional_rewind( type_kw() && one_star_sp_cmt() ) ||
            optional_rewind( is_get_char( ':' ) && star_sp_cmt() );
}

bool GrammarParser::rule_def_type_rule()
{
    /* ABNF: 
    rule-def-type-rule = value-rule / type-choice-rule
    */
    // value_rule() || type_choice()

    cl::locator loc( this );

    return optional_rewind( value_rule() ) ||
            optional_rewind( type_choice() );
}

bool GrammarParser::value_rule()
{
    /* ABNF: 
    value-rule       = primitive-rule / array-rule / object-rule
    */
    // primitive_rule() || array_rule() || object_rule()

    cl::locator loc( this );

    return optional_rewind( primitive_rule() ) ||
            optional_rewind( array_rule() ) ||
            optional_rewind( object_rule() );
}

bool GrammarParser::member_rule()
{
    /* ABNF: 
    member-rule      = annotations
                   member-name-spec *sp-cmt ":" *sp-cmt type-rule
    */
    // annotations() && member_name_spec() && *sp_cmt() ":" && *sp_cmt() && type_rule()

    // No need to record location because member_rule() is always part of a rewound choice

    Annotations member_rule_annotations;

    if( annotations( member_rule_annotations ) && member_name_spec() )
    {
        star_sp_cmt() && (is_get_char( ':' ) || fatal( "Expected ':' after name of member rule" )) &&
            star_sp_cmt() && type_rule() || fatal( "Expected type-rule after member-name" );

        m.p_rule->annotations.merge( member_rule_annotations );

        return true;
    }

    return false;
}

bool GrammarParser::member_name_spec()
{
    /* ABNF: 
    member-name-spec = regex / q-string
    */
    // regex() || q_string()

    cl::accumulator member_name_accumulator( this );

    if( regex() )
    {
        m.p_rule->member_name.set_regex( member_name_accumulator.get() );

        return true;
    }

    else if( member_name_accumulator.clear() && q_string_as_utf8() )
    {
        m.p_rule->member_name.set_literal( member_name_accumulator.get() );

        return true;
    }

    return false;
}

bool GrammarParser::type_rule()
{
    /* ABNF: 
    type-rule        = value-rule / type-choice-rule / target-rule-name
    */
    // value_rule() || type_choice() || target_rule_name()

    cl::locator loc( this );

    return optional_rewind( value_rule() ) ||
            optional_rewind( type_choice() ) ||
            optional_rewind( target_rule_name() );
}

bool GrammarParser::type_choice()
{
    /* ABNF: 
    type-choice      = annotations "(" type-choice-items
                   *( choice-combiner type-choice-items ) ")"
    */
    // annotations() && "(" && type_choice_items() && *( choice_combiner() && type_choice_items() ) && ")"

    Annotations type_choice_annotations;

    if( annotations( type_choice_annotations ) && is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::TYPE_CHOICE;
        m.p_rule->child_combiner = Rule::Choice;
        m.p_rule->annotations.merge( type_choice_annotations );

        type_choice_items() || fatal( "Must be at least one type specified within a type-choice" );

        while( choice_combiner() )
        {
            type_choice_items() || fatal( "Expected type-choice-item after choice-combiner in type-choice" );
        }

        is_get_char( ')' ) || fatal( "Expected ')' at end of type-choice" );

        return true;
    }

    return false;
}

bool GrammarParser::explicit_type_choice()
{
    /* ABNF: 
    explicit-type-choice = type-designator type-choice
    */
    // type_designator() && type_choice()

    cl::locator loc_outer( this );

    return type_designator() && type_choice() || location_top( false );
}

bool GrammarParser::type_choice_items()
{
    /* ABNF: 
    type-choice-items = *sp-cmt ( type-choice-rule / type-rule ) *sp-cmt
    */
    // *sp_cmt() && ( type_choice() || type_rule() ) && *sp_cmt()

    cl::locator loc_outer( this );

    star_sp_cmt();

    cl::locator loc_inner( this );

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new Rule );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( ( optional_rewind( type_choice() ) || optional_rewind( type_rule() ) ) &&
            star_sp_cmt() )
    {
        p_parent->append_child_rule( pu_rule );
        return true;
    }
    
    location_top();

    return false;
}

bool GrammarParser::annotations( Annotations & r_annotations )
{
    /* ABNF: 
    annotations      = *( "@{" *sp-cmt annotation-set *sp-cmt "}" *sp-cmt )
    */
    // *( "@{" && *sp_cmt() && annotation_set() && *sp_cmt() && "}" && *sp_cmt() )

    // *( "@{" && *sp_cmt() && annotation_set() && *sp_cmt() && "}" && *sp_cmt() )

    while( fixed( "@{" ) )
    {
        star_sp_cmt() && annotation_set( r_annotations ) && star_sp_cmt() &&
            (fixed( "}" ) || fatal( "Expected ')' at end of annotation" )) &&
            star_sp_cmt();
    }

    return true;    // Annotations are optional, so we always return 'true' unless we've errored
}

bool GrammarParser::annotation_set( Annotations & r_annotations )
{
    /* ABNF: 
    annotation-set   = not-annotation / unordered-annotation /
                   root-annotation / tbd-annotation
    */
    // not_annotation() || unordered_annotation() || root_annotation() || tbd_annotation()

    cl::locator loc( this );    // Current annotations don't benefit from optional_rewind(), but maintain the pattern for consistency and possible future proofing

    return optional_rewind( not_annotation( r_annotations ) ) ||
            optional_rewind( unordered_annotation( r_annotations ) ) ||
            optional_rewind( root_annotation( r_annotations ) ) ||
            optional_rewind( tbd_annotation() ) ||
            fatal( "Unrecognised annotation format" );     // Getting to fatal() will throw an exception
}

bool GrammarParser::not_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    not-annotation   = not-kw
    */
    // not_kw()

    return not_kw() && set( r_annotations.is_not, true );
}

bool GrammarParser::unordered_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    unordered-annotation = unordered-kw
    */
    // unordered_kw()

    return unordered_kw() && set( r_annotations.is_unordered, true );
}

bool GrammarParser::root_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    root-annotation  = root-kw
    */
    // root_kw()

    return root_kw() && set( r_annotations.is_root, true );
}

bool GrammarParser::tbd_annotation()
{
    /* ABNF: 
    tbd-annotation   = annotation-name [ spaces annotation-parameters ]
    */
    // annotation_name() [ spaces() && annotation_parameters() ]

    // annotation_name() [ spaces() && annotation_parameters() ]

    cl::accumulator name_accumulator( this );

    annotation_name() && optional( spaces() && name_accumulator.none() && annotation_parameters() );

    if( name_accumulator.get() == "id" || name_accumulator.get() == "assert" || name_accumulator.get() == "when" || name_accumulator.get() == "doc" )
        warning( (std::string( "Annotation: '" ) + name_accumulator.get() + "' not yet implemented").c_str() );
    else
        fatal( (std::string( "Annotation: '" ) + name_accumulator.get() + "' unknown").c_str() );

    return true;    // We've 'accepted' this path despite having decided to error
}

bool GrammarParser::annotation_name()
{
    /* ABNF: 
    annotation-name  = name
    */
    // name()

    return name();
}

bool GrammarParser::annotation_parameters()
{
    /* ABNF: 
    annotation-parameters = multi-line-parameters
    */
    // multi_line_parameters()

    return multi_line_parameters();
}

bool GrammarParser::primitive_rule()
{
    /* ABNF: 
    primitive-rule   = annotations primitive-def
    */
    // annotations() && primitive_def()

    Annotations primitive_rule_annotations;

    if( annotations( primitive_rule_annotations ) && primitive_def() )
    {
        m.p_rule->annotations.merge( primitive_rule_annotations );
        return true;
    }

    return false;
}

bool GrammarParser::primitive_def()
{
    /* ABNF: 
    primitive-def    = string-type / string-range / string-value /
                   null-type / boolean-type / true-value / false-value /
                   double-type / float-type / float-range / float-value /
                   integer-type / integer-range / integer-value /
                   sized-int-type / sized-uint-type /
                   ipv4-type / ipv6-type / ipaddr-type / fqdn-type / idn-type /
                   uri-range / uri-type / phone-type / email-type /
                   datetime-type / date-type / time-type /
                   hex-type / base32hex-type / base32-type / base64url-type / base64-type /
                   any
    */
    // string_type() || string_range() || string_value() ||
    //      null_type() ||
    //      boolean_type() || true_value() || false_value() ||
    //      double_type() || float_type() || float_range() || float_value() ||
    //      integer_type() || integer_range() || integer_value() || sized_int_type() || sized_uint_type() ||
    //      ipv4_type() || ipv6_type() || ipaddr_type() || fqdn_type() || idn_type() ||
    //      uri_range() || uri_type() ||
    //      phone_type() || email_type() ||
    //      datetime_type() || date_type() || time_type() ||
    //      hex_type() || base32hex_type() || base32_type() || base64url_type() || base64_type() ||
    //      any()

    cl::locator loc( this );

     return optional_rewind( null_type() ) ||
            optional_rewind( boolean_type() ) ||
            optional_rewind( true_value() ) ||
            optional_rewind( false_value() ) ||
            optional_rewind( string_type() ) ||
            optional_rewind( string_range() ) ||
            optional_rewind( string_value() ) ||
            optional_rewind( double_type() ) ||
            optional_rewind( float_type() ) ||
            optional_rewind( float_range() ) ||
            optional_rewind( float_value() ) ||
            optional_rewind( integer_type() ) ||
            optional_rewind( integer_range() ) ||
            optional_rewind( integer_value() ) ||
            optional_rewind( sized_int_type() ) ||
            optional_rewind( sized_uint_type() ) ||
            optional_rewind( ipv4_type() ) ||
            optional_rewind( ipv6_type() ) ||
            optional_rewind( ipaddr_type() ) ||
            optional_rewind( fqdn_type() ) ||
            optional_rewind( idn_type() ) ||
            optional_rewind( uri_range() ) ||
            optional_rewind( uri_type() ) ||
            optional_rewind( phone_type() ) ||
            optional_rewind( email_type() ) ||
            optional_rewind( datetime_type() ) ||
            optional_rewind( date_type() ) ||
            optional_rewind( time_type() ) ||
            optional_rewind( hex_type() ) ||
            optional_rewind( base32hex_type() ) ||
            optional_rewind( base32_type() ) ||
            optional_rewind( base64url_type() ) ||
            optional_rewind( base64_type() ) ||
            optional_rewind( any() );
}

bool GrammarParser::null_type()
{
    /* ABNF: 
    null-type        = null-kw
    */
    // null_kw()

    return null_kw() && set( m.p_rule->type, Rule::TNULL );
}

bool GrammarParser::boolean_type()
{
    /* ABNF: 
    boolean-type     = boolean-kw
    */
    // boolean_kw()

    return boolean_kw() && set( m.p_rule->type, Rule::BOOLEAN );
}

bool GrammarParser::true_value()
{
    /* ABNF: 
    true-value       = true-kw
    */
    // true_kw()

    return true_kw() &&
            set( m.p_rule->type, Rule::BOOLEAN ) &&
            set( m.p_rule->min, true ) &&
            set( m.p_rule->max, true );
}

bool GrammarParser::false_value()
{
    /* ABNF: 
    false-value      = false-kw
    */
    // false_kw()

    return false_kw() &&
            set( m.p_rule->type, Rule::BOOLEAN ) &&
            set( m.p_rule->min, false ) &&
            set( m.p_rule->max, false );
}

bool GrammarParser::string_type()
{
    /* ABNF: 
    string-type      = string-kw
    */
    // string_kw()

    return string_kw() && set( m.p_rule->type, Rule::STRING_TYPE );
}

bool GrammarParser::string_value()
{
    /* ABNF: 
    string-value     = q-string
    */
    // q_string()

    cl::accumulator q_string_accumulator( this );

    if( q_string_as_utf8() )
    {
        m.p_rule->type = Rule::STRING_LITERAL;
        m.p_rule->min = m.p_rule->max = q_string_accumulator.get();

        return true;
    }

    return false;
}

bool GrammarParser::string_range()
{
    /* ABNF: 
    string-range     = regex
    */
    // regex()

    cl::accumulator regex_accumulator( this );

    if( regex() )
    {
        m.p_rule->type = Rule::STRING_REGEX;
        m.p_rule->min = m.p_rule->max = regex_accumulator.get();

        return true;
    }

    return false;
}

bool GrammarParser::double_type()
{
    /* ABNF: 
    double-type      = double-kw
    */
    // double_kw()

    return double_kw() && set( m.p_rule->type, Rule::DOUBLE );
}

bool GrammarParser::float_type()
{
    /* ABNF: 
    float-type       = float-kw
    */
    // float_kw()

    return float_kw() && set( m.p_rule->type, Rule::FLOAT );
}

bool GrammarParser::float_range()
{
    /* ABNF: 
    float-range      = float-min ".." [ float-max ] / ".." float-max
    */
    // float_min() && ".." && [ float_max() ] || ".." && float_max()

    // float_min() && ".." && [ float_max() ] || ".." && float_max()

    cl::accumulator float_min_accumulator( this );
    cl::accumulator_deferred float_max_accumulator( this );

    // No need to record location because always part of primitive_def() rewind choice

    return optional_rewind( float_min() && fixed( ".." ) &&
                    set( m.p_rule->type, Rule::DOUBLE ) &&
                    set( m.p_rule->min, float_min_accumulator.to_float() ) &&
                optional( float_max_accumulator.select() && float_max() &&
                        set( m.p_rule->max, float_max_accumulator.to_float() ) ) ) ||
            optional_rewind( fixed( ".." ) && float_max_accumulator.select() && float_max() &&
                    set( m.p_rule->type, Rule::DOUBLE ) &&
                    set( m.p_rule->max, float_max_accumulator.to_float() ) );
}

bool GrammarParser::float_min()
{
    /* ABNF: 
    float-min        = float
    */
    // float()

    return float_num();
}

bool GrammarParser::float_max()
{
    /* ABNF: 
    float-max        = float
    */
    // float()

    return float_num();
}

bool GrammarParser::float_value()
{
    /* ABNF: 
    float-value      = float
    */
    // float()

    cl::accumulator float_accumulator( this );

    return float_num() &&
            set( m.p_rule->type, Rule::DOUBLE ) &&
            set( m.p_rule->min, float_accumulator.to_float() ) &&
            set( m.p_rule->max, float_accumulator.to_float() );
}

bool GrammarParser::integer_type()
{
    /* ABNF: 
    integer-type     = integer-kw
    */
    // integer_kw()

    return integer_kw() && set( m.p_rule->type, Rule::INTEGER );
}

bool GrammarParser::integer_range()
{
    /* ABNF: 
    integer-range    = integer-min ".." [ integer-max ] / ".." integer-max
    */
    // integer_min() ".." [ integer_max() ] || ".." && integer_max()

    cl::accumulator integer_min_accumulator( this );
    cl::accumulator_deferred integer_max_accumulator( this );

    // No need to record location because always part of primitive_def() rewind choice
    
    if( optional_rewind( integer_min() && fixed( ".." ) && optional( integer_max_accumulator.select() && integer_max() ) ) ||
            optional_rewind( fixed( ".." ) && integer_max_accumulator.select() && integer_max() ) )
    {
        if( (integer_min_accumulator.get().empty() || integer_min_accumulator.get()[0] == '-') ||       // Assume signed if 'min' is absent
                (! integer_max_accumulator.get().empty() && integer_max_accumulator.get()[0] == '-') )  // If 'max' is absent, assume min case will decide if signed
        {
            m.p_rule->type = Rule::INTEGER;
            if( ! integer_min_accumulator.get().empty() && ! integer_max_accumulator.get().empty() )
                (integer_min_accumulator.to_int64() <= integer_max_accumulator.to_int64()) || error( "integer range minimum greater than maximum" );
            if( ! integer_min_accumulator.get().empty() )
                m.p_rule->min = integer_min_accumulator.to_int64();
            if( ! integer_max_accumulator.get().empty() )
                m.p_rule->max = integer_max_accumulator.to_int64();
        }
        else
        {
            m.p_rule->type = Rule::UINTEGER;
            if( ! integer_min_accumulator.get().empty() && ! integer_max_accumulator.get().empty() )
                (integer_min_accumulator.to_uint64() <= integer_max_accumulator.to_uint64()) || error( "integer range minimum greater than maximum" );
            if( ! integer_min_accumulator.get().empty() )
                m.p_rule->min = integer_min_accumulator.to_uint64();
            if( ! integer_max_accumulator.get().empty() )
                m.p_rule->max = integer_max_accumulator.to_uint64();
        }

        return true;
    }

    return false;
}

bool GrammarParser::integer_min()
{
    /* ABNF: 
    integer-min      = integer
    */
    // integer()

    return integer();
}

bool GrammarParser::integer_max()
{
    /* ABNF: 
    integer-max      = integer
    */
    // integer()

    return integer();
}

bool GrammarParser::integer_value()
{
    /* ABNF: 
    integer-value    = integer
    */
    // integer()

    cl::accumulator integer_accumulator( this );

    if( integer() )
    {
        if( integer_accumulator.get()[0] == '-' )
        {
            m.p_rule->type = Rule::INTEGER;
            m.p_rule->min = m.p_rule->max = integer_accumulator.to_int64();
        }
        else
        {
            m.p_rule->type = Rule::UINTEGER;
            m.p_rule->min = m.p_rule->max = integer_accumulator.to_uint64();
        }
        return true;
    }
    
    return false;
}

int64 sized_int_min( int bits )
{
    assert( bits >= 1 && bits <= 64 );

    if( bits <= 0 || bits > 64 )
        return -1;
    uint64 v = ~0;
    v = v << (bits-1);
    return static_cast<int64>(v);
}

int64 sized_int_max( int bits )
{
    return ~sized_int_min( bits );
}

uint64 sized_uint_min( int bits ) // For completeness
{
    return 0;
}

uint64 sized_uint_max( int bits )
{
    return ~(static_cast<uint64>(sized_int_min( bits )) << 1);
}

bool GrammarParser::sized_int_type()
{
    /* ABNF: 
    sized-int-type   = int-kw pos-integer
    */
    // int_kw() && pos_integer()

    cl::accumulator num_bits_accumulator( this );

    return int_kw() && pos_integer() &&
            ( num_bits_accumulator.to_int() <= 64 || error( "sized int size too large") ) &&
            set( m.p_rule->type, Rule::INTEGER ) &&
            set( m.p_rule->min, sized_int_min( num_bits_accumulator.to_int() ) ) && // TODO_TEST_MIN_MAX_SIZED_INTS
            set( m.p_rule->max, sized_int_max( num_bits_accumulator.to_int() ) );
}

bool GrammarParser::sized_uint_type()
{
    /* ABNF: 
    sized-uint-type  = uint-kw pos-integer
    */
    // uint_kw() && pos_integer()

    cl::accumulator num_bits_accumulator( this );

    return int_kw() && pos_integer() &&
            ( num_bits_accumulator.to_int() <= 64 || error( "sized iint size too large") ) &&
            set( m.p_rule->type, Rule::UINTEGER ) &&
            set( m.p_rule->min, sized_uint_min( num_bits_accumulator.to_int() ) ) && // TODO_TEST_MIN_MAX_SIZED_INTS
            set( m.p_rule->max, sized_uint_max( num_bits_accumulator.to_int() ) );
}

bool GrammarParser::ipv4_type()
{
    /* ABNF: 
    ipv4-type        = ipv4-kw
    */
    // ipv4_kw()

    return ipv4_kw() && set( m.p_rule->type, Rule::IPV4 );
}

bool GrammarParser::ipv6_type()
{
    /* ABNF: 
    ipv6-type        = ipv6-kw
    */
    // ipv6_kw()

    return ipv6_kw() && set( m.p_rule->type, Rule::IPV6 );
}

bool GrammarParser::ipaddr_type()
{
    /* ABNF: 
    ipaddr-type      = ipaddr-kw
    */
    // ipaddr_kw()

    return ipaddr_kw() && set( m.p_rule->type, Rule::IPADDR );
}

bool GrammarParser::fqdn_type()
{
    /* ABNF: 
    fqdn-type        = fqdn-kw
    */
    // fqdn_kw()

    return fqdn_kw() &&  set( m.p_rule->type, Rule::FQDN );
}

bool GrammarParser::idn_type()
{
    /* ABNF: 
    idn-type         = idn-kw
    */
    // idn_kw()

    return idn_kw() &&  set( m.p_rule->type, Rule::IDN );
}

bool GrammarParser::uri_range()
{
    /* ABNF: 
    uri-range        = uri-dotdot-kw uri-template
    */
    // uri_dotdot_kw() && uri_template()

    cl::accumulator uri_accumulator( this );

    return uri_dotdot_kw() && uri_template() &&
            set( m.p_rule->type, Rule::URI_RANGE ) &&
            set( m.p_rule->min, uri_accumulator.get() ) &&
            set( m.p_rule->max, uri_accumulator.get() );
}

bool GrammarParser::uri_type()
{
    /* ABNF: 
    uri-type         = uri-kw
    */
    // uri_kw()

    return uri_kw() && set( m.p_rule->type, Rule::URI_TYPE );
}

bool GrammarParser::phone_type()
{
    /* ABNF: 
    phone-type       = phone-kw
    */
    // phone_kw()

    return phone_kw() && set( m.p_rule->type, Rule::PHONE );
}

bool GrammarParser::email_type()
{
    /* ABNF: 
    email-type       = email-kw
    */
    // email_kw()

    return email_kw() && set( m.p_rule->type, Rule::EMAIL );
}

bool GrammarParser::datetime_type()
{
    /* ABNF: 
    datetime-type    = datetime-kw
    */
    // datetime_kw()

    return datetime_kw() && set( m.p_rule->type, Rule::DATETIME );
}

bool GrammarParser::date_type()
{
    /* ABNF: 
    date-type        = date-kw
    */
    // date_kw()

    // full_date_type() = full_date_kw()

    return date_kw() && set( m.p_rule->type, Rule::DATE );
}

bool GrammarParser::time_type()
{
    /* ABNF: 
    time-type        = time-kw
    */
    // time_kw()

    return time_kw() && set( m.p_rule->type, Rule::TIME );
}

bool GrammarParser::hex_type()
{
    /* ABNF: 
    hex-type         = hex-kw
    */
    // hex_kw()

    return hex_kw() && set( m.p_rule->type, Rule::HEX );
}

bool GrammarParser::base32hex_type()
{
    /* ABNF: 
    base32hex-type   = base32hex-kw
    */
    // base32hex_kw()

    return base32hex_kw() && set( m.p_rule->type, Rule::BASE32HEX );
}

bool GrammarParser::base32_type()
{
    /* ABNF: 
    base32-type      = base32-kw
    */
    // base32_kw()

    return base32_kw() && set( m.p_rule->type, Rule::BASE32 );
}

bool GrammarParser::base64url_type()
{
    /* ABNF: 
    base64url-type   = base64url-kw
    */
    // base64url_kw()

    return base64url_kw() && set( m.p_rule->type, Rule::BASE64URL );
}

bool GrammarParser::base64_type()
{
    /* ABNF: 
    base64-type      = base64-kw
    */
    // base64_kw()

    return base64_kw() && set( m.p_rule->type, Rule::BASE64 );
}

bool GrammarParser::any()
{
    /* ABNF: 
    any              = any-kw
    */
    // any_kw()

    return any_kw() && set( m.p_rule->type, Rule::ANY );
}

bool GrammarParser::object_rule()
{
    /* ABNF: 
    object-rule      = annotations "{" *sp-cmt [ object-items *sp-cmt ] "}"
    */
    // annotations() "{" && *sp_cmt() [ object_items() && *sp_cmt() ] "}"

    // No need to record location because array_rule() is always part of a rewound choice

    Annotations object_annotations;

    if( annotations( object_annotations ) && is_get_char( '{' ) )
    {
        m.p_rule->type = Rule::OBJECT;
        m.p_rule->annotations.merge( object_annotations );

        star_sp_cmt() && optional( object_items() ) && star_sp_cmt();

        is_get_char( '}' ) || fatal( "Expected '}' at end of object rule" );

        return true;
    }

    return false;
}

bool GrammarParser::object_items()
{
    /* ABNF: 
    object-items     = object-item (*( sequence-combiner object-item ) /
                   *( choice-combiner object-item ) )
    */
    // object_item() && (*( sequence_combiner() && object_item() ) || *( choice_combiner() && object_item() ) )

    if( object_item() )
    {
        star_sequence_combiner_and_object_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            star_choice_combiner_and_object_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::star_sequence_combiner_and_object_item()
{
    bool is_used = false;

    while( sequence_combiner() )
    {
        is_used = true;
        object_item() || fatal( "Expected object-item after sequence-combiner in object definition" );
    }

    if( is_used && choice_combiner() )
        fatal( "choice-combiner can not be used with sequence-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::star_choice_combiner_and_object_item()
{
    bool is_used = false;

    while( choice_combiner() )
    {
        is_used = true;
        object_item() || fatal( "Expected object-item after choice-combiner in object definition" );
    }

    if( is_used && sequence_combiner() )
        fatal( "sequence-combiner can not be used with choice-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::object_item()
{
    /* ABNF: 
    object-item      = object-item-types *sp-cmt [ repetition ]
    */
    // object_item_types() && *sp_cmt() && [ repetition() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new Rule );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( object_item_types() && star_sp_cmt() && optional( repetition() ) )
    {
        p_parent->append_child_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::object_item_types()
{
    /* ABNF: 
    object-item-types = object-group / member-rule / target-rule-name
    */
    // object_group() || member_rule() || target_rule_name()

    cl::locator loc( this );

    return optional_rewind( object_group() ) ||
            optional_rewind( member_rule() ) ||
            optional_rewind( target_rule_name() );
}

bool GrammarParser::object_group()
{
    /* ABNF: 
    object-group     = "(" *sp-cmt [ object-items *sp-cmt ] ")"
    */
    // "(" && *sp_cmt() [ object_items() && *sp_cmt() ] ")"

    if( is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::OBJECT_GROUP;

        star_sp_cmt() && optional( object_items() ) && star_sp_cmt();
        is_get_char( ')' ) || fatal( "Expected ')' at end of object-group" );

        return true;
    }

    return false;
}

bool GrammarParser::array_rule()
{
    /* ABNF: 
    array-rule       = annotations "[" *sp-cmt [ array-items *sp-cmt ] "]"
    */
    // annotations() "[" && *sp_cmt() [ array_items() && *sp_cmt() ] "]"

    // annotations() "[" && *sp_cmt() [ array_items() && *sp_cmt() ] "]"

    // No need to record location because array_rule() is always part of a rewound choice

    Annotations array_annotations;

    if( annotations( array_annotations ) && is_get_char( '[' ) )
    {
        m.p_rule->type = Rule::ARRAY;
        m.p_rule->annotations.merge( array_annotations );

        star_sp_cmt() && optional( array_items() ) && star_sp_cmt();

        is_get_char( ']' ) || fatal( "Expected ']' at end of array rule" );

        return true;
    }

    return false;
}

bool GrammarParser::array_items()
{
    /* ABNF: 
    array-items      = array-item (*( sequence-combiner array-item ) /
                   *( choice-combiner array-item ) )
    */
    // array_item() && (*( sequence_combiner() && array_item() ) || *( choice_combiner() && array_item() ) )

    if( array_item() )
    {
        star_sequence_combiner_and_array_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            star_choice_combiner_and_array_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::star_sequence_combiner_and_array_item()
{
    bool is_used = false;

    while( sequence_combiner() )
    {
        is_used = true;
        array_item() || fatal( "Expected array-item after sequence-combiner in array definition" );
    }

    if( is_used && choice_combiner() )
        fatal( "choice-combiner can not be used with sequence-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::star_choice_combiner_and_array_item()
{
    bool is_used = false;

    while( choice_combiner() )
    {
        is_used = true;
        array_item() || fatal( "Expected array-item after choice-combiner in array definition" );
    }

    if( is_used && sequence_combiner() )
        fatal( "sequence-combiner can not be used with choice-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::array_item()
{
    /* ABNF: 
    array-item       = array-item-types *sp-cmt [ repetition ]
    */
    // array_item_types() && *sp_cmt() [ repetition() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new Rule );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( array_item_types() && star_sp_cmt() && optional( repetition() ) )
    {
        p_parent->append_child_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::array_item_types()
{
    /* ABNF: 
    array-item-types = array-group / type-rule / explicit-type-choice
    */
    // array_group() || type_rule() || explicit_type_choice()

    cl::locator loc( this );

    return optional_rewind( array_group() ) ||
            optional_rewind( type_rule() ) ||
            optional_rewind( explicit_type_choice() );
}

bool GrammarParser::array_group()
{
    /* ABNF: 
    array-group      = "(" *sp-cmt [ array-items *sp-cmt ] ")"
    */
    // "(" && *sp_cmt() && [ array_items() && *sp_cmt() ] && ")"

    if( is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::ARRAY_GROUP;

        star_sp_cmt() && optional( array_items() ) && star_sp_cmt();
        is_get_char( ')' ) || fatal( "Expected ')' at end of array-group" );

        return true;
    }

    return false;
}

bool GrammarParser::group_rule()
{
    /* ABNF: 
    group-rule       = annotations "(" *sp-cmt [ group-items *sp-cmt ] ")"
    */
    // annotations() "(" && *sp_cmt() [ group_items() && *sp_cmt() ] ")"

    // No need to record location because array_rule() is always part of a rewound choice

    Annotations group_annotations;

    if( annotations( group_annotations ) && is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::GROUP;
        m.p_rule->annotations.merge( group_annotations );

        star_sp_cmt() && optional( group_items() ) && star_sp_cmt();

        is_get_char( ')' ) || fatal( "Expected ')' at end of group rule" );

        return true;
    }

    return false;
}

bool GrammarParser::group_items()
{
    /* ABNF: 
    group-items      = group-item (*( sequence-combiner group-item ) /
                   *( choice-combiner group-item ) )
    */
    // group_item() && (*( sequence_combiner() && group_item() ) || *( choice_combiner() && group_item() ) )

    if( group_item() )
    {
        star_sequence_combiner_and_group_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            star_choice_combiner_and_group_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::star_sequence_combiner_and_group_item()
{
    bool is_used = false;

    while( sequence_combiner() )
    {
        is_used = true;
        group_item() || fatal( "Expected group-item after sequence-combiner in group definition" );
    }

    if( is_used && choice_combiner() )
        fatal( "choice-combiner can not be used with sequence-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::star_choice_combiner_and_group_item()
{
    bool is_used = false;

    while( choice_combiner() )
    {
        is_used = true;
        group_item() || fatal( "Expected group-item after choice-combiner in group definition" );
    }

    if( is_used && sequence_combiner() )
        fatal( "sequence-combiner can not be used with choice-combiner without Parentheses" );

    return is_used;
}

bool GrammarParser::group_item()
{
    /* ABNF: 
    group-item       = group-item-types *sp-cmt [ repetition ]
    */
    // group_item_types() && *sp_cmt() [ repetition() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new Rule );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( group_item_types() && star_sp_cmt() && optional( repetition() ) )
    {
        p_parent->append_child_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::group_item_types()
{
    /* ABNF: 
    group-item-types = group-group / member-rule / type-rule / explicit-type-choice
    */
    // group_group() || member_rule() || type_rule() || explicit_type_choice()

    cl::locator loc( this );

    return optional_rewind( group_group() ) ||
            optional_rewind( member_rule() ) ||
            optional_rewind( type_rule() ) ||
            optional_rewind( explicit_type_choice() );
}

bool GrammarParser::group_group()
{
    /* ABNF: 
    group-group      = group-rule
    */
    // group_rule()

    return group_rule() && set( m.p_rule->type, Rule::GROUP_GROUP );
}

bool GrammarParser::sequence_combiner()
{
    /* ABNF: 
    sequence-combiner = *sp-cmt "," *sp-cmt
    */
    // *sp_cmt() && "," && *sp_cmt()

    return star_sp_cmt() && is_get_char( ',' ) && star_sp_cmt();
}

bool GrammarParser::choice_combiner()
{
    /* ABNF: 
    choice-combiner  = *sp-cmt "|" *sp-cmt
    */
    // *sp_cmt() && "|" && *sp_cmt()

    return star_sp_cmt() && is_get_char( '|' ) && star_sp_cmt();
}

bool GrammarParser::repetition()
{
    /* ABNF: 
    repetition       = "@" *sp-cmt ( optional / one-or-more / min-max-repetition /
                   min-repetition / max-repetition /
                   zero-or-more / specific-repetition )
    */
    // "@" && *sp_cmt() && ( optional() || one_or_more() || min_max_repetition() || min_repetition() || max_repetition() || zero_or_more() || specific_repetition() )

    cl::locator loc_outer( this );

    if( is_get_char( '@' ) && star_sp_cmt() )
    {
        cl::locator loc_inner( this );

        // The order of these routines is important
        if( optional_rewind( optional_marker() ) ||
                optional_rewind( one_or_more() ) ||
                optional_rewind( min_max_repetition() ) ||
                optional_rewind( min_repetition() ) ||
                optional_rewind( max_repetition() ) ||
                optional_rewind( zero_or_more() ) ||
                optional_rewind( specific_repetition() ) )
            return true;   // if false, fall through to loc_outer recorded location
    }
    
    location_top();
    
    return false;
}

bool GrammarParser::optional_marker()
{
    /* ABNF: 
    optional         = "?"
    */
    // "?"

    return is_get_char( '?' ) && set( m.p_rule->repetition.min, 0 ) && set( m.p_rule->repetition.max, 1 );
}

bool GrammarParser::one_or_more()
{
    /* ABNF: 
    one-or-more      = "+" [ repetition-step ]
    */
    // "+" [ repetition_step() ]

    return is_get_char( '+' ) && set( m.p_rule->repetition.min, 1 ) && set( m.p_rule->repetition.max, -1 ) &&
            optional( repetition_step() );
}

bool GrammarParser::zero_or_more()
{
    /* ABNF: 
    zero-or-more     = "*" [ repetition-step ]
    */
    // "*" [ repetition_step() ]

    return is_get_char( '*' ) && set( m.p_rule->repetition.min, 0 ) && set( m.p_rule->repetition.max, -1 ) &&
            optional( repetition_step() );
}

bool GrammarParser::min_max_repetition()
{
    /* ABNF: 
    min-max-repetition = min-repeat ".." max-repeat [ repetition-step ]
    */
    // min_repeat() && ".." && max_repeat() [ repetition_step() ]

    cl::accumulator min_accumulator( this );
    cl::accumulator_deferred max_accumulator( this );

    return min_repeat() && fixed( ".." ) && max_accumulator.select() && max_repeat() &&
            set( m.p_rule->repetition.min, min_accumulator.to_int() ) && set( m.p_rule->repetition.max, max_accumulator.to_int() ) &&
            optional( repetition_step() );
}

bool GrammarParser::min_repetition()
{
    /* ABNF: 
    min-repetition   = min-repeat ".." [ repetition-step ]
    */
    // min_repeat() ".." [ repetition_step() ]

    cl::accumulator min_accumulator( this );

    return min_repeat() && fixed( ".." ) &&
            set( m.p_rule->repetition.min, min_accumulator.to_int() ) && set( m.p_rule->repetition.max, -1 ) &&
            optional( repetition_step() );
}

bool GrammarParser::max_repetition()
{
    /* ABNF: 
    max-repetition   = ".."  max-repeat [ repetition-step ]
    */
    // ".." && max_repeat() [ repetition_step() ]

    cl::accumulator max_accumulator( this );

    return fixed( ".." ) && max_repeat() &&
            set( m.p_rule->repetition.min, 0 ) && set( m.p_rule->repetition.max, max_accumulator.to_int() ) &&
            optional( repetition_step() );
}

bool GrammarParser::min_repeat()
{
    /* ABNF: 
    min-repeat       = non-neg-integer
    */
    // non_neg_integer()

    return non_neg_integer();
}

bool GrammarParser::max_repeat()
{
    /* ABNF: 
    max-repeat       = non-neg-integer
    */
    // non_neg_integer()

    return non_neg_integer();
}

bool GrammarParser::specific_repetition()
{
    /* ABNF: 
    specific-repetition = non-neg-integer
    */
    // non_neg_integer()

    cl::accumulator specific_repetition_accumulator( this );

    return non_neg_integer() &&
            set( m.p_rule->repetition.min, specific_repetition_accumulator.to_int() ) &&
            set( m.p_rule->repetition.max, specific_repetition_accumulator.to_int() );
}

bool GrammarParser::repetition_step()
{
    /* ABNF: 
    repetition-step  = "%" non-neg-integer
    */
    // "%" && non_neg_integer()

    if( is_get_char( '%' ) )
    {
        cl::accumulator repetition_accumulator( this );
        
        non_neg_integer() && set( m.p_rule->repetition.step, repetition_accumulator.to_int() ) || fatal( "Expected repetition step size after '%'" );

        return true;
    }

    return false;
}

bool GrammarParser::integer()
{
    /* ABNF: 
    integer          = "0" / ["-"] pos-integer
    */
    // "0" / ["-"] && pos_integer()

    return (zero() && (peek_is_in( cl::alphabet_not( cl::alphabet_digit() ) ) || error( "Leading zeros not allow on integers" ) ) ) ||
            optional( minus() ) && pos_integer();
}

bool GrammarParser::non_neg_integer()
{
    /* ABNF: 
    non-neg-integer  = "0" / pos-integer
    */
    // "0" || pos_integer()

    return (zero() && (peek_is_in( cl::alphabet_not( cl::alphabet_digit() ) ) || error( "Leading zeros not allow on integers" ) ) ) ||
            pos_integer();
}

bool GrammarParser::pos_integer()
{
    /* ABNF: 
    pos-integer      = digit1-9 *DIGIT
    */
    // digit1_9() && *DIGIT()

    return digit1_9() && star_DIGIT();
}

bool GrammarParser::float_num()
{
    /* ABNF: 
    float            = [ minus ] int frac [ exp ]
    */
    // [ minus() ] && int() && frac() [ exp() ]

    cl::locator loc( this );

    return optional( minus() ) && int_num() && frac() && optional( exp() ) || location_top( false );
}

bool GrammarParser::minus()
{
    /* ABNF: 
    minus            = %x2D                          ; -
    */
    // %x2D                          ; -

    return accumulate( '-' );
}

bool GrammarParser::plus()
{
    /* ABNF: 
    plus             = %x2B                          ; +
    */
    // %x2B                          ; +

    return accumulate( '+' );
}

bool GrammarParser::int_num()
{
    /* ABNF: 
    int              = zero / ( digit1-9 *DIGIT )
    */
    // zero() || ( digit1_9() && *DIGIT() )

    return zero() || ( digit1_9() && star_DIGIT() );
}

cl::alphabet_char_class digit1_9_alphabet( "1-9" );

bool GrammarParser::digit1_9()
{
    /* ABNF: 
    digit1-9         = %x31-39                       ; 1-9
    */
    // %x31-39                       ; 1-9

    return accumulate( digit1_9_alphabet );
}

bool GrammarParser::frac()
{
    /* ABNF: 
    frac             = decimal-point 1*DIGIT
    */
    // decimal_point() && 1*DIGIT()

    return decimal_point() && one_star_DIGIT();
}

bool GrammarParser::decimal_point()
{
    /* ABNF: 
    decimal-point    = %x2E                          ; .
    */
    // %x2E                          ; .

    return accumulate( '.' );
}

bool GrammarParser::exp()
{
    /* ABNF: 
    exp              = e [ minus / plus ] 1*DIGIT
    */
    // e() [ minus() || plus() ] && 1*DIGIT()

    return e() && optional( minus() || plus() ) && one_star_DIGIT();
}

bool GrammarParser::e()
{
    /* ABNF: 
    e                = %x65 / %x45                   ; e E
    */
    // %x65 / %x45                   ; e() && E

    return accumulate( 'e' ) || accumulate( 'E' );
}

bool GrammarParser::zero()
{
    /* ABNF: 
    zero             = %x30                          ; 0
    */
    // %x30                          ; 0

    return accumulate( '0' );
}

bool GrammarParser::q_string_as_utf8()
{
    if( is_get_char( '"' ) )
    {
        std::string utf8_string;

        return get_qstring_contents( &utf8_string ) && is_get_char( '"' ) &&
                accumulator_append( utf8_string ) || fatal( "Badly formed QString" );
    }

    return false;
}

bool GrammarParser::q_string()
{
    /* ABNF: 
    q-string         = quotation-mark *char quotation-mark 
    */
    // quotation_mark() && *qs_char() && quotation_mark()

    if( quotation_mark() )
    {
        star_qs_char() && quotation_mark() || fatal( "Badly formed QString" );

        return true;
    }

    return false;
}

bool GrammarParser::quotation_mark()
{
    /* ABNF: 
    quotation-mark   = %x22      ; "
    */
    // %x22      ; "

    return accumulate( '"' );
}

bool GrammarParser::qs_char()
{
    /* ABNF: 
    char             = unescaped /
                   escape (
                   %x22 /          ; "    quotation mark  U+0022
                   %x5C /          ; \    reverse solidus U+005C
                   %x2F /          ; /    solidus         U+002F
                   %x62 /          ; b    backspace       U+0008
                   %x66 /          ; f    form feed       U+000C
                   %x6E /          ; n    line feed       U+000A
                   %x72 /          ; r    carriage return U+000D
                   %x74 /          ; t    tab             U+0009
                   %x75 4HEXDIG )  ; uXXXX                U+XXXX
    */
    // unescaped() || escape() && (
    //                    %x22 /          ; " && quotation mark  U+0022
    //                    %x5C /          ; \    reverse solidus U+005C
    //                    %x2F /          ; /    solidus         U+002F
    //                    %x62 /          ; b    backspace       U+0008
    //                    %x66 /          ; f    form feed       U+000C
    //                    %x6E /          ; n    line feed       U+000A
    //                    %x72 /          ; r    carriage return U+000D
    //                    %x74 /          ; t    tab             U+0009
    //                    %x75 4HEXDIG )  ; uXXXX                U+XXXX

    return unescaped() || escape() && (escaped_code() || u() && four_HEXDIG());
}

bool is_qstring_unescaped( char c )
{
    // unescaped        = %x20-21 / %x23-5B / %x5D-10FFFF

    return c >= 0x20 && c <= 0x21 || c >= 0x23 && c <= 0x5b || c >= 0x5d;
}

bool GrammarParser::unescaped()
{
    /* ABNF: 
    unescaped        = %x20-21 / %x23-5B / %x5D-10FFFF
    */
    // %x20-21 / %x23-5B / %x5D-10FFFF

    return accumulate( cl::alphabet_function( is_qstring_unescaped ) );
}

bool GrammarParser::escape()
{
    /* ABNF: 
    escape           = %x5C              ; \
    */
    // %x5C              ; \

    return accumulate( '\\' );
}

cl::alphabet_char_class escaped_code_alphabet( "\"\\/bfnrt" );

bool GrammarParser::escaped_code()
{
    return accumulate( escaped_code_alphabet );
}

bool GrammarParser::u()
{
    return accumulate( 'u' );
}

bool GrammarParser::four_HEXDIG()
{
    for( size_t i=0; i<4; ++i )
        if( ! HEXDIG() )
            return false;
    return true;
}

bool GrammarParser::regex()
{
    /* ABNF: 
    regex            = "/" *( escape "/" / not-slash ) "/" [ regex-modifiers ]
    */
    // "/" && *( escape() "/" || not_slash() ) "/" [ regex_modifiers() ]

    if( accumulate( '/' ) )
    {
        while( (escape() && accumulate( '/' )) || not_slash() )
        {}
        accumulate( '/' ) && optional( regex_modifiers() ) || fatal( "Error reading regular expression" );

        return true;
    }

    return false;
}

bool is_not_slash( char c )
{
    // not_slash() = HTAB() || CR() || LF() / %x20-2E / %x30-10FFFF

    return c == '\t' || c == '\r' || c == '\n' ||
            c >= 0x20 && c <= 0x2e || c >= 0x30;
}

bool GrammarParser::not_slash()
{
    /* ABNF: 
    not-slash        = HTAB / CR / LF / %x20-2E / %x30-10FFFF
    */
    // HTAB() || CR() || LF() / %x20-2E / %x30-10FFFF

    return accumulate( cl::alphabet_function( is_not_slash ) );
}

cl::alphabet_char_class regex_modifiers_alphabet( "isx" );

bool GrammarParser::regex_modifiers()
{
    /* ABNF: 
    regex-modifiers  = *( "i" / "s" / "x" )
    */
    // *( "i" || "s" || "x" )

    return accumulate( regex_modifiers_alphabet );
}

bool GrammarParser::uri_template()
{
    /* ABNF: 
    uri-template     = 1*ALPHA ":" 1*not-space
    */
    // 1*ALPHA() && ":" && 1*not_space()

    return one_star_ALPHA() && accumulate( ':' ) && one_star_not_space();
}

bool GrammarParser::any_kw()
{
    /* ABNF: 
    any-kw           = %x61.6E.79                      ; "any"
    */
    // %x61.6E.79                      ; "any"

    return fixed( "any" );
}

bool GrammarParser::as_kw()
{
    /* ABNF: 
    as-kw            = %x61.73                         ; "as"
    */
    // %x61.73                         ; "as"

    return fixed( "as" );
}

bool GrammarParser::base32_kw()
{
    /* ABNF: 
    base32-kw        = %x62.61.73.65.33.32             ; "base32"
    */
    // %x62.61.73.65.33.32             ; "base32"

    return fixed( "base32" );
}

bool GrammarParser::base32hex_kw()
{
    /* ABNF: 
    base32hex-kw     = %x62.61.73.65.33.32.68.65.78    ; "base32hex"
    */
    // %x62.61.73.65.33.32.68.65.78    ; "base32hex"

    return fixed( "base32hex" );
}

bool GrammarParser::base64_kw()
{
    /* ABNF: 
    base64-kw        = %x62.61.73.65.36.34             ; "base64"
    */
    // %x62.61.73.65.36.34             ; "base64"

    return fixed( "base64" );
}

bool GrammarParser::base64url_kw()
{
    /* ABNF: 
    base64url-kw     = %x62.61.73.65.36.34.75.72.6C    ; "base64url"
    */
    // %x62.61.73.65.36.34.75.72.6C    ; "base64url"

    return fixed( "base64url" );
}

bool GrammarParser::boolean_kw()
{
    /* ABNF: 
    boolean-kw       = %x62.6F.6F.6C.65.61.6E          ; "boolean"
    */
    // %x62.6F.6F.6C.65.61.6E          ; "boolean"

    return fixed( "boolean" );
}

bool GrammarParser::date_kw()
{
    /* ABNF: 
    date-kw          = %x64.61.74.65                   ; "date"
    */
    // %x64.61.74.65                   ; "date"

    return fixed( "date" );
}

bool GrammarParser::datetime_kw()
{
    /* ABNF: 
    datetime-kw      = %x64.61.74.65.74.69.6D.65       ; "datetime"
    */
    // %x64.61.74.65.74.69.6D.65       ; "datetime"

    return fixed( "datetime" );
}

bool GrammarParser::double_kw()
{
    /* ABNF: 
    double-kw        = %x64.6F.75.62.6C.65             ; "double"
    */
    // %x64.6F.75.62.6C.65             ; "double"

    return fixed( "double" );
}

bool GrammarParser::email_kw()
{
    /* ABNF: 
    email-kw         = %x65.6D.61.69.6C                ; "email"
    */
    // %x65.6D.61.69.6C                ; "email"

    return fixed( "email" );
}

bool GrammarParser::false_kw()
{
    /* ABNF: 
    false-kw         = %x66.61.6C.73.65                ; "false"
    */
    // %x66.61.6C.73.65                ; "false"

    return fixed( "false" );
}

bool GrammarParser::float_kw()
{
    /* ABNF: 
    float-kw         = %x66.6C.6F.61.74                ; "float"
    */
    // %x66.6C.6F.61.74                ; "float()"

    return fixed( "float" );
}

bool GrammarParser::fqdn_kw()
{
    /* ABNF: 
    fqdn-kw          = %x66.71.64.6E                   ; "fqdn"
    */
    // %x66.71.64.6E                   ; "fqdn"

    return fixed( "fqdn" );
}

bool GrammarParser::hex_kw()
{
    /* ABNF: 
    hex-kw           = %x68.65.78                      ; "hex"
    */
    // %x68.65.78                      ; "hex"

    return fixed( "hex" );
}

bool GrammarParser::idn_kw()
{
    /* ABNF: 
    idn-kw           = %x69.64.6E                      ; "idn"
    */
    // %x69.64.6E                      ; "idn"

    return fixed( "idn" );
}

bool GrammarParser::import_kw()
{
    /* ABNF: 
    import-kw        = %x69.6D.70.6F.72.74             ; "import"
    */
    // %x69.6D.70.6F.72.74             ; "import"

    return fixed( "import" );
}

bool GrammarParser::int_kw()
{
    /* ABNF: 
    int-kw           = %x69.6E.74                      ; "int"
    */
    // %x69.6E.74                      ; "int"

    return fixed( "int" );
}

bool GrammarParser::integer_kw()
{
    /* ABNF: 
    integer-kw       = %x69.6E.74.65.67.65.72          ; "integer"
    */
    // %x69.6E.74.65.67.65.72          ; "integer()"

    return fixed( "integer" );
}

bool GrammarParser::ipaddr_kw()
{
    /* ABNF: 
    ipaddr-kw        = %x69.70.61.64.64.72             ; "ipaddr"
    */
    // %x69.70.61.64.64.72             ; "ipaddr"

    return fixed( "ipaddr" );
}

bool GrammarParser::ipv4_kw()
{
    /* ABNF: 
    ipv4-kw          = %x69.70.76.34                   ; "ipv4"
    */
    // %x69.70.76.34                   ; "ipv4"

    return fixed( "ipv4" );
}

bool GrammarParser::ipv6_kw()
{
    /* ABNF: 
    ipv6-kw          = %x69.70.76.36                   ; "ipv6"
    */
    // %x69.70.76.36                   ; "ipv6"

    return fixed( "ipv6" );
}

bool GrammarParser::jcr_version_kw()
{
    /* ABNF: 
    jcr-version-kw   = %x6A.63.72.2D.76.65.72.73.69.6F.6E ; "jcr-version"
    */
    // %x6A.63.72.2D.76.65.72.73.69.6F.6E ; "jcr-version"

    return fixed( "jcr-version" );
}

bool GrammarParser::not_kw()
{
    /* ABNF: 
    not-kw           = %x6E.6F.74                      ; "not"
    */
    // %x6E.6F.74                      ; "not"

    return fixed( "not" );
}

bool GrammarParser::null_kw()
{
    /* ABNF: 
    null-kw          = %x6E.75.6C.6C                   ; "null"
    */
    // %x6E.75.6C.6C                   ; "null"

    return fixed( "null" );
}

bool GrammarParser::phone_kw()
{
    /* ABNF: 
    phone-kw         = %x70.68.6F.6E.65                ; "phone"
    */
    // %x70.68.6F.6E.65                ; "phone"

    return fixed( "phone" );
}

bool GrammarParser::root_kw()
{
    /* ABNF: 
    root-kw          = %x72.6F.6F.74                   ; "root"
    */
    // %x72.6F.6F.74                   ; "root"

    return fixed( "root" );
}

bool GrammarParser::ruleset_id_kw()
{
    /* ABNF: 
    ruleset-id-kw    = %x72.75.6C.65.73.65.74.2D.69.64 ; "ruleset-id"
    */
    // %x72.75.6C.65.73.65.74.2D.69.64 ; "ruleset_id()"

    return fixed( "ruleset-id" );
}

bool GrammarParser::string_kw()
{
    /* ABNF: 
    string-kw        = %x73.74.72.69.6E.67             ; "string"
    */
    // %x73.74.72.69.6E.67             ; "string"

    return fixed( "string" );
}

bool GrammarParser::time_kw()
{
    /* ABNF: 
    time-kw          = %x74.69.6D.65                   ; "time"
    */
    // %x74.69.6D.65                   ; "time"

    return fixed( "time" );
}

bool GrammarParser::true_kw()
{
    /* ABNF: 
    true-kw          = %x74.72.75.65                   ; "true"
    */
    // %x74.72.75.65                   ; "true"

    return fixed( "true" );
}

bool GrammarParser::type_kw()
{
    /* ABNF: 
    type-kw          = %x74.79.70.65                   ; "type"
    */
    // %x74.79.70.65                   ; "type"

    return fixed( "type" );
}

bool GrammarParser::uint_kw()
{
    /* ABNF: 
    uint-kw          = %x75.69.6E.74                   ; "uint"
    */
    // %x75.69.6E.74                   ; "uint"

    return fixed( "uint" );
}

bool GrammarParser::unordered_kw()
{
    /* ABNF: 
    unordered-kw     = %x75.6E.6F.72.64.65.72.65.64    ; "unordered"
    */
    // %x75.6E.6F.72.64.65.72.65.64    ; "unordered"

    return fixed( "unordered" );
}

bool GrammarParser::uri_dotdot_kw()
{
    /* ABNF: 
    uri-dotdot-kw    = %x75.72.69.2E.2E                ; "uri.."
    */
    // %x75.72.69.2E.2E                ; "uri.."

    return fixed( "uri.." );
}

bool GrammarParser::uri_kw()
{
    /* ABNF: 
    uri-kw           = %x75.72.69                      ; "uri"
    */
    // %x75.72.69                      ; "uri"

    return fixed( "uri" );
}

bool GrammarParser::ALPHA()
{
    /* ABNF: 
    ALPHA            = %x41-5A / %x61-7A   ; A-Z / a-z
    */
    // %x41-5A / %x61-7A   ; A-Z / a-z

    return accumulate( cl::alphabet_alpha() );
}

bool GrammarParser::CR()
{
    /* ABNF: 
    CR               = %x0D         ; carriage return
    */
    // %x0D         ; carriage return

    return is_get_char( '\r' );
}

bool GrammarParser::DIGIT()
{
    /* ABNF: 
    DIGIT            = %x30-39      ; 0-9
    */
    // %x30-39      ; 0-9

    return accumulate( cl::alphabet_digit() );
}

bool GrammarParser::HEXDIG()
{
    /* ABNF: 
    HEXDIG           = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
    */
    // DIGIT() || "A" || "B" || "C" || "D" || "E" || "F"

    return accumulate( cl::alphabet_hex() );
}

bool GrammarParser::HTAB()
{
    /* ABNF: 
    HTAB             = %x09         ; horizontal tab
    */
    // %x09         ; horizontal tab

    return is_get_char( '\t' );
}

bool GrammarParser::LF()
{
    /* ABNF: 
    LF               = %x0A         ; linefeed
    */
    // %x0A         ; linefeed

    return is_get_char( '\x0a' );
}

bool GrammarParser::SP()
{
    /* ABNF: 
    SP               = %x20         ; space
    */
    // %x20         ; space

    return is_get_char( ' ' );
}

bool GrammarParser::WSP()
{
    /* ABNF: 
    WSP              = SP / HTAB    ; white space
    */
    // SP() || HTAB()    ; white space

    return SP() || HTAB();
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
