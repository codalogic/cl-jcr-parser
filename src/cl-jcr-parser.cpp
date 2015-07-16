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

namespace cljcr {

//----------------------------------------------------------------------------
//                           Internal detail
//----------------------------------------------------------------------------

namespace { // Anonymous namespace for detail

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
        JCRParser::Status status;

        Members( JCRParser * p_parent_in,
                cl::reader & r_reader_in,
                Grammar * p_grammar_in )
            :
            p_parent( p_parent_in ),
            p_grammar( p_grammar_in ),
            r_reader( r_reader_in ),
            status( JCRParser::S_OK )
        {}
    } m;

public:
    GrammarParser( JCRParser * p_parent, cl::reader & r_reader, Grammar * p_grammar );
    bool parse();
    JCRParser::Status status() const { return m.status; }

private:
    bool error( JCRParser::Status code, const char * p_message );
    bool one_star_c_wsp();
	bool comment();
    bool star_c_wsp();
    bool rule_or_directive();
    void directive();
    void rule();
    void recover_bad_rule_or_directive();
    void recover_badly();
};

GrammarParser::GrammarParser(
                        JCRParser * p_parent,
                        cl::reader & r_reader,
                        Grammar * p_grammar )
    :
    cl::dsl_pa( r_reader ),
    m( p_parent, r_reader, p_grammar )
{}

bool GrammarParser::parse()
{
    //  grammar         = 1*( *c-wsp (rule / directive) ) *c-wsp
    while( star_c_wsp() && rule_or_directive() )
    {}

    star_c_wsp();

    if( ! is_current_at_end() )
        error( JCRParser::S_EXPECTED_END_OF_RULES, "Unexpected input at end of rules" );

    return m.status == JCRParser::S_OK;
}

bool GrammarParser::rule_or_directive()
{
    using namespace cl::alphabet_helpers;
    
    get();

    if( is_current_at_end() )
        return false;

    else if( current_is( '#' ) )
        directive();

    else if( is_alpha( current() ) )
        rule();

    else
        recover_bad_rule_or_directive();

    return true;
}

void GrammarParser::directive()
{
    Directive * p_directive = m.p_grammar->append_directive();
    std::string directive_line;
    get_until( &directive_line, cl::alphabet_eol() );
    p_directive->set( directive_line );
    skip( cl::alphabet_eol() );
}

void GrammarParser::rule()
{
	// First character is in current()
}

void GrammarParser::recover_bad_rule_or_directive()
{
    recover_badly();
}

void GrammarParser::recover_badly()
{
    while( get() && ! is_current_at_end() )
    {}
}
    //
    //  rule            = rulename *c-wsp definition
    //
    //  ; rulenames must be unique, and may not be a reserved word
    //  ; rulenames are case sensitive
    //  rulename        = ALPHA *(ALPHA / DIGIT / "-" / "_")
    //
    //  definition      = member-rule / definition-rule
    //
    //  member-rule     = member-name *c-wsp definition-rule
    //
    //  member-name     = ( "^" %x22.22 ) /
    //                     ( %x22 *q-string %x22 )
    //
    //  definition-rule =  ( value-rule /
    //                            object-rule /
    //                            array-rule /
    //                            group-rule /
    //                            rule-name )
    //
    //  value-rule      = ":" *c-wsp type-rule
    //
    //  type-rule       = boolean-type /
    //                   null-type /
    //                   integer-type /
    //                   float-type /
    //                   string-type /
    //                   uri-type /
    //                   ip-type /
    //                   dns-type /
    //                   date-type /
    //                   email-type /
    //                   phone-type /
    //                   base64-type /
    //                   enum-type /
    //                   any-type
    //
    //  boolean-type    = "boolean"
    //  null-type       = "null"
    //  integer-type    = "integer" [ 1*c-wsp integer ".." integer ]
    //  float-type      = "float"   [ 1*c-wsp float   ".." float   ]
    //  string-type     = "string"  [ *c-wsp "/" *regex-char "/" ]
    //  uri-type        = "uri"     [ 1*c-wsp URI ] ; URI defined in RFC 3986
    //  ip-type         = "ip4" / "ip6"
    //  dns-type        = "fqdn" / "idn"
    //  date-type       = "date-time" / "full-date" / "full-time"
    //  email-type      = "email"
    //  phone-type      = "phone"
    //  base64-type     = "base64"
    //  enum-type       = "<" *c-wsp enum-item *(1*c-wsp enum-item) *c-wsp ">"
    //  any-type        = "any"
    //
    //  enum-item       = float / integer /
    //                   "1" / "0" / "true" / "false" /
    //                   "null" /
    //                   q-string
    //
    //  object-rule     = "{" *c-wsp object-member *(
    //                                        *c-wsp
    //                                        and-or
    //                                        *c-wsp
    //                                        object-member
    //                                      ) *c-wsp "}"
    //
    //  object-member   = ["?" *c-wsp ] object-item
    //  object-item     = ( rulename / member-rule / group-rule )
    //  and-or          = ( "," / "/" )
    //
    //  array-rule      = "[" *c-wsp array-member *(
    //                                       *c-wsp
    //                                       and-or
    //                                       *c-wsp
    //                                       array-member
    //                                     ) *c-wsp "]"
    //
    //  array-member    = [ array-count *c-wsp ] definition-rule
    //                   [ *c-wsp "/" *c-wsp array-member ]
    //
    //  array-count     = [int] *c-wsp "*" *c-wsp [int]
    //
    //  group-rule      = "(" *c-wsp group-member *(
    //                                        1*c-wsp
    //                                        and-or
    //                                        *c-wsp
    //                                        group-member
    //                                     ) *c-wsp ")"
    //
    //  group-member    = [ ("?" / array-count ) ] *c-wsp definition
    //
    //  directive       = "#" *( VCHAR / WSP / %x7F-10FFFF ) EOL
    //
    // Supported definitions:
    //      #name
    //      #import
    //      #jcr-version
    //      #root
    //      #pedantic
    //      #language-compatible-members
    //      #include
    //
    //  ; Adapted from the ABNF for JSON, RFC 4627 s 2.4
    //  float           = [ "-" ] int [ frac ] [ exp ]
    //  integer         = [ "-" ] int [ exp ]
    //  exp             = ( "e" / "E" ) [ "+" / "-" ] 1*DIGIT
    //  frac            = "." 1*DIGIT
    //  int             = "0" / ( %x31-39 *DIGIT )
    //
    //  ; The regex-char rule allows for any sequence of characters,
    //  ; including whitespace and newlines, with backslash only
    //  ; allowed before either a forward or a backslash.
    //  regex-char      = %x21-2E / %x30-5D / %x5E-7E / WSP /
    //                   CR / LF / "\/" / "\\"
    //
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
    //
    //  ; Taken from the ABNF for ABNF (RFC 4627 section 4) and slightly
    //  ; adapted newlines in a c-wsp do not need whitespace at the
    //  ; start of a newline to form a valid continuation line, and
    //  ; EOL might not be a full CRLF
    //
    //
    //  ; core rules
    //  ALPHA          =  %x41-5A / %x61-7A   ; A-Z / a-z
    //  CR             =  %x0D
    //  DIGIT          =  %x30-39
    //  HEXDIG         =  DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
    //  LF             =  %x0A
    //  VCHAR          =  %x21-7E


bool GrammarParser::error( JCRParser::Status code, const char * p_message )
{
    if( m.status == JCRParser::S_OK )
        m.status = code;
    m.p_parent->error( m.r_reader.get_line_number(), m.r_reader.get_column_number(), code, p_message );
    return false;
}

bool GrammarParser::one_star_c_wsp()
{
    //  c-wsp           = WSP / c-nl
    //  c-nl            = comment / EOL
    //  comment         =  ";" *(WSP / VCHAR) EOL
    //  EOL             = 1*( CR / LF )
    //  WSP            =  SP / HTAB
    //  SP             =  %x20
    //  HTAB           =  %x09

    bool is_found = false;
    while( space() || comment() )
		is_found = true;

    return is_found;
}

bool GrammarParser::comment()
{
    if( peek_is( ';' ) )
    {
        skip_until( cl::alphabet_eol() );
        skip( cl::alphabet_eol() );
        return true;
    }
    return false;
}

bool GrammarParser::star_c_wsp()
{
    return optional( one_star_c_wsp() );
}

} // End of Anonymous namespace

//----------------------------------------------------------------------------
//                           class Directive
//----------------------------------------------------------------------------

void Directive::set( const std::string & r_directive )
{
	using namespace cl::short_alphabets;

	m.directive = r_directive;
	cl::reader_string reader( r_directive );
	cl::dsl_pa line_parser( reader );
	for(;;)
	{
		line_parser.space();
		std::string part;
		line_parser.get( &part, and( not( space() ), not( semicolon() ) ) );
		if( part.empty() )
			break;
		m.parts.push_back( part );
		if( line_parser.current_is( ';' ) )
			break;
	}
}

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
    GrammarParser parser( this, reader, m.p_grammar_set->append_grammar() );

    parser.parse();

    return parser.status();
}

}   // namespace cljcr
