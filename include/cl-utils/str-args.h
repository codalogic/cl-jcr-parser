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

#ifndef CLUTIL_STR_ARGS
#define CLUTIL_STR_ARGS

#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <sstream>

namespace clutils {

class str_argsOutOfRangeException : public std::out_of_range
{
public:
    str_argsOutOfRangeException( const char what[] ) : std::out_of_range( what )
    {}
};

class str_args
{
public:
    typedef std::vector< std::string > args_t;

private:
    args_t args;

public:
    str_args()
    {
        args.reserve( 10 );
    }
    str_args( const str_args & r_v )
    {
        args.reserve( 10 );
        *this << r_v;
    }
    str_args( const std::string & r_v )
    {
        args.reserve( 10 );
        *this << r_v;
    }
    str_args( char v )
    {
        args.reserve( 10 );
        *this << v;
    }
    str_args( const char * p_v )
    {
        args.reserve( 10 );
        *this << p_v;
    }
    template< typename T >
    str_args( const T & v )
    {
        args.reserve( 10 );
        *this << v;
    }
    str_args & operator << ( const str_args & r_v )
    {
        for( size_t i=0; i<r_v.args.size(); ++i )
            args.push_back( r_v.args[i] );
        return *this;
    }
    str_args & operator << ( const std::string & r_v )
    {
        args.push_back( r_v );
        return *this;
    }
    str_args & operator << ( char v )
    {
        char str[2];
        str[0] = v;
        str[1] = '\0';
        args.push_back( str );
        return *this;
    }
    str_args & operator << ( const char * p_v )
    {
        args.push_back( p_v );
        return *this;
    }
    template< typename T >
    str_args & operator << ( T v )
    {
        std::ostringstream sos;
        sos << v;
        args.push_back( sos.str() );
        return *this;
    }
    std::string expand( const char * format ) const
    {
        std::string s;
        return *expand_append( &s, format );
    }
    std::string * expand_append( std::string * p_out, const char * format ) const;
};

inline std::string expand( const char format[], const str_args & r_args )
{
    return r_args.expand( format );
}

inline std::string expand( const char format[], const str_args & r_arg_1, const str_args & r_arg_2 )
{
    return expand( format, str_args( r_arg_1 ) << r_arg_2 );
}

inline std::string & expand_append( std::string * p_out, const char format[], const str_args & r_args )
{
    r_args.expand_append( p_out, format );
    return *p_out;
}

inline std::string & expand_append( std::string * p_out, const char format[], const str_args & r_arg_1, const str_args & r_arg_2 )
{
    return expand_append( p_out, format, str_args( r_arg_1 ) << r_arg_2 );
}

}   // namespace clutils

#endif // CLUTIL_STR_ARGS
