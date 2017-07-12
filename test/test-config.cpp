//----------------------------------------------------------------------------
// Copyright (c) 2015-2017, Codalogic Ltd (http://www.codalogic.com)
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

#include "clunit.h"

#include "cl-jcr-parser/config.h"

using namespace cljcr;

TFEATURE( "Config - Configuration" )
{
    Config config;

    TTEST( config.has_jcr() == false );
    TTEST( config.jcr_size() == 0 );
    TTEST( config.has_json() == false );

    config.set_json( "JSON" );
    TTEST( config.has_jcr() == false );
    TTEST( config.jcr_size() == 0 );
    TTEST( config.has_json() == true );
    TTEST( config.json() == "JSON" );

    config.add_jcr( "JCR-0" );
    TTEST( config.has_jcr() == true );
    TTEST( config.jcr_size() == 1 );
    TTEST( config.jcr( 0 ) == "JCR-0" );
    TTEST( config.has_json() == true );
    TTEST( config.json() == "JSON" );

    config.add_jcr( "JCR-1" );
    TTEST( config.has_jcr() == true );
    TTEST( config.jcr_size() == 2 );
    TTEST( config.jcr( 0 ) == "JCR-0" );
    TTEST( config.jcr( 1 ) == "JCR-1" );
    TTEST( config.has_json() == true );
    TTEST( config.json() == "JSON" );
}
