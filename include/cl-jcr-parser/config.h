//----------------------------------------------------------------------------
// Copyright (c) 2015-2017, Codalogic Ltd (http://www.codalogic.com)
//
// This Source Code is subject to the terms of the GNU LESSER GENERAL PUBLIC
// LICENSE version 3. If a copy of the LGPLv3 was not distributed with
// this file, you can obtain one at http://opensource.org/licenses/LGPL-3.0.
//----------------------------------------------------------------------------

#ifndef CL_JCR_PARSER__CONFIG
#define CL_JCR_PARSER__CONFIG

#include "cl-utils/ptr-vector.h"

#include <string>

namespace cljcr {

class Config
{
    typedef clutils::ptr_vector< std::string > jcr_file_list_t;

    struct Members
    {
        jcr_file_list_t jcr_file_list;
        std::string json_to_validate;
    } m;

public:
    void add_jcr( const std::string & jcr_file ) { m.jcr_file_list.push_back( jcr_file ); }
    bool has_jcr() const { return ! m.jcr_file_list.empty(); }
    size_t jcr_size() const { return m.jcr_file_list.size(); }
    const std::string & jcr( size_t index ) { return m.jcr_file_list[index]; }

    void set_json( const std::string & json_file ) { m.json_to_validate = json_file; }
    bool has_json() const { return ! m.json_to_validate.empty(); }
    const std::string & json() const { return m.json_to_validate; }
};

}   // namespace cljcr

#endif // CL_JCR_PARSER__CONFIG
