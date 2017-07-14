//----------------------------------------------------------------------------
// Copyright (c) 2017, Codalogic Ltd (http://www.codalogic.com)
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

//----------------------------------------------------------------------------
// CommandLineArgs is a cheap and cheerful utility class to help make
// processing command line arguments passed to main() easier.
//----------------------------------------------------------------------------

#ifndef CL_UTILS__COMMAND_LINE_ARGS
#define CL_UTILS__COMMAND_LINE_ARGS

#include <cstring>
#include <iostream>

namespace clutils {

class CommandLineArgs
{
private:
    int argc;
    char ** argv;

public:
    CommandLineArgs( int argc, char ** argv ) : argc( argc ), argv( argv )
    {
        next();
    }

    bool is_flag() const      // Is a flag as opposed to a value etc
    {
        return **argv == '-';
    }
    bool is_flag( const char * p_option_1 ) const  // Is a specified flag
    {
        return strcmp( p_option_1, *argv ) == 0;
    }
    bool is_flag( const char * p_option_1, int desired_extra_count, const char * p_on_insufficient_message = 0 )  // Is a specified flag
    {
        if( is_flag( p_option_1 ) )
            return ensure( desired_extra_count, p_on_insufficient_message );
        return false;
    }
    bool is_flag( const char * p_option_1, const char * p_option_2 ) const
    {
        return strcmp( p_option_1, *argv ) == 0 || strcmp( p_option_2, *argv ) == 0;
    }
    bool is_flag( const char * p_option_1, const char * p_option_2, int desired_extra_count, const char * p_on_insufficient_message = 0 )
    {
        if( is_flag( p_option_1, p_option_2 ) )
            return ensure( desired_extra_count, p_on_insufficient_message );
        return false;
    }
    bool ensure( int desired_extra_count, const char * p_on_insufficient_message = 0 )
    {
        if( (argc - 1) < desired_extra_count )
        {
            if( p_on_insufficient_message )
                std::cout << p_on_insufficient_message << "\n";
            else
                std::cout << *argv << " requires " << desired_extra_count <<
                        " additional parameter" << ((desired_extra_count == 1)?"":"s") << "\n";
            argc = 0;   // Stop trying to process remaining insufficient set of arguments
            return false;
        }
        return true;
    }
    const char * current() const { return *argv; }
    const char * next() { if( argc > 0 ) { ++argv; --argc; } return *argv; }
    void operator ++ () { next(); }
    void operator ++ (int) { next(); }
    bool empty() const { return argc <= 0; }
    operator bool () { return ! empty(); }
};

} // namespace clutils

#endif // CL_UTILS__COMMAND_LINE_ARGS
