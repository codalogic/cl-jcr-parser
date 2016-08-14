//----------------------------------------------------------------------------
// Copyright (c) 2016, Codalogic Ltd (http://www.codalogic.com)
// All rights reserved.
//
// The license for this file is based on the BSD-3-Clause license
// (http://www.opensource.org/licenses/BSD-3-Clause).
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// - Neither the name Codalogic Ltd nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------

#include "cl-utils/str-args.h"

#include <cassert>
#include <algorithm>

namespace clutils {

namespace { // Implementation detail

class str_args_detail
{
private:
    size_t i;
    std::string * p_result;
    const char * format;
    const str_args::args_t & args;

public:
    str_args_detail( std::string * p_out, const char * format, const str_args::args_t & args )
        :
        p_result( p_out ),
        format( format ),
        args( args )
    {
        try
        {
            p_result->reserve( p_result->size() + strlen( format ) );
            for( i=0; format[i] != '\0'; ++i )
            {
                if( format[i] != '%' )
                    p_result->append( 1, format[i] );
                else
                    process_parameter_decl();
            }
        }

        catch( std::out_of_range & )
        {
            assert( 0 ); // We've done something like %1 in our args string when args[1] doesn't exist.  N.B. args are 0 based.

            throw str_argsOutOfRangeException( "str_args args[] index out of range" );
        }
    }

    void process_parameter_decl()
    {
        ++i;
        if( format[i] == '\0' )     // Malformed case, but be tolerant
            p_result->append( 1, '%' );
        else if( format[i] == '%' )  // %% -> %
            p_result->append( 1, '%' );
        else if( is_numerical_parameter() )
            p_result->append( args.at( format[i] - '0' ) );
        else if( format[i] == '{' )
            process_long_form_parameter_decl();
        else
            silently_accept_standalone_parameter_indicator();
    }

    bool is_numerical_parameter()
    {
        return format[i] >= '0' && format[i] <= '9';
    }

    void process_long_form_parameter_decl()
    {
        ++i;
        if( format[i] != '\0' )
        {
            if( is_numerical_parameter() )
                process_numbered_long_form_parameter_decl();
            else
                process_named_long_form_parameter_decl();
        }
    }

    void process_numbered_long_form_parameter_decl()
    {
        // Long form %{0:a description}
        size_t index = read_numerical_parameter_index();
        p_result->append( args.at( index ) );
        skip_remainder_of_parameter_decl();
    }

    size_t read_numerical_parameter_index()
    {
        size_t index = 0;
        while( is_numerical_parameter() )
            index = 10 * index + format[i++] - '0';
        return index;
    }

    void skip_remainder_of_parameter_decl()
    {
        for( ; format[i] != '\0' && format[i] != '}'; ++i )
        {}  // Skip over rest of characters in format specifier
    }

    void process_named_long_form_parameter_decl()
    {
        // Named long form %{var-name}
        std::string name = read_parameter_name();
        str_args::args_t::const_iterator key_index = std::find( args.begin(), args.end(), name );
        if( key_index != args.end() )
            p_result->append( *(++key_index) );
        skip_remainder_of_parameter_decl();
    }

    std::string read_parameter_name()
    {
        std::string name;
        for( ; format[i] != '\0' && format[i] != '}'; ++i )
            name.append( 1, format[i] );
        return name;
    }

    void silently_accept_standalone_parameter_indicator()
    {
        p_result->append( 1, '%' );
        p_result->append( 1, format[i] );
    }

    std::string * result() const { return p_result; }
};

} // End of namespace { // Implementation detail

std::string * str_args::expand_append( std::string * p_out, const char * format ) const
{
    return str_args_detail( p_out, format, args ).result();
}

}   // namespace clutils
