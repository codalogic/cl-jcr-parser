//----------------------------------------------------------------------------
// Copyright (c) 2015-2018, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

// Implements jcr-abnf - 2018-03-29

//----------------------------------------------------------------------------
// Notes:
//      Leave_as_warning :
//              Leave these as warnings to facilitate unit testing.  Also, if
//              they are important, other errors are likely to show up also.
//----------------------------------------------------------------------------

#include "cl-jcr-parser/parser.h"

#include "dsl-pa/dsl-pa.h"
#include "cl-utils/str-args.h"

#if defined(_MSC_VER)
// Require error when nonstandard extension used :
//      'token' : conversion from 'type' to 'type'
//      e.g. disallow foo( /*const*/ foo & ) for copy constructor
//      and           foo( /*const*/ bar() ) for bar object constructed in-situ
#pragma warning(error: 4239)
#endif

#include <iostream>
#include <cassert>
#include <algorithm>

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
    return major == "0" &&
            (minor == "9");
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
        JCRParser * p_jcr_parser;
        GrammarSet * p_grammar_set;
        Grammar * p_grammar;
        cl::reader & r_reader;
        bool is_errored;
        JCRParser::Status status;
        bool is_infer_types;

        Rule * p_rule;

        Members(
            JCRParser * p_jcr_parser_in,
            cl::reader & r_reader_in,
            GrammarSet * p_grammar_set_in,
            Grammar * p_grammar_in )
            :
            p_jcr_parser( p_jcr_parser_in ),
            p_grammar_set( p_grammar_set_in ),
            p_grammar( p_grammar_in ),
            r_reader( r_reader_in ),
            is_errored( false ),
            status( JCRParser::S_OK ),
            is_infer_types( false ),
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
    GrammarParser( JCRParser * p_jcr_parser, cl::reader & r_reader, GrammarSet * p_grammar_set, Grammar * p_grammar );
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
    struct DirectiveForm { enum Enum { one_line, multi_line }; };
    bool one_line_directive();
    bool multi_line_directive();
    bool directive_def( DirectiveForm::Enum form );
    bool jcr_version_d( DirectiveForm::Enum form );
    bool DSPs( DirectiveForm::Enum form );
    bool major_version();
    bool minor_version();
    bool extension_id();
    bool id();
    bool id_tail();
    STAR( id_tail )
    bool ruleset_id_d( DirectiveForm::Enum form );
    bool import_d( DirectiveForm::Enum form );
    bool ruleset_id();
    bool ruleset_id_alias();
    bool infer_types_d();
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
    bool target_rule_name_reader( TargetRule & r_target_rule );
    bool name();
    bool rule_def();
    bool type_designator();
    bool rule_def_type_rule();
    bool value_rule();
    bool member_rule_or_string_type_ambiguity_hack();
    bool member_rule();
    bool member_name_spec();
    bool convert_member_name_to_string_type();
    bool type_rule();
    bool type_choice();
    bool type_choice_items();
    bool annotations( Annotations & );
    bool annotation_set( Annotations & );
    bool not_annotation( Annotations & );
    bool unordered_annotation( Annotations & );
    bool root_annotation( Annotations & );
    bool exclude_min_annotation( Annotations & );
    bool exclude_max_annotation( Annotations & );
    bool default_annotation( Annotations & );
    bool primitive_value();
    bool format_annotation( Annotations & );
    bool choice_annotation( Annotations & );
    bool augments_annotation( Annotations & );
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
    bool one_star_sequence_combiner_and_object_item();
    bool one_star_choice_combiner_and_object_item();
    bool object_item();
    bool object_item_types();
    bool object_group();
    bool array_rule();
    bool array_items();
    bool one_star_sequence_combiner_and_array_item();
    bool one_star_choice_combiner_and_array_item();
    bool array_item();
    bool array_item_types();
    bool array_group();
    bool group_rule();
    bool group_items();
    bool one_star_sequence_combiner_and_group_item();
    bool one_star_choice_combiner_and_group_item();
    bool group_item();
    bool group_item_types();
    bool group_group();
    bool sequence_combiner();
    bool choice_combiner();
    bool repetition();
    bool optional_marker();
    bool one_or_more();
    bool zero_or_more();
    bool repetition_range();
    bool min_max_repetition();
    bool min_repetition();
    bool min_repeat();
    bool max_repeat();
    bool specific_repetition();
    bool repetition_step();
    bool step_size();
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
    bool escape_code();
    bool u();
    bool four_HEXDIG();
    bool quotation_mark();
    bool unescaped();
    bool regex();
    bool re_escape_code();
    bool not_slash();
    bool regex_modifiers();
    bool uri_scheme();
    bool any_kw();
    bool as_kw();
    bool augments_kw();
    bool base32_kw();
    bool base32hex_kw();
    bool base64_kw();
    bool base64url_kw();
    bool boolean_kw();
    bool choice_kw();
    bool date_kw();
    bool datetime_kw();
    bool default_kw();
    bool double_kw();
    bool email_kw();
    bool exclude_max_kw();
    bool exclude_min_kw();
    bool false_kw();
    bool float_kw();
    bool format_kw();
    bool fqdn_kw();
    bool hex_kw();
    bool idn_kw();
    bool import_kw();
    bool infer_types_kw();
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

    Rule * new_rule() { return new Rule( m.p_grammar, m.r_reader.get_line_number(), m.r_reader.get_column_number() ); }

    std::string error_token();

    bool warning( const char * p_message )
    {
        report( Severity::WARNING, p_message );
        return true;
    }
    bool warning( const char * p_format, const clutils::str_args & r_arg_1 )
    {
        return warning( expand( p_format, r_arg_1 ).c_str() );
    }
    bool warning( const char * p_format, const clutils::str_args & r_arg_1, const clutils::str_args & r_arg_2 )
    {
        return warning( expand( p_format, r_arg_1, r_arg_2 ).c_str() );
    }
    bool error( const char * p_message )
    {
        report( Severity::ERROR, p_message );
        m.is_errored = true;
        return true;    // Return 'true', because if we are throwing an error it suggests we're on the right parse path, but have an invalid token.  And we'd usually want to recover from this with an && clause.
    }
    bool error( const char * p_format, const clutils::str_args & r_arg_1 )
    {
        return error( expand( p_format, r_arg_1 ).c_str() );
    }
    bool error( const char * p_format, const clutils::str_args & r_arg_1, const clutils::str_args & r_arg_2 )
    {
        return error( expand( p_format, r_arg_1, r_arg_2 ).c_str() );
    }
    bool fatal( const char * p_message )
    {
        report( Severity::FATAL, p_message );
        m.is_errored = true;
        throw GrammarParserFatalError();
        return false;
    }
    bool fatal( const char * p_format, const clutils::str_args & r_arg_1 )
    {
        return fatal( expand( p_format, r_arg_1 ).c_str() );
    }
    bool fatal( const char * p_format, const clutils::str_args & r_arg_1, const clutils::str_args & r_arg_2 )
    {
        return fatal( expand( p_format, r_arg_1, r_arg_2 ).c_str() );
    }

    void report( Severity severity, const char * p_message )
    {
        m.p_jcr_parser->report( m.p_grammar->jcr_source, m.r_reader.get_line_number(), m.r_reader.get_column_number(), severity, p_message );
    }

    bool recover_to_eol()
    {
        while( is_get_char_in( cl::alphabet_not( cl::alphabet_eol() ) ) )
        {}
        get();  // Get the end of line character
        return true;
    }
    bool recover_to( char c )
    {
        while( skip_until( cl::alphabet_char( c ) ) )
        {}
        get();  // Chew the target character
        return true;
    }
    bool recover_to_before( char c )
    {
        while( skip_until( cl::alphabet_char( c ) ) )
        {}
        return true;
    }
    bool abandon_path()
    {
        return false;
    }
};

GrammarParser::GrammarParser(
                        JCRParser * p_jcr_parser,
                        cl::reader & r_reader,
                        GrammarSet * p_grammar_set,
                        Grammar * p_grammar )
    :
    cl::dsl_pa( r_reader ),
    m( p_jcr_parser, r_reader, p_grammar_set, p_grammar )
{}

class UnspecifiedRetreat : public std::exception {};
class SurrenderRetreat : public UnspecifiedRetreat {};
class TopLevelRetreat : public UnspecifiedRetreat {};

bool GrammarParser::parse()
{
    if( ! jcr() || m.is_errored )
        m.status = JCRParser::S_ERROR;
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
        return is_peek_at_end() || fatal( "Expected start of <directive> or <rule>. Got: '%0'", error_token() );
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
    comment          = ";" *comment-char comment-end-char
    */
    // ";" && *comment_char() && comment_end_char()

    if( is_get_char( ';' ) )
    {
        while( comment_char() )
        {}
        return comment_end_char();
    }
    return false;
}

bool is_jcr_comment_char( char c )
{
    // comment_char() = HTAB() / %x20-10FFFF

    return c == '\t' || c >= 0x20 || ((c & 0x80) != 0);
}

bool GrammarParser::comment_char()
{
    /* ABNF:
    comment-char     = HTAB / %x20-10FFFF
    */
    // HTAB() / %x20-10FFFF

    return is_get_char_in( cl::alphabet_function( is_jcr_comment_char ) );
}

bool GrammarParser::comment_end_char()
{
    /* ABNF:
    comment-end-char = CR / LF
    */
    // CR() || LF()

    return is_get_char_in( cl::alphabet_char_class( "\r\n" ) ) || is_peek_at_end();
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

        return rewind_on_reject( one_line_directive() ) ||
                rewind_on_reject( multi_line_directive() ) ||
                error( "Invalid #directive format" );
    }

    return false;
}

bool GrammarParser::one_line_directive()
{
    /* ABNF:
    one-line-directive = *WSP()
                   (directive-def / one-line-tbd-directive-d) *WSP eol
    */
    // *WSP() && (directive_def() || one_line_tbd_directive_d()) && *WSP() && eol()

    if( star_WSP() && (directive_def( DirectiveForm::one_line ) || one_line_tbd_directive_d()) )
    {
        // Use is_peek_at_end() to allow ruleset to end with a directive that doesn't have newline at end
        star_WSP() &&
            (eol() || is_peek_at_end() ||
                error( "Unexpected additional material in directive. Got: '%0'", error_token() ) && recover_to_eol());

        return true;
    }

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
        bool is_parse_complete = false;

        star_sp_cmt() &&
            (directive_def( DirectiveForm::multi_line )
                || multi_line_tbd_directive_d()
                || end_path_with( error( "Unable to read multi-line #{directive}. Got: '%0'", error_token() ) )
                ) &&
            star_sp_cmt() &&
            (is_get_char( '}' )
                || (is_current_at_end() && error( "Unexpected end of file in multi-line #{directive}" )
                    || end_path_with( error( "Unexpected additional material in multi-line #{directive}. Got: '%0'", error_token() ) ) )
                ) &&
            set( is_parse_complete, true );

        if( ! is_parse_complete )
            recover_to( '}' );

        return true;
    }

    return false;
}

bool GrammarParser::directive_def( DirectiveForm::Enum form )
{
    /* ABNF:
    directive-def    = jcr-version-d / ruleset-id-d / import-d / infer-types-d
    */
    // jcr_version_d() || ruleset_id_d() || import_d() || infer_types_d()

    cl::locator loc( this );

    return rewind_on_reject( jcr_version_d( form ) ) ||
            rewind_on_reject( ruleset_id_d( form ) ) ||
            rewind_on_reject( import_d( form ) ) ||
            rewind_on_reject( infer_types_d() );
}

bool GrammarParser::jcr_version_d( DirectiveForm::Enum form )
{
    /* ABNF:
    jcr-version-d    = jcr-version-kw DSPs major-version "." minor-version
                   *( DSPs "+" [ DSPs ] extension-id )
    */
    // jcr_version_kw() && DSPs() && major_version() "." && minor_version() && *( DSPs() "+" [ DSPs() ] && extension_id() )

    cl::accumulator_deferred major_version_accumulator( this );
    cl::accumulator_deferred minor_version_accumulator( this );

    if( jcr_version_kw() )
    {
        if( (DSPs( form ) || fatal( "Expected spaces and major.minor version after <jcr-version> keyword in #jcr-directive. Got '%0'", error_token() ) ) &&
                major_version_accumulator.select() &&
                (major_version() || fatal( "Expected <major-version> in #jcr-directive. Got '%0'", error_token() ) ) &&
                (is_get_char( '.' ) || fatal( "Expected '.' after <major-version> in #jcr-directive. Got '%0'", error_token() ) ) &&
                minor_version_accumulator.select() &&
                (minor_version() || fatal( "Expected <minor-version> after '.' in #jcr-directive. Got '%0'", error_token() ) ) )
        {
            std::string major_number = major_version_accumulator.get();
            std::string minor_number = minor_version_accumulator.get();

            if( ! is_supported_jcr_version( major_number, minor_number ) )
                error( "Unsupported JCR version in #jcr-directive. Got: '%0.%1'", major_number, minor_number );

            cl::accumulator extension_accumulator( this );
            while( extension_accumulator.clear() && DSPs( form ) && is_get_char( '+' ) &&
                    optional( DSPs( form ) ) &&
                    (extension_id() || end_path_with( error( "Expected <extension-id> after '+' in #jcr-directive. Got: '%0'", error_token() ))) )
            {
                warning( "Unknown <extension-id> in #jcr-directive. Got: '%0'", extension_accumulator.get() ); // See Leave_as_warning
            }
        }

        return true;
    }

    return false;
}

bool GrammarParser::DSPs( DirectiveForm::Enum form )  // "Directive spaces" - May later include a flag to test if in one-line or multi-line directive
{
    return form == DirectiveForm::one_line ? one_star_WSP() : one_star_sp_cmt();
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

bool GrammarParser::extension_id()
{
    /* ABNF:
    extension-id     = id
    */
    // id()

    return id();
}

bool GrammarParser::id()
{
    /* ABNF:
    id               = ALPHA *id-tail
    */
    // ALPHA() && *id_tail()

    return ALPHA() && star_id_tail();
}

bool GrammarParser::id_tail()
{
    /* ABNF:
    id-tail          = %x21-7C / %x7E-10FFFF ; not spaces, not }
    */
    // %x21-7C / %x7E-10FFFF ; not spaces(), not }

    return accumulate( cl::alphabet_not( cl::alphabet_or( cl::alphabet_space(), cl::alphabet_char( '}' ) ) ) );
}

bool GrammarParser::ruleset_id_d( DirectiveForm::Enum form )
{
    /* ABNF:
    ruleset-id-d     = ruleset-id-kw DSPs ruleset-id
    */
    // ruleset_id_kw() && DSPs() && ruleset_id()

    if( ruleset_id_kw() )
    {
        cl::accumulator ruleset_id_accumulator( this );

        if( (DSPs( form ) && ruleset_id() )
            || fatal( "Unable to read <ruleset-id> in #ruleset-id directive. Got '%0'", error_token() ) )
        {
            m.p_grammar->ruleset_id = ruleset_id_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::import_d( DirectiveForm::Enum form )
{
    /* ABNF:
    import-d         = import-kw DSPs ruleset-id
                   [ DSPs as-kw DSPs ruleset-id-alias ]
    */
    // import_kw() && DSPs() && ruleset_id()
    //                    [ DSPs() && as_kw() && DSPs() && ruleset_id_alias() ]

    if( import_kw() )
    {
        cl::accumulator_deferred ruleset_id_accumulator( this );
        cl::accumulator_deferred ruleset_id_alias_accumulator( this );

        if( (DSPs( form ) || fatal( "Expected space and <ruleset-id> after #import directive keyword. Got '%0'", error_token() )) &&
            (ruleset_id_accumulator.select() && ruleset_id() || fatal( "Unable to read <ruleset-id> in #import directive. Got '%0'", error_token() )) &&
            optional(
                DSPs( form ) &&
                as_kw() &&
                (DSPs( form ) || fatal( "Expected space and <ruleset-id-alias> after 'as' keyword in #import directive. Got '%0'", error_token() )) &&
                ((ruleset_id_alias_accumulator.select() && ruleset_id_alias()) || (fatal( "Unable to read <ruleset-id-alias> after 'as' keyword in #import directive. Got '%0'", error_token() ))) ) )
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
    ruleset-id       = id
    */
    // id()

    return id();
}

bool GrammarParser::ruleset_id_alias()
{
    /* ABNF:
    ruleset-id-alias = name
    */
    // name()

    return name();
}

bool GrammarParser::infer_types_d()
{
    /* ABNF: 
    infer-types-d    = infer-types-kw
    */
    // infer_types_kw()

    if( infer_types_kw() )
    {
        m.is_infer_types = true;

        return true;
    }

    return false;
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
        optional( one_star_WSP() && tbd_directive_parameters_accumulator.select() && one_line_directive_parameters() ) )
    {
        warning( "Unknown #directive '%0', with parameters '%1'",
                tbd_directive_name_accumulator.get(), tbd_directive_parameters_accumulator.get() );  // See Leave_as_warning
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
    /* ABNF:
    multi-line-tbd-directive-d = directive-name
                   [ 1*sp-cmt multi-line-directive-parameters ]
    */
    // directive_name()
    //                    [ 1*sp_cmt() && multi_line_directive_parameters() ]

    cl::accumulator tbd_directive_name_accumulator( this );
    cl::accumulator_deferred tbd_directive_parameters_accumulator( this );

    if( directive_name() &&
        optional( one_star_sp_cmt() && tbd_directive_parameters_accumulator.select() && multi_line_directive_parameters() ) )
    {
        warning( "Unknown directive: %0", tbd_directive_name_accumulator.get() ); // See Leave_as_warning
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
    multi-line-parameters = *(comment / q-string /
                   not-multi-line-special)
    */
    // *(comment() || q_string() || not_multi_line_special())

    while( comment() || q_string() || not_multi_line_special() )
    {}

    return true;
}

cl::alphabet_char_class not_dquote_or_semicolon_or_right_brace( "^\";}" );

bool GrammarParser::not_multi_line_special()
{
    /* ABNF:
    not-multi-line-special = spaces / %x21 / %x23-3A /
                   %x3C-7C / %x7E-10FFFF ; not ", ; or }
    */
    // spaces() / %x21 / %x23-3A /
    //                    %x3C-7C / %x7E-10FFFF ; not ", ; or }

    return skip( not_dquote_or_semicolon_or_right_brace ) > 0;
}

bool GrammarParser::root_rule()
{
    /* ABNF:
    root-rule        = value-rule / group-rule
    */
    // value_rule() || group_rule()

    cl::locator loc( this );

    Rule::uniq_ptr pu_rule( new_rule() );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( rewind_on_reject( value_rule() ) || rewind_on_reject( group_rule() ) )
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

        Rule::uniq_ptr pu_rule( new_rule() );
        RuleStackLogger rule_stack_logger( this, pu_rule );

        (rule_name() || fatal( "Expected <rule-name> after '$' in rule definition. Got: '%0'", error_token() )) &&
            star_sp_cmt() &&
            (is_get_char( '=' ) || fatal( "Expected '=' after <rule-name> in rule definition. Got: '%0'", error_token() )) &&
            star_sp_cmt() &&
            (rule_def() || fatal( "Expected <rule-def> after '=' in rule definition. Got: '%0'", error_token() ));

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
    TargetRule target_rule_name;

    if( annotations( target_rule_name_annotations ) && target_rule_name_reader( target_rule_name ) )
    {
        m.p_rule->type = Rule::TARGET_RULE;
        m.p_rule->annotations.merge( target_rule_name_annotations );
        m.p_rule->target_rule = target_rule_name;

        return true;
    }

    return false;
}

bool GrammarParser::target_rule_name_reader( TargetRule & r_target_rule )
{
    if( is_get_char( '$' ) )
    {
        cl::accumulator name_accumulator( this );

        ruleset_id_alias() || fatal( "Expected <rule_name> after '$' when reading <target_rule_name>. Got '%0'", error_token() );

        if( is_get_char( '.' ) )
        {
            std::string alias_name( name_accumulator.get() );
            name_accumulator.clear();

            AliasLookupResult alias_lookup_result( m.p_grammar->get_aliased_import( alias_name ) );
            if( ! alias_lookup_result.is_found() )
                return fatal( "Unknown alias in <target_rule_name>: %0", alias_name );

            if( rule_name() )
            {
                r_target_rule.ruleset_id = alias_lookup_result;
                r_target_rule.rule_name = name_accumulator.get();
            }
            else
                return fatal( "Expected <rule_name> in <target_rule_name> with format \"$<ruleset_id_alias>.<rule-name>\". Got '$%0.%1'", alias_name, error_token() );
        }
        else
        {
            r_target_rule.rule_name = name_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::name()
{
    /* ABNF:
    name             = ALPHA *( ALPHA / DIGIT / "-" / "_" )
    */
    // ALPHA() && *( ALPHA() || DIGIT() || "-" || "_" )

    if( ALPHA() )
    {
        while( ALPHA() || DIGIT() || accumulate( '-' ) || accumulate( '_' ) )
        {}
        return true;
    }

    return false;
}

bool GrammarParser::rule_def()
{
    /* ABNF:
    rule-def         = member-rule / type-designator rule-def-type-rule /
                   value-rule / group-rule / target-rule-name
    */
    // member_rule_or_string_type_ambiguity_hack() || type_designator() && rule_def_type_rule() || value_rule() || group_rule() || target_rule_name()


    cl::locator loc( this );

    return rewind_on_reject( member_rule_or_string_type_ambiguity_hack() ) ||
            rewind_on_reject( type_designator() && rule_def_type_rule() ) ||
            rewind_on_reject( value_rule() ) ||
            rewind_on_reject( group_rule() ) ||
            rewind_on_reject( target_rule_name() );
}

bool GrammarParser::type_designator()
{
    /* ABNF:
    type-designator  = "type" 1*sp-cmt / ":" *sp-cmt
    */
    // "type" && 1*sp_cmt() || ":" && *sp_cmt()

    cl::locator loc( this );

    return rewind_on_reject( type_kw() && one_star_sp_cmt() ) ||
            rewind_on_reject( is_get_char( ':' ) && star_sp_cmt() );
}

bool GrammarParser::rule_def_type_rule()
{
    /* ABNF:
    rule-def-type-rule = value-rule / type-choice-rule
    */
    // value_rule() || type_choice()

    cl::locator loc( this );

    return rewind_on_reject( value_rule() ) ||
            rewind_on_reject( type_choice() );
}

bool GrammarParser::value_rule()
{
    /* ABNF:
    value-rule       = primitive-rule / array-rule / object-rule
    */
    // primitive_rule() || array_rule() || object_rule()

    cl::locator loc( this );

    return rewind_on_reject( primitive_rule() ) ||
            rewind_on_reject( array_rule() ) ||
            rewind_on_reject( object_rule() );
}

bool GrammarParser::member_rule_or_string_type_ambiguity_hack()
{
    /* ABNF:
    Either:
    member-rule      = annotations
                   member-name-spec *sp-cmt ":" *sp-cmt type-rule
    Or:
                     =  annotations ( string-value / string-range )
    */

    // No need to record location because member_rule_or_string_type_ambiguity_hack() is always part of a rewound choice

    Annotations rule_annotations;

    if( annotations( rule_annotations ) && member_name_spec() )
    {
        star_sp_cmt();

        if( is_get_char( ':' ) )
        {
            // It's a member-rule
            star_sp_cmt() &&
            (type_rule() || fatal( "Expected <type-rule> after <member-name> %0. Got '%1'", m.p_rule->member_name, error_token() ) );
        }

        else
        {
            // It's a string type
            convert_member_name_to_string_type();
        }

        m.p_rule->annotations.merge( rule_annotations );

        return true;
    }

    return false;
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
        star_sp_cmt() &&
            (is_get_char( ':' ) || fatal( "Expected ':' after <member-name> %0. Got '%1'", m.p_rule->member_name, error_token() ) ) &&
            star_sp_cmt() &&
            (type_rule() || fatal( "Expected <type-rule> after <member-name> %0. Got '%1'", m.p_rule->member_name, error_token() ) );

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

bool GrammarParser::convert_member_name_to_string_type()
{
    if( ! m.is_infer_types )
    {
        m.p_rule->type = m.p_rule->member_name.is_literal() ? Rule::STRING_LITERAL : Rule::STRING_REGEX;
        m.p_rule->min = m.p_rule->max = m.p_rule->member_name.name();
        m.p_rule->member_name.clear();
    }

    else
    {
        m.p_rule->type = Rule::STRING_TYPE;
    }

    return true;
}

bool GrammarParser::type_rule()
{
    /* ABNF:
    type-rule        = value-rule / type-choice-rule / target-rule-name
    */
    // value_rule() || type_choice() || target_rule_name()

    cl::locator loc( this );

    return rewind_on_reject( value_rule() ) ||
            rewind_on_reject( type_choice() ) ||
            rewind_on_reject( target_rule_name() );
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

        type_choice_items() || fatal( "Must be at least one <type-choice-item> in <type-choice>. Got: '%0'", error_token() );

        while( choice_combiner() )
        {
            type_choice_items() || fatal( "Expected <type-choice-item> after <choice-combiner> in <type-choice>. Got: '%0'", error_token() );
        }

        is_get_char( ')' ) || fatal( "Expected ')' at end of <type-choice>. Got: '%0'", error_token() );

        return true;
    }

    return false;
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

    Rule::uniq_ptr pu_rule( new_rule() );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( ( rewind_on_reject( type_choice() ) || rewind_on_reject( type_rule() ) ) &&
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

    while( fixed( "@{" ) )
    {
        star_sp_cmt() &&
            annotation_set( r_annotations ) &&
            star_sp_cmt() &&
            (fixed( "}" ) || fatal( "Expected '}' at end of <annotation>. Got '%0'", error_token() )) &&
            star_sp_cmt();
    }

    return true;    // Annotations are optional, so we always return 'true' unless we've errored
}

bool GrammarParser::annotation_set( Annotations & r_annotations )
{
    /* ABNF:
    annotation-set   = not-annotation / unordered-annotation /
                       root-annotation /
                       exclude-min-annotation / exclude-max-annotation /
                       default-annotation / tbd-annotation
                       default-annotation / format-annotation /
                       choice-annotation / augments-annotation /
                       tbd-annotation
    */
    // not_annotation() || unordered_annotation() || root_annotation() || exclude_min_annotation() || exclude_max_annotation() || default_annotation() || format_annotation() || choice_annotation() || augments_annotation() || tbd_annotation()

    cl::locator loc( this );    // Current annotations don't benefit from rewind_on_reject(), but maintain the pattern for consistency and possible future proofing

    return rewind_on_reject( not_annotation( r_annotations ) ) ||
            rewind_on_reject( unordered_annotation( r_annotations ) ) ||
            rewind_on_reject( root_annotation( r_annotations ) ) ||
            rewind_on_reject( exclude_min_annotation( r_annotations ) ) ||
            rewind_on_reject( exclude_max_annotation( r_annotations ) ) ||
            rewind_on_reject( default_annotation( r_annotations ) ) ||
            rewind_on_reject( format_annotation( r_annotations ) ) ||
            rewind_on_reject( choice_annotation( r_annotations ) ) ||
            rewind_on_reject( augments_annotation( r_annotations ) ) ||
            rewind_on_reject( tbd_annotation() ) ||
            fatal( "Unrecognised <annotation> format. Got: '%0'" );     // Calling fatal() will throw an exception
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

bool GrammarParser::exclude_min_annotation( Annotations & r_annotations )
{
    /* ABNF:
    exclude-min-annotation = exclude-min-kw
    */
    // exclude_min_kw()

    return exclude_min_kw() && set( r_annotations.is_exclude_min, true );
}

bool GrammarParser::exclude_max_annotation( Annotations & r_annotations )
{
    /* ABNF:
    exclude-max-annotation = exclude-max-kw
    */
    // exclude_max_kw()

    return exclude_max_kw() && set( r_annotations.is_exclude_max, true );
}

bool GrammarParser::default_annotation( Annotations & r_annotations )
{
    /* ABNF:
    default-annotation = default-kw spaces primitive-value
    */
    // default_kw() && spaces() && primitive_value()

    if( default_kw() )
    {
        cl::accumulator default_accumulator( this );

        if( (spaces() || fatal( "Expected <spaces> after 'default' keyword in @{default} <annotation>. Got '%0'", error_token() )) &&
                 (primitive_value() || fatal( "Expected <primitive-value> in @{default} <annotation>. Got '%0'", error_token() )) )
        {
            r_annotations.is_defaulted = true;
            r_annotations.default_value = default_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::primitive_value()
{
    /* ABNF:
    primitive-value  = false-value / null-type / true-value /
                   float-value / integer-value / string-value
    */
    // false_value() || null_type() || true_value() || float_value() || integer_value() || string_value()

    cl::locator loc( this );
    cl::accumulator acc( this );

    return rewind_on_reject( accumulate_atomic( false_kw() && accumulator_append( "false" ) ) ) ||
            rewind_on_reject( accumulate_atomic( null_kw() && accumulator_append( "null" ) ) ) ||
            rewind_on_reject( accumulate_atomic( true_kw() && accumulator_append( "true" ) ) ) ||
            rewind_on_reject( accumulate_atomic( float_num() ) ) ||
            rewind_on_reject( accumulate_atomic( integer() ) ) ||
            rewind_on_reject( accumulate_atomic( q_string() ) );
}

bool GrammarParser::format_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    format-annotation = format-kw spaces id
    */
    // format_kw() && spaces() && id()

    if( format_kw() )
    {
        cl::accumulator id_accumulator( this );

        if( (spaces() || fatal( "Expected <spaces> after 'format' keyword in @{format} <annotation>. Got '%0'", error_token() )) &&
                 (id() || fatal( "Expected <id> in @{format} <annotation>. Got '%0'", error_token() )) )
        {
            r_annotations.format = id_accumulator.get();
        }

        return true;
    }

    return false;
}

bool GrammarParser::choice_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    choice-annotation = choice-kw
    */
    // choice_kw()

    if( choice_kw() )
    {
        r_annotations.is_choice = true;

        return true;
    }

    return false;
}

bool GrammarParser::augments_annotation( Annotations & r_annotations )
{
    /* ABNF: 
    augments-annotation = augments-kw *(spaces target-rule-name)
    */
    // augments_kw() && *(spaces() && target_rule_name())

    if( augments_kw() )
    {
        TargetRule target_rule_name;
        
        while( spaces() && target_rule_name_reader( target_rule_name ) )
        {
            r_annotations.augments.push_back( target_rule_name );
            target_rule_name.clear();
        }

        return true;
    }

    return false;
}

bool GrammarParser::tbd_annotation()
{
    /* ABNF:
    tbd-annotation   = annotation-name [ spaces annotation-parameters ]
    */
    // annotation_name() [ spaces() && annotation_parameters() ]

    cl::accumulator name_accumulator( this );

    annotation_name();

    // Report errors relating to the annotation name, near where the name is specified
    if( name_accumulator.get() == "id" || name_accumulator.get() == "assert" || name_accumulator.get() == "when" || name_accumulator.get() == "doc" )
        warning( "Unimplemented <annotation>: '%0'", name_accumulator.get() ); // See Leave_as_warning
    else if( ! name_accumulator.get().empty() )
        error( "Unknown <annotation>: '%0'", name_accumulator.get() );
    else
        fatal( "Expected <annotation> name. Got: '%0'", error_token() );

    optional( spaces() && name_accumulator.none() && annotation_parameters() );

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
                   uri-type / phone-type / email-type /
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
    //      uri_type() || phone_type() || email_type() ||
    //      datetime_type() || date_type() || time_type() ||
    //      hex_type() || base32hex_type() || base32_type() || base64url_type() || base64_type() ||
    //      any()

    cl::locator loc( this );

     return rewind_on_reject( null_type() ) ||
            rewind_on_reject( boolean_type() ) ||
            rewind_on_reject( true_value() ) ||
            rewind_on_reject( false_value() ) ||
            rewind_on_reject( string_type() ) ||
            rewind_on_reject( string_range() ) ||
            rewind_on_reject( string_value() ) ||
            rewind_on_reject( double_type() ) ||
            rewind_on_reject( float_type() ) ||
            rewind_on_reject( float_range() ) ||
            rewind_on_reject( float_value() ) ||
            rewind_on_reject( integer_type() ) ||
            rewind_on_reject( integer_range() ) ||
            rewind_on_reject( integer_value() ) ||
            rewind_on_reject( sized_int_type() ) ||
            rewind_on_reject( sized_uint_type() ) ||
            rewind_on_reject( ipv4_type() ) ||
            rewind_on_reject( ipv6_type() ) ||
            rewind_on_reject( ipaddr_type() ) ||
            rewind_on_reject( fqdn_type() ) ||
            rewind_on_reject( idn_type() ) ||
            rewind_on_reject( uri_type() ) ||
            rewind_on_reject( phone_type() ) ||
            rewind_on_reject( email_type() ) ||
            rewind_on_reject( datetime_type() ) ||
            rewind_on_reject( date_type() ) ||
            rewind_on_reject( time_type() ) ||
            rewind_on_reject( hex_type() ) ||
            rewind_on_reject( base32hex_type() ) ||
            rewind_on_reject( base32_type() ) ||
            rewind_on_reject( base64url_type() ) ||
            rewind_on_reject( base64_type() ) ||
            rewind_on_reject( any() );
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

    if( true_kw() )
    {
        m.p_rule->type = Rule::BOOLEAN;
        if( ! m.is_infer_types )
        {
            m.p_rule->min = m.p_rule->max = true;
        }
        return true;
    }
    
    return false;
}

bool GrammarParser::false_value()
{
    /* ABNF:
    false-value      = false-kw
    */
    // false_kw()

    if( false_kw() )
    {
        m.p_rule->type = Rule::BOOLEAN;
        if( ! m.is_infer_types )
        {
            m.p_rule->min = m.p_rule->max = false;
        }
        return true;
    }
    
    return false;
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
    string-value    = q-string
    */
    // q_string()

    cl::accumulator q_string_accumulator( this );

    if( q_string_as_utf8() )
    {
        if( ! m.is_infer_types )
        {
            m.p_rule->type = Rule::STRING_LITERAL;
            m.p_rule->min = m.p_rule->max = q_string_accumulator.get();
        }
        else
        {
            m.p_rule->type = Rule::STRING_TYPE;
        }

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

    bool is_float_max_complete( false );

    if( rewind_on_reject( float_min() && fixed( ".." ) && optional( float_max_accumulator.select() && record( is_float_max_complete, float_max() ) ) ) )
    {
        m.p_rule->type = Rule::DOUBLE;

        if( ! float_max_accumulator.get().empty() && ! is_float_max_complete )
            error( "Incomplete <float-max> value in <float-range>. Got: '%0'", float_max_accumulator.get() );

        if( ! float_min_accumulator.get().empty() && ! float_max_accumulator.get().empty() )
        {
            if( float_min_accumulator.to_float() > float_max_accumulator.to_float() )
                error( "Float range minimum ('%0') greater than maximum ('%1')", float_min_accumulator.get(), float_max_accumulator.get() );
        }

        if( ! float_min_accumulator.get().empty() )
            m.p_rule->min = float_min_accumulator.to_float();
        if( ! float_max_accumulator.get().empty() )
            m.p_rule->max = float_max_accumulator.to_float();

        return true;
    }

    else if( rewind_on_reject( fixed( ".." ) && float_max_accumulator.select() && float_max() ) )
    {
        m.p_rule->type = Rule::DOUBLE;

        m.p_rule->max = float_max_accumulator.to_float();

        return true;
    }

    return false;
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

    if( float_num() )
    {
        m.p_rule->type = Rule::DOUBLE;
        if( ! m.is_infer_types )
        {
            m.p_rule->min = m.p_rule->max = float_accumulator.to_float();
        }
        return true;
    }

    return false;
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

    if( rewind_on_reject( integer_min() && fixed( ".." ) && optional( integer_max_accumulator.select() && integer_max() ) ) ||
            rewind_on_reject( fixed( ".." ) && integer_max_accumulator.select() && integer_max() ) )
    {
        if( integer_min_accumulator.get().empty() || integer_min_accumulator.get()[0] == '-' )
        {
            m.p_rule->type = Rule::INTEGER;
            if( ! integer_min_accumulator.get().empty() && ! integer_max_accumulator.get().empty() )
            {
                if( integer_min_accumulator.to_int64() > integer_max_accumulator.to_int64() )
                    error( "Integer range minimum ('%0') greater than maximum ('%1')", integer_min_accumulator.get(), integer_max_accumulator.get() );
            }
            if( ! integer_min_accumulator.get().empty() )
                m.p_rule->min = integer_min_accumulator.to_int64();
            if( ! integer_max_accumulator.get().empty() )
                m.p_rule->max = integer_max_accumulator.to_int64();
        }
        else
        {
            m.p_rule->type = Rule::UINTEGER;
            if( ! integer_min_accumulator.get().empty() && ! integer_max_accumulator.get().empty() )
            {
                if( integer_min_accumulator.to_uint64() > integer_max_accumulator.to_uint64() )
                    error( "Integer range minimum ('%0') greater than maximum ('%1')", integer_min_accumulator.get(), integer_max_accumulator.get() );
            }
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
        if( m.is_infer_types )
        {
            m.p_rule->type = Rule::INTEGER;
        }
        else if( integer_accumulator.get()[0] == '-' )
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

uint64 sized_uint_min( int /*bits*/ ) // For completeness
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
            ( num_bits_accumulator.to_int() <= 64 || error( "sized int of %0 bits is larger than supported", num_bits_accumulator.get() ) ) &&
            set( m.p_rule->type, Rule::INTEGER ) &&
            set( m.p_rule->min, sized_int_min( num_bits_accumulator.to_int() ) ) &&
            set( m.p_rule->max, sized_int_max( num_bits_accumulator.to_int() ) );
}

bool GrammarParser::sized_uint_type()
{
    /* ABNF:
    sized-uint-type  = uint-kw pos-integer
    */
    // uint_kw() && pos_integer()

    cl::accumulator num_bits_accumulator( this );

    return uint_kw() && pos_integer() &&
            ( num_bits_accumulator.to_int() <= 64 || error( "sized uint of %0 bits is larger than supported", num_bits_accumulator.get() ) ) &&
            set( m.p_rule->type, Rule::UINTEGER ) &&
            set( m.p_rule->min, sized_uint_min( num_bits_accumulator.to_int() ) ) &&
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

bool GrammarParser::uri_type()
{
    /* ABNF:
    uri-type         = uri-kw [ ".." uri-scheme ]
    */
    // uri_kw() [ ".." && uri_scheme() ]

    if( uri_kw() )
    {
        m.p_rule->type = Rule::URI_TYPE;
        if( fixed( ".." ) )
        {
            cl::accumulator uri_scheme_accumulator( this );
            if( uri_scheme() )
            {
                m.p_rule->type = Rule::URI_RANGE;
                m.p_rule->min = m.p_rule->max = uri_scheme_accumulator.get();
            }
            else
                error( "Expected URI scheme specification after 'uri..' keyword. Got: '%0'", error_token() );
        }

        return true;
    }

    return false;
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

    // No need to record location because object_rule() is always part of a rewound choice

    Annotations object_annotations;

    if( annotations( object_annotations ) && is_get_char( '{' ) )
    {
        m.p_rule->type = Rule::OBJECT;
        m.p_rule->annotations.merge( object_annotations );

        star_sp_cmt() && optional( object_items() ) && star_sp_cmt();

        is_get_char( '}' ) || fatal( "Unexpected character in <object-rule>. Got: '%0'", error_token() );

        return true;
    }

    return false;
}

bool GrammarParser::object_items()
{
    /* ABNF:
    object-items     = object-item [ 1*( sequence-combiner object-item ) /
                   1*( choice-combiner object-item ) ]
    */
    // object_item() && [ 1*( sequence_combiner() && object_item() ) || 1*( choice_combiner() && object_item() ) ]

    if( object_item() )
    {
        one_star_sequence_combiner_and_object_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            one_star_choice_combiner_and_object_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::one_star_sequence_combiner_and_object_item()
{
    bool is_sequence = false;
    bool is_choice_combiner = false;

    while( sequence_combiner() || (is_sequence && record( is_choice_combiner, choice_combiner())) )
    {
        is_sequence = true;

        if( is_choice_combiner )
        {
            error( "<choice-combiner> can not be used with <sequence-combiner> in <object-rule> without Parentheses" );
            is_choice_combiner = false;
        }

        object_item() || fatal( "Expected <object-item> after <sequence-combiner> in object definition" );

    }

    return is_sequence;
}

bool GrammarParser::one_star_choice_combiner_and_object_item()
{
    bool is_choice = false;
    bool is_sequence_combiner = false;

    while( choice_combiner() || (is_choice && record( is_sequence_combiner, sequence_combiner())) )
    {
        is_choice = true;

        if( is_sequence_combiner )
        {
            error( "<sequence-combiner> can not be used with <choice-combiner> in <object-rule> without Parentheses" );
            is_sequence_combiner = false;
        }

        object_item() || fatal( "Expected <object-item> after <choice-combiner> in object definition" );
    }

    return is_choice;
}

bool GrammarParser::object_item()
{
    /* ABNF:
    object-item      = object-item-types *sp-cmt [ repetition *sp-cmt ]
    */
    // object_item_types() && *sp_cmt() && [ repetition() && *sp_cmt() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new_rule() );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( object_item_types() && star_sp_cmt() && optional( repetition() && star_sp_cmt() ) )
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

    return rewind_on_reject( object_group() ) ||
            rewind_on_reject( member_rule() ) ||
            rewind_on_reject( target_rule_name() );
}

bool GrammarParser::object_group()
{
    /* ABNF:
    object-group     = annotations "(" *sp-cmt [ object-items *sp-cmt ] ")"
    */
    // annotations() && "(" && *sp_cmt() [ object_items() && *sp_cmt() ] ")"

    Annotations object_group_annotations;

    if( annotations( object_group_annotations ) && is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::OBJECT_GROUP;
        m.p_rule->annotations.merge( object_group_annotations );

        star_sp_cmt() && optional( object_items() ) && star_sp_cmt();

        is_get_char( ')' ) || fatal( "Unexpected character in <object-group>. Got: '%0'", error_token() );

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

    // No need to record location because array_rule() is always part of a rewound choice

    Annotations array_annotations;

    if( annotations( array_annotations ) && is_get_char( '[' ) )
    {
        m.p_rule->type = Rule::ARRAY;
        m.p_rule->annotations.merge( array_annotations );

        star_sp_cmt() && optional( array_items() ) && star_sp_cmt();

        is_get_char( ']' ) || fatal( "Unexpected character in <array-rule>. Got: '%0'", error_token() );

        return true;
    }

    return false;
}

bool GrammarParser::array_items()
{
    /* ABNF:
    array-items      = array-item [ 1*( sequence-combiner array-item ) /
                   1*( choice-combiner array-item ) ]
    */
    // array_item() && [ 1*( sequence_combiner() && array_item() ) || 1*( choice_combiner() && array_item() ) ]

    if( array_item() )
    {
        one_star_sequence_combiner_and_array_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            one_star_choice_combiner_and_array_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::one_star_sequence_combiner_and_array_item()
{
    bool is_sequence = false;
    bool is_choice_combiner = false;

    while( sequence_combiner() || (is_sequence && record( is_choice_combiner, choice_combiner())) )
    {
        is_sequence = true;

        if( is_choice_combiner )
        {
            error( "<choice-combiner> can not be used with <sequence-combiner> in <array-rule> without Parentheses" );
            is_choice_combiner = false;
        }

        array_item() || fatal( "Expected <array-item> after <sequence-combiner> in object definition" );

    }

    return is_sequence;
}

bool GrammarParser::one_star_choice_combiner_and_array_item()
{
    bool is_choice = false;
    bool is_sequence_combiner = false;

    while( choice_combiner() || (is_choice && record( is_sequence_combiner, sequence_combiner())) )
    {
        is_choice = true;

        if( is_sequence_combiner )
        {
            error( "<sequence-combiner> can not be used with <choice-combiner> in <array-rule> without Parentheses" );
            is_sequence_combiner = false;
        }

        array_item() || fatal( "Expected <array-item> after <choice-combiner> in object definition" );
    }

    return is_choice;
}

bool GrammarParser::array_item()
{
    /* ABNF:
    array-item       = array-item-types *sp-cmt [ repetition *sp-cmt ]
    */
    // array_item_types() && *sp_cmt() [ repetition() && *sp_cmt() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new_rule() );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( array_item_types() && star_sp_cmt() && optional( repetition() && star_sp_cmt() ) )
    {
        p_parent->append_child_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::array_item_types()
{
    /* ABNF:
    array-item-types = array-group / type-rule
    */
    // array_group() || type_rule()

    cl::locator loc( this );

    return rewind_on_reject( array_group() ) ||
            rewind_on_reject( type_rule() );
}

bool GrammarParser::array_group()
{
    /* ABNF:
    array-group      = annotations "(" *sp-cmt [ array-items *sp-cmt ] ")"
    */
    // annotations() && "(" && *sp_cmt() && [ array_items() && *sp_cmt() ] && ")"

    Annotations array_group_annotations;

    if( annotations( array_group_annotations ) && is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::ARRAY_GROUP;
        m.p_rule->annotations.merge( array_group_annotations );

        star_sp_cmt() && optional( array_items() ) && star_sp_cmt();

        is_get_char( ')' ) || fatal( "Unexpected character in <array-group>. Got: '%0'", error_token() );

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

    // No need to record location because group_rule() is always part of a rewound choice

    Annotations group_annotations;

    if( annotations( group_annotations ) && is_get_char( '(' ) )
    {
        m.p_rule->type = Rule::GROUP;
        m.p_rule->annotations.merge( group_annotations );

        star_sp_cmt() && optional( group_items() ) && star_sp_cmt();

        is_get_char( ')' ) || fatal( "Unexpected character in <group-rule> or <group-group>. Got: '%0'", error_token() );

        return true;
    }

    return false;
}

bool GrammarParser::group_items()
{
    /* ABNF:
    group-items      = group-item [ 1*( sequence-combiner group-item ) /
                   1*( choice-combiner group-item ) ]
    */
    // group_item() && [ 1*( sequence_combiner() && group_item() ) || 1*( choice_combiner() && group_item() ) ]

    if( group_item() )
    {
        one_star_sequence_combiner_and_group_item() && set( m.p_rule->child_combiner, Rule::Sequence ) ||
            one_star_choice_combiner_and_group_item() && set( m.p_rule->child_combiner, Rule::Choice );
        return true;
    }

    return false;
}

bool GrammarParser::one_star_sequence_combiner_and_group_item()
{
    bool is_sequence = false;
    bool is_choice_combiner = false;

    while( sequence_combiner() || (is_sequence && record( is_choice_combiner, choice_combiner())) )
    {
        is_sequence = true;

        if( is_choice_combiner )
        {
            error( "<choice-combiner> can not be used with <sequence-combiner> in <group-rule> without Parentheses" );
            is_choice_combiner = false;
        }

        group_item() || fatal( "Expected <group-item> after <sequence-combiner> in object definition" );

    }

    return is_sequence;
}

bool GrammarParser::one_star_choice_combiner_and_group_item()
{
    bool is_choice = false;
    bool is_sequence_combiner = false;

    while( choice_combiner() || (is_choice && record( is_sequence_combiner, sequence_combiner())) )
    {
        is_choice = true;

        if( is_sequence_combiner )
        {
            error( "<sequence-combiner> can not be used with <choice-combiner> in <group-rule> without Parentheses" );
            is_sequence_combiner = false;
        }

        group_item() || fatal( "Expected <group-item> after <choice-combiner> in object definition" );
    }

    return is_choice;
}

bool GrammarParser::group_item()
{
    /* ABNF:
    group-item       = group-item-types *sp-cmt [ repetition *sp-cmt ]
    */
    // group_item_types() && *sp_cmt() [ repetition() && *sp_cmt() ]

    Rule * p_parent = m.p_rule;

    Rule::uniq_ptr pu_rule( new_rule() );
    RuleStackLogger rule_stack_logger( this, pu_rule );

    if( group_item_types() && star_sp_cmt() && optional( repetition() && star_sp_cmt() ) )
    {
        p_parent->append_child_rule( pu_rule );

        return true;
    }

    return false;
}

bool GrammarParser::group_item_types()
{
    /* ABNF:
    group-item-types = group-group / member-rule / type-rule
    */
    // group_group() || member_rule_or_string_type_ambiguity_hack() || type_rule()

    cl::locator loc( this );

    return rewind_on_reject( group_group() ) ||
            rewind_on_reject( member_rule_or_string_type_ambiguity_hack() ) ||
            rewind_on_reject( type_rule() );
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
    sequence-combiner = "," *sp-cmt
    */
    // "," && *sp_cmt()

    return is_get_char( ',' ) && star_sp_cmt();
}

bool GrammarParser::choice_combiner()
{
    /* ABNF:
    choice-combiner  = "|" *sp-cmt
    */
    // "|" && *sp_cmt()

    return is_get_char( '|' ) && star_sp_cmt();
}

bool GrammarParser::repetition()
{
    /* ABNF:
    repetition       = optional / one-or-more /
                   repetition-range / zero-or-more
    */
    // optional() || one_or_more() || repetition_range() || zero_or_more()

    cl::locator loc( this );

    // The order of these routines is important
    return rewind_on_reject( optional_marker() ) ||
            rewind_on_reject( one_or_more() ) ||
            rewind_on_reject( repetition_range() ) ||
            rewind_on_reject( zero_or_more() );
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

bool GrammarParser::repetition_range()
{
    /* ABNF:
    repetition-range = "*" *sp-cmt (
                   min-max-repetition / min-repetition /
                   specific-repetition )
    */
    // "*" && *sp_cmt() && (
    //                    min_max_repetition() || min_repetition() || specific_repetition() )

    cl::locator loc_outer( this );

    if( is_get_char( '*' ) && star_sp_cmt() )
    {
        cl::locator loc_inner( this );

        // The order of these routines is important
        if( rewind_on_reject( min_max_repetition() ) ||
                rewind_on_reject( min_repetition() ) ||
                rewind_on_reject( specific_repetition() ) )
            return true;   // if false, fall through to restore loc_outer recorded location (we'll want to re-scan the '*')
    }

    location_top();

    return false;
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

        step_size() && set( m.p_rule->repetition.step, repetition_accumulator.to_int() ) || fatal( "Expected repetition step size after '%'. Got: '%0'", error_token() );

        return true;
    }

    return false;
}

bool GrammarParser::step_size()
{
    /* ABNF:
    step-size        = non-neg-integer
    */
    // non_neg_integer()

    return non_neg_integer();
}

bool GrammarParser::integer()
{
    /* ABNF:
    integer          = "0" / ["-"] pos-integer
    */
    // "0" / ["-"] && pos_integer()

    return (zero() && (! peek_is_in( cl::alphabet_digit() ) || fatal( "Leading zeros not allowed on integers. Got '0%0'", error_token() ) ) ) ||
            optional( minus() ) && pos_integer();
}

bool GrammarParser::non_neg_integer()
{
    /* ABNF:
    non-neg-integer  = "0" / pos-integer
    */
    // "0" || pos_integer()

    return (zero() && (! peek_is_in( cl::alphabet_digit() ) || fatal( "Leading zeros not allowed on integers. Got '0%0'", error_token() ) ) ) ||
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

bool GrammarParser::q_string_as_utf8()  // Doesn't collect wrapping quotation marks
{
    if( is_get_char( '"' ) )    // Don't accumulate quotation_mark()
    {
        std::string utf8_string;

        return get_qstring_contents( &utf8_string ) && is_get_char( '"' ) &&
                accumulator_append( utf8_string ) || fatal( "Badly formed QString. Got '%0'", utf8_string );
    }

    return false;
}

bool GrammarParser::q_string()  // Collects wrapping quotation marks
{
    /* ABNF:
    q-string         = quotation-mark *char quotation-mark
    */
    // quotation_mark() && *qs_char() && quotation_mark()

    if( quotation_mark() )
    {
        cl::accumulator qstring_accumulator( this );

        star_qs_char();

        if( quotation_mark() )
            qstring_accumulator.append_to_previous();
        else
            fatal( "Badly formed QString. Got '%0'", qstring_accumulator.get() );

        return true;
    }

    return false;
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

    return unescaped() || escape() && (escape_code() || u() && four_HEXDIG());
}

bool GrammarParser::escape()
{
    /* ABNF:
    escape           = %x5C              ; \
    */
    // %x5C              ; '\'

    return accumulate( '\\' );
}

cl::alphabet_char_class escape_code_alphabet( "\"\\\\/bfnrt" );     // \\\\ - \\ to make a single \ in C++ string, \\\\ to make \\ in char class

bool GrammarParser::escape_code()
{
    return accumulate( escape_code_alphabet );
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

bool GrammarParser::quotation_mark()
{
    /* ABNF:
    quotation-mark   = %x22      ; "
    */
    // %x22      ; "

    return accumulate( '"' );
}

bool is_qstring_unescaped( char c )
{
    // unescaped        = %x20-21 / %x23-5B / %x5D-10FFFF

    return c >= 0x20 && c <= 0x21 || c >= 0x23 && c <= 0x5b || c >= 0x5d || ((c & 0x80) != 0);
}

bool GrammarParser::unescaped()
{
    /* ABNF:
    unescaped        = %x20-21 / %x23-5B / %x5D-10FFFF
    */
    // %x20-21 / %x23-5B / %x5D-10FFFF

    return accumulate( cl::alphabet_function( is_qstring_unescaped ) );
}

bool GrammarParser::regex()
{
    /* ABNF:
    regex            = "/" *( escape ascii-char / not-slash ) "/" [ regex-modifiers ]
    */
    // "/" && *( escape() re_escape_code() || not_slash() ) "/" [ regex_modifiers() ]

    if( is_get_char( '/' ) )
    {
        cl::accumulator re_accumulator( this );

        re_accumulator.append( '/' );

        while( escape() ? re_escape_code() : not_slash() )
        {}

        (accumulate( '/' ) && optional( regex_modifiers() )) || fatal( "Error reading regular expression. Got: '%0'", re_accumulator.get() );

        re_accumulator.append_to_previous();

        return true;
    }

    return false;
}

bool is_re_escape_code( char c )
{
    return c >= 0x20 && c <= 0x7f;
}

bool GrammarParser::re_escape_code()
{
    /* ABNF:
    re-escape-code   = %x20-7F ; Specific codes listed elsewhere
    */
    // %x20-7F ; Specific codes listed elsewhere

    return accumulate( cl::alphabet_function( is_re_escape_code ) );
}

bool is_not_slash( char c )
{
    // not_slash() = HTAB() || CR() || LF() / %x20-2E / %x30-10FFFF

    return c == '\t' || c == '\r' || c == '\n' ||
            c >= 0x20 && c <= 0x2e || c >= 0x30 || ((c & 0x80) != 0);
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

bool GrammarParser::uri_scheme()
{
    /* ABNF:
    uri-scheme       = 1*ALPHA
    */
    // 1*ALPHA()

    return one_star_ALPHA();
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

bool GrammarParser::augments_kw()
{
    /* ABNF: 
    augments-kw      = %x61.75.67.6D.65.6E.74.73       ; "augments"
    */
    // %x61.75.67.6D.65.6E.74.73       ; "augments"

    return fixed( "augments" );
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

bool GrammarParser::choice_kw()
{
    /* ABNF: 
    choice-kw        = %x63.68.6F.69.63.65             ; "choice"
    */
    // %x63.68.6F.69.63.65             ; "choice"

    return fixed( "choice" );
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

bool GrammarParser::default_kw()
{
    /* ABNF:
    default-kw       = %x64.65.66.61.75.6C.74          ; "default"
    */
    // %x64.65.66.61.75.6C.74          ; "default"

    return fixed( "default" );
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

bool GrammarParser::exclude_max_kw()
{
    /* ABNF:
    exclude-max-kw   = %x65.78.63.6C.75.64.65.2D.6D.61.78 ; "exclude-max"
    */
    // %x65.78.63.6C.75.64.65.2D.6D.61.78 ; "exclude-max"

    return fixed( "exclude-max" );
}

bool GrammarParser::exclude_min_kw()
{
    /* ABNF:
    exclude-min-kw   = %x65.78.63.6C.75.64.65.2D.6D.69.6E ; "exclude-min"
    */
    // %x65.78.63.6C.75.64.65.2D.6D.69.6E ; "exclude-min"

    return fixed( "exclude-min" );
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

bool GrammarParser::format_kw()
{
    /* ABNF: 
    format-kw        = %x66.6F.72.6D.61.74             ; "format"
    */
    // %x66.6F.72.6D.61.74             ; "format"

    return fixed( "format" );
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

bool GrammarParser::infer_types_kw()
{
    /* ABNF: 
    infer-types-kw   = %x69.6E.66.65.72.2D.74.79.70.65.73 ; "infer-types"
    */
    // %x69.6E.66.65.72.2D.74.79.70.65.73 ; "infer-types"

    return fixed( "infer-types" );
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

std::string GrammarParser::error_token()    // Attempts to extract the token that led to an error, and then re-winds
{
    using namespace cl::alphabet_helpers;

    std::string token;

    cl::locator loc( this );

    get();

    if( is_current_at_end() )
        return "<end-of-input>";

    token += current();

    if( is_digit( current() ) || current() == '.' || current() == '-' )
    {
        while( is_digit( get() ) || current() == '.' || current() == 'e' || current() == '-' || current() == '+' )
            token += current();
    }

    else if( current() == '$' || is_alpha( current() ) || current() == '_' )
    {
        while( is_alpha( get() ) || is_digit( current() ) ||
                current() == '_' || current() == '-' || current() == '.' )
            token += current();
    }

    else if( (current() & 0x80) != 0 )
    {
        // A UTF-8 character
        while( (get() & 0x80) != 0 )
            token += current();
    }

    else
    {
        // Was probably some form of punctuation - already added to 'token' return value
    }

    location_top();

    return token;
}

//----------------------------------------------------------------------------
//                        Internal class Linker
//----------------------------------------------------------------------------

class Linker
{
private:
    struct Members {
        JCRParser * p_jcr_parser;
        GrammarSet * p_grammar_set;
        bool is_errored;
        Members(
            JCRParser * p_jcr_parser_in,
            GrammarSet * p_grammar_set_in )
            :
            p_jcr_parser( p_jcr_parser_in ),
            p_grammar_set( p_grammar_set_in ),
            is_errored( false )
        {}
    } m;

    struct LoopDetector
    {
        LoopDetector * p_prev;
        Rule * p_rule;

        LoopDetector( Rule * p_rule_in )
            : p_prev( 0 ), p_rule( p_rule_in )
        {}
        LoopDetector( LoopDetector * p_prev_in, Rule * p_rule_in )
            : p_prev( p_prev_in ), p_rule( p_rule_in )
        {}

        bool is_looped() const
        {
            for( LoopDetector * p_loop_detector = p_prev; p_loop_detector; p_loop_detector = p_loop_detector->p_prev )
                if( p_loop_detector->p_rule == p_rule )
                    return true;
            return false;
        }
    };

    struct LinkResult
    {
        Rule * p_initial_rule;
        Rule * p_member_rule;
        Rule * p_type_rule;

        LinkResult( Rule * p_rule_in ) : p_initial_rule( p_rule_in ), p_member_rule( p_rule_in ), p_type_rule( p_rule_in ) {}
    };

public:
    Linker( JCRParser * p_jcr_parser, GrammarSet * p_grammar_set )
        : m( p_jcr_parser, p_grammar_set )
    {}
    bool link();
    bool link( Grammar * p_grammar );

private:
    void check_for_duplicate_ruleset_ids();
    void check_for_duplicate_rule_names( Grammar * p_grammar );
    void link_global_rules( Grammar * p_grammar );
    void link_global_rule( Rule * p_global_rule );
    void do_link( LinkResult * p_link_result, LoopDetector * p_loop_detector, Rule * p_global_rule );
    void link_child_rules( Rule * p_rule );
    void link_child_rule( Rule * p_rule );

    void error( const Rule * p_rule, const char * p_message )
    {
        report( p_rule, Severity::ERROR, p_message );
        m.is_errored = true;
    }
    void error( const Rule * p_rule, const char * p_format, const clutils::str_args & r_arg_1 )
    {
        error( p_rule, expand( p_format, r_arg_1 ).c_str() );
    }
    void error( const Rule * p_rule, const char * p_format, const clutils::str_args & r_arg_1, const clutils::str_args & r_arg_2 )
    {
        error( p_rule, expand( p_format, r_arg_1, r_arg_2 ).c_str() );
    }
    void report( const Rule * p_rule, Severity severity, const char * p_message )
    {
        m.p_jcr_parser->report( p_rule->p_grammar->jcr_source, p_rule->line_number, p_rule->column_number, severity, p_message );
    }

    void error( const Grammar * p_grammar, const char * p_message )
    {
        report( p_grammar, Severity::ERROR, p_message );
        m.is_errored = true;
    }
    void error( const Grammar * p_grammar, const char * p_format, const clutils::str_args & r_arg_1 )
    {
        error( p_grammar, expand( p_format, r_arg_1 ).c_str() );
    }
    void error( const Grammar * p_grammar, const char * p_format, const clutils::str_args & r_arg_1, const clutils::str_args & r_arg_2 )
    {
        error( p_grammar, expand( p_format, r_arg_1, r_arg_2 ).c_str() );
    }
    void report( const Grammar * p_grammar, Severity severity, const char * p_message )
    {
        m.p_jcr_parser->report( p_grammar->jcr_source, severity, p_message );
    }
};

bool Linker::link()
{
    check_for_duplicate_ruleset_ids();
    for( size_t i=0; i<m.p_grammar_set->size(); ++i )
        link( &(*m.p_grammar_set)[i] );
    return ! m.is_errored;
}

void Linker::check_for_duplicate_ruleset_ids()
{
    for( size_t i=0; i<m.p_grammar_set->size(); ++i )
    {
        Grammar * p_grammar_under_test = &(*m.p_grammar_set)[i];
        if( ! p_grammar_under_test->ruleset_id.empty() )
        {
            for( size_t j=i+1; j<m.p_grammar_set->size(); ++j )
            {
                Grammar * p_possible_duplicate = &(*m.p_grammar_set)[j];
                if( p_grammar_under_test->ruleset_id == p_possible_duplicate->ruleset_id )
                    error( p_grammar_under_test,
                            "Duplicate <ruleset-id> '%0' found in source '%1'",
                            p_grammar_under_test->ruleset_id,
                            p_possible_duplicate->jcr_source );
            }
        }
    }
}

bool Linker::link( Grammar * p_grammar )
{
    check_for_duplicate_rule_names( p_grammar );
    link_global_rules( p_grammar );
    return ! m.is_errored;
}

void Linker::check_for_duplicate_rule_names( Grammar * p_grammar )
{
    for( size_t i=0; i<p_grammar->rules.size(); ++i )
    {
        Rule * p_rule_under_test = &(p_grammar->rules[i]);
        for( size_t j=i+1; j<p_grammar->rules.size(); ++j )
        {
            Rule * p_possible_duplicate = &(p_grammar->rules[j]);
            if( p_rule_under_test->rule_name == p_possible_duplicate->rule_name )
                error( p_rule_under_test,
                        "Duplicate <rule-name> '$%0' found at (line: '%1', char: '%2')",
                        clutils::str_args( p_rule_under_test->rule_name ) <<
                            p_possible_duplicate->line_number <<
                            p_possible_duplicate->column_number );
        }
    }
}

void Linker::link_global_rules( Grammar * p_grammar )
{
    // Link global rules to global rules first.  Then link any child rules to global rules.
    // This will be more efficient, and the names for global rules will offer better error messages.

    for( size_t i=0; i<p_grammar->rules.size(); ++i )
    {
        link_global_rule( &p_grammar->rules[i] );
    }
    for( size_t i=0; i<p_grammar->rules.size(); ++i )
    {
        link_child_rules( &p_grammar->rules[i] );
    }
}

void Linker::link_global_rule( Rule * p_global_rule )
{
    LinkResult link_result( p_global_rule );
    LoopDetector loop_detector( p_global_rule );
    do_link( &link_result, &loop_detector, p_global_rule );
    p_global_rule->p_rule = link_result.p_member_rule;
    p_global_rule->p_type = link_result.p_type_rule;
}

void Linker::do_link( LinkResult * p_link_result, LoopDetector * p_loop_detector, Rule * p_global_rule )
{
    if( ! p_global_rule->target_rule.rule_name.empty() )
    {
        Rule * p_target_rule = p_global_rule->find_target_rule();
        if( ! p_target_rule )
        {
            error( p_global_rule, "Unable to find Target rule '$%0' for global rule '$%1'",
                    p_global_rule->target_rule,
                    p_global_rule->get_rule_name() );
        }
        else
        {
            LoopDetector loop_detector( p_loop_detector, p_target_rule );
            if( loop_detector.is_looped() )
            {
                error( p_target_rule, "Target rule '$%0' loops back to itself when linking global rule '$%1'",
                        p_target_rule->rule_name,
                        p_global_rule->get_rule_name() );
            }
            else
            {
                p_link_result->p_type_rule = p_target_rule;
                if( p_target_rule->is_member_rule() )
                {
                    if( p_link_result->p_member_rule->is_member_rule() )
                        error( p_link_result->p_member_rule, "Global member rule '$%0' links to another Member rule: '%1'",
                                p_global_rule->get_rule_name(),
                                p_link_result->p_member_rule->target_rule );
                    else
                        p_link_result->p_member_rule = p_target_rule;
                }
                do_link( p_link_result, &loop_detector, p_target_rule );
            }
        }
    }
}

void Linker::link_child_rules( Rule * p_rule )
{
    for( size_t i=0; i<p_rule->children.size(); ++i )
    {
        link_child_rule( &p_rule->children[i] );
        link_child_rules( &p_rule->children[i] );
    }
}

void Linker::link_child_rule( Rule * p_rule )
{
    if( ! p_rule->target_rule.rule_name.empty() )
    {
        Rule * p_target_rule = p_rule->find_target_rule();
        if( ! p_target_rule )
        {
            error( p_rule, "Unable to find Target rule '%0'", p_rule->target_rule );
        }
        else
        {
            p_rule->p_type = p_target_rule->p_type;
            if( p_target_rule->is_member_rule() )
            {
                if( p_rule->is_member_rule() )
                    error( p_rule, "Member rule links to another Member rule: '$%0'",
                            p_target_rule->p_rule->rule_name );
                else
            p_rule->p_rule = p_target_rule->p_rule;
            }
        }
    }
}

} // End of Anonymous namespace

//----------------------------------------------------------------------------
//                           class JCRParser
//----------------------------------------------------------------------------

JCRParser::Status JCRParser::add_grammar( const char * p_file_name )
{
    cl::reader_file reader( p_file_name );
    if( ! reader.is_open() )
    {
        m.p_grammar_set->inc_error_count();
        return S_UNABLE_TO_OPEN_FILE;
    }

    return parse_grammar( reader, p_file_name );
}

JCRParser::Status JCRParser::add_grammar( const std::string & rules )
{
    cl::reader_string reader( rules );

    return parse_grammar( reader, clutils::expand( "std::string @ ", (void *)&rules ) );
}

JCRParser::Status JCRParser::add_grammar( const char * p_rules, size_t size )
{
    cl::reader_mem_buf reader( p_rules, size );

    return parse_grammar( reader, clutils::expand( "const char * ", (void *)p_rules ) );
}

JCRParser::Status JCRParser::link()
{
    Linker linker( this, m.p_grammar_set );

    return linker.link() ? S_OK : S_ERROR;
}

JCRParser::Status JCRParser::link( Grammar * p_grammar )
{
    Linker linker( this, m.p_grammar_set );

    return linker.link( p_grammar ) ? S_OK : S_ERROR;
}

JCRParser::Status JCRParser::parse_grammar( cl::reader & reader, const std::string & jcr_source )
{
    GrammarParser parser( this, reader, m.p_grammar_set, m.p_grammar_set->append_grammar( jcr_source ) );

    parser.parse();

    return parser.status();
}

//----------------------------------------------------------------------------
//                           class JCRParserWithReporter
//----------------------------------------------------------------------------

void JCRParserWithReporter::report( const std::string & source, size_t line, size_t column, Severity severity, const char * p_message )
{
    std::ostringstream oss;
    oss << severity << ": " << source;
    if( line != ~0U )
    {
        oss << " (line: " << line;
        if( column != ~0U )
            oss << ", char: " << column;
        oss << ")";
    }
    oss << ":\n      " << p_message << "\n";
    std::string constructed_message = oss.str();

    if( reported_messages.find( constructed_message ) == reported_messages.end() )
    {
        std::cout << constructed_message;

        if( severity == Severity::WARNING )
            grammar_set()->inc_warning_count();
        else
            grammar_set()->inc_error_count();

        reported_messages.insert( constructed_message );
    }
}

//----------------------------------------------------------------------------
//                           class MemberName
//----------------------------------------------------------------------------

std::string MemberName::pattern() const
{
    size_t first = m.name.find_first_of( '/' );
    size_t last = m.name.find_last_of( '/' );
    if( first == std::string::npos || last == std::string::npos )
        return m.name;
    return m.name.substr( first + 1, last - 1 );
}

std::string MemberName::modifiers() const
{
    size_t last = m.name.find_last_of( '/' );
    if( last == std::string::npos )
        return "";
    return m.name.substr( last + 1 );
}

std::ostream & operator << ( std::ostream & r_os, const MemberName & r_mn )
{
    if( r_mn.is_absent() )
        r_os << "<absent>";
    else if( r_mn.is_literal() )
        r_os << '"' << r_mn.name() << '"';
    else
        r_os << '/' << r_mn.pattern() << '/' << r_mn.modifiers();
    return r_os;
}

//----------------------------------------------------------------------------
//                           class ValueConstraint
//----------------------------------------------------------------------------

std::string ValueConstraint::as_pattern() const
{
    assert( m.form == Members::string_form );
    size_t first = m.string_value.find_first_of( '/' );
    size_t last = m.string_value.find_last_of( '/' );
    if( first == std::string::npos || last == std::string::npos )
        return m.string_value;
    return m.string_value.substr( first + 1, last - 1 );
}

std::string ValueConstraint::as_modifiers() const
{
    assert( m.form == Members::string_form );
    size_t last = m.string_value.find_last_of( '/' );
    if( last == std::string::npos )
        return "";
    return m.string_value.substr( last + 1 );
}

//----------------------------------------------------------------------------
//                           class TargetRule
//----------------------------------------------------------------------------

std::ostream & operator << ( std::ostream & r_os, const TargetRule & r_tr )
{
    if( ! r_tr.ruleset_id.empty() )
        r_os << "${" << r_tr.ruleset_id << "}." << r_tr.rule_name;
    else
        r_os << "$" << r_tr.rule_name;
    return r_os;
}

//----------------------------------------------------------------------------
//                           class Rule
//----------------------------------------------------------------------------

const Rule * Rule::find_target_rule() const
{
    if( target_rule.p_rule )
        return target_rule.p_rule;

    const Rule * p_rule;

    if( target_rule.rule_name.empty() )
    {
        return this;    // Resolve to self in the absence of a link
    }

    else if( target_rule.ruleset_id.empty() )
    {
        p_rule = p_grammar->find_rule( target_rule.rule_name );
        if( p_rule )
            return p_rule;
        for( size_t i=0; i<p_grammar->unaliased_imports.size(); ++i )
        {
            const Grammar * p_g = p_grammar->p_grammar_set->find_grammar( p_grammar->unaliased_imports[i] );
            if( p_g )
            {
                p_rule = p_g->find_rule( target_rule.rule_name );
                if( p_rule )
                    return p_rule;
            }
        }
    }

    else
    {
        const Grammar * p_g = p_grammar->p_grammar_set->find_grammar( target_rule.ruleset_id );
        if( p_g )
        {
            p_rule = p_g->find_rule( target_rule.rule_name );
            if( p_rule )
                return p_rule;
        }
    }

    return 0;
}

}   // namespace cljcr
