// Copyrights(c) 2017-2020, The Electroneum Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef ELECTRONEUM_VALIDATORS_COMMANDS_DEFS_H
#define ELECTRONEUM_VALIDATORS_COMMANDS_DEFS_H

#include "include_base_utils.h"
#include "storages/http_abstract_invoke.h"

namespace bydotcoin {
    namespace basic {

        struct json_obj {
            struct validator {
                std::string validation_public_key;
                uint64_t valid_from_height;
                uint64_t valid_to_height;
                std::string name;
                std::string domain;
                std::string page_link;

                BEGIN_KV_SERIALIZE_MAP()
                KV_SERIALIZE(validation_public_key)
                KV_SERIALIZE(valid_from_height)
                KV_SERIALIZE(valid_to_height)
                KV_SERIALIZE(name)
                KV_SERIALIZE(domain)
                KV_SERIALIZE(page_link)
                END_KV_SERIALIZE_MAP()
            };

            std::vector<validator> validators;
            std::uint64_t list_timestamp;

            BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(validators)
            KV_SERIALIZE(list_timestamp)
            END_KV_SERIALIZE_MAP()
        };

        struct v_list_struct_request {
            BEGIN_KV_SERIALIZE_MAP()
            END_KV_SERIALIZE_MAP()
        };

        struct v_list_struct {

            std::string blob;
            int version = 0;
            std::vector<std::string> signatures;
            std::vector<std::string> pubkeys;

            BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(blob)
            KV_SERIALIZE(version)
            KV_SERIALIZE(signatures)
            KV_SERIALIZE(pubkeys)
            END_KV_SERIALIZE_MAP()
        };
    }
}

#endif //ELECTRONEUM_VALIDATORS_COMMANDS_DEFS_H
