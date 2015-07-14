//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#include "cl-jcr-parser/cl-jcr-parser.h"

#include "dsl-pa/dsl-pa.h"

namespace cljcr {

namespace { // Anonymous namespace for detail

class JCRP : public cl::dsl_pa
{
private:
    struct Members {
        JCRParser * p_parent;
        const char * p_file_name;
        cl::reader_file & r_reader;
        Grammar * p_grammar;
        JCRParser::Status status;

        Members( JCRParser * p_parent_in,
                const char * p_file_name_in,
                cl::reader_file & r_reader_in,
                Grammar * p_grammar_in )
            :
            p_parent( p_parent_in ),
            p_file_name( p_file_name_in ),
            r_reader( r_reader_in ),
            p_grammar( p_grammar_in ),
            status( JCRParser::S_OK )
        {}
    } m;

public:
    JCRP( JCRParser * p_parent,
            const char * p_file_name,
            cl::reader_file & r_reader,
            Grammar * p_grammar )
        :
        cl::dsl_pa( r_reader ),
        m( p_parent, p_file_name, r_reader, p_grammar )
    {}
    bool parse();
    void error( size_t line, const char * p_message );
};

bool JCRP::parse()
{
    //  grammar         = 1*( *c-wsp (rule / directive) ) *c-wsp

    return m.status == JCRParser::S_OK;
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
    //  c-wsp           = WSP / c-nl
    //  c-nl            = comment / EOL
    //  comment         =  ";" *(WSP / VCHAR) EOL
    //  EOL             = 1*( CR / LF )
    //
    //  ; core rules
    //  ALPHA          =  %x41-5A / %x61-7A   ; A-Z / a-z
    //  CR             =  %x0D
    //  DIGIT          =  %x30-39
    //  HEXDIG         =  DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
    //  LF             =  %x0A
    //  VCHAR          =  %x21-7E
    //  WSP            =  SP / HTAB
    //  SP             =  %x20
    //  HTAB           =  %x09

}   // namespace cljcr

} // End of Anonymous namespace

namespace cljcr {

JCRParser::Status JCRParser::parse()
{
    cl::reader_file reader( m.p_file_name );
    if( ! reader.is_open() )
		return S_UNABLE_TO_OPEN_FILE;

    JCRP parser( this, m.p_file_name, reader, m.p_grammar );

    return m.status;
}

}   // namespace cljcr
