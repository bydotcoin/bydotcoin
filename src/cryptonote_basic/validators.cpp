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

#include "validators.h"

#undef BYDOTCOIN_DEFAULT_LOG_CATEGORY
#define BYDOTCOIN_DEFAULT_LOG_CATEGORY "Validators"

namespace bydotcoin {
    namespace basic {
        Validator::Validator(const std::string &publicKey, uint64_t startHeight, uint64_t endHeight, string name, string domain, string page_link)
                : publicKey(publicKey), startHeight(startHeight), endHeight(endHeight), name(name), domain(domain), page_link(page_link) {}

        Validator::Validator() = default;

        list_update_outcome Validators::validate_and_update(bydotcoin::basic::v_list_struct res, bool saveToDB, bool isEmergencyUpdate) {

          MGINFO("Loading Validator List...");
          LOG_PRINT_L2("Validator List Struct received: " << store_t_to_json(res));

          std::vector<std::string> testnet_vl_publicKeys = {"443ED0A8172A8A79E606771679F06BD5A9AAFC65F01C8BB94A1A01257393A96E",
                                    "9C2F74BE7292BD9DE1DBF15667E48778F3971EEBB85FFCB265B8D03356A5F9C2",
                                    "171BE7497D81281C36E62D865BFDA8C86CC76171580CB50FDA4C64C14C184773"};

          std::vector<std::string> mainnet_vl_publicKeys = {"814A92F191735D989FFD3A2A7B33A2EE3ED6AD746B1530AED8E91E3B259DCD4B",
                                   "38BBE01388170750FAF8FE9B9C31DF6432987283F49171DA86039566C3288BF9",
                                   "8BC9D71CE4CD0DE0D50F45C8619257399B108D35C8CBB72FC29B38DFE3847769"};

          std::vector<std::string> vl_publicKeys = this->testnet ? testnet_vl_publicKeys : mainnet_vl_publicKeys;

          //Check against our hardcoded public-keys to make sure it's a valid message
          if (res.pubkeys.size() != vl_publicKeys.size()) {
            LOG_PRINT_L1("Validator list has too few public keys.");
            return list_update_outcome::Invalid_Etn_Pubkey;
          }

          if (res.signatures.size() != vl_publicKeys.size()) {
            LOG_PRINT_L1("Validator list has too few signatures.");
            return list_update_outcome::Invalid_Sig;
          }

          //Check against our hardcoded public-keys to make sure it's a valid message
          if (res.pubkeys != vl_publicKeys) {
            LOG_PRINT_L1("Validator list has one or more invalid public keys.");
            return list_update_outcome::Invalid_Etn_Pubkey;
          }

          //We sign our validator lists with multiple keys for security purposes.
          for (unsigned int i = 0; i < vl_publicKeys.size(); ++i){
              if(!crypto::verify_signature(res.blob, unhex(string(vl_publicKeys[i])), unhex(string(res.signatures[i])))){
                  LOG_PRINT_L1("Validator list has an invalid signature and will be ignored.");
                  return list_update_outcome::Invalid_Sig;
              }
          }

          LOG_PRINT_L2("Validator List received: " << crypto::base64_decode(res.blob));

          LOG_PRINT_L2("BEFORE");
          int v_counter = 0;
          all_of(this->list.begin(), this->list.end(), [&v_counter](std::unique_ptr<Validator> &v) {
              LOG_PRINT_L2("Validator " << v_counter << " (" << v->getName() << ") :: PublicKey=" << v->getPublicKey() << ",\n FromHeight=" << v->getStartHeight() << ", ToHeight=" << v->getEndHeight());
              v_counter++;
              return true;
          });

          json_obj obj;
          load_t_from_json(obj, crypto::base64_decode(res.blob));

          if(obj.list_timestamp < this->current_list_timestamp) {

            this->last_updated = time(nullptr);
            this->status = ValidatorsState::Valid;
            
            LOG_PRINT_L1("Validator list received is older than our local list.");

            if(isEmergencyUpdate && (std::time(nullptr) - obj.list_timestamp < 18000)){
                return list_update_outcome::Recent_Emergency_List;
            }else {
                return list_update_outcome::Old_List;
            }
          } else if(obj.list_timestamp == this->current_list_timestamp) {

            this->last_updated = time(nullptr);
            this->status = ValidatorsState::Valid;

            LOG_PRINT_L1("Validator list received has the same timestamp than our local list.");
            if(isEmergencyUpdate && (std::time(nullptr) - obj.list_timestamp < 18000)){
                return list_update_outcome::Same_Emergency_List;
            }
            else return list_update_outcome::Same_List;
          }

          uint8_t anon_v_count = 0;
          MGINFO_MAGENTA("Public Validators:");
          for (const auto &v : obj.validators) {
            this->addOrUpdate(v.validation_public_key, v.valid_from_height, v.valid_to_height, v.name, v.domain, v.page_link);

            if(!v.name.empty()) {
              MGINFO(v.name << " | Public Key: " << v.validation_public_key);
            } else {
              anon_v_count++;
            }
          }
          MGINFO_MAGENTA("Anon Validators: " << to_string(anon_v_count));

          LOG_PRINT_L2("AFTER");
          v_counter = 0;
          all_of(this->list.begin(), this->list.end(), [&v_counter](std::unique_ptr<Validator> &v) {
            LOG_PRINT_L2("Validator " << v_counter << " (" << v->getName() << ") :: PublicKey=" << v->getPublicKey() << ",\n FromHeight=" << v->getStartHeight() << ", ToHeight=" << v->getEndHeight());
            v_counter++;
            return true;
          });

          //Serialize & save valid http response to propagate to p2p upon request
          this->serialized_v_list = store_t_to_json(res);
          this->last_updated = time(nullptr);
          this->current_list_timestamp = obj.list_timestamp;
          this->status = ValidatorsState::Valid;

          if(saveToDB) {
            m_db.set_validator_list(this->serialized_v_list, this->last_updated + this->timeout);
          }

          // Only relay emergency lists within a 5 hour window to prevent p2p spam.
          // Regular list updates at the endpoint will be time-stamped so that they cannot fall into this timezone
          // and be propagated by spammers as if they were emergency lists.
          if (isEmergencyUpdate && (std::time(nullptr) - obj.list_timestamp < 18000)){
              return list_update_outcome::Emergency_Success;
          }
          return list_update_outcome::Success;
        }

        void Validators::add(const string &key, uint64_t startHeight, uint64_t endHeight, string name, string domain, string page_link) {
          if (!this->exists(key)) this->list.emplace_back(std::unique_ptr<Validator>(new Validator(key, startHeight, endHeight, name, domain, page_link)));
        }

        void Validators::addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight, string name, string domain, string page_link) {
          this->exists(key) ? this->update(key, endHeight, name, domain, page_link) : this->list.emplace_back(
                  std::unique_ptr<Validator>(new Validator(key, startHeight, endHeight, name, domain, page_link)));
        }

        std::unique_ptr<Validator> Validators::find(const string &key) {
          auto it = find_if(this->list.begin(), this->list.end(), [&key](std::unique_ptr<Validator> &v) {
              return v->getPublicKey() == key;
          });
          return std::move(*it);
        }

        bool Validators::exists(const string &key) {
          bool found = false;
          all_of(this->list.begin(), this->list.end(), [&key, &found](std::unique_ptr<Validator> &v) {
              if (v->getPublicKey() == key) {
                found = true;
                return false;
              }
              return true;
          });
          return found;
        }

        void Validators::update(const string &key, uint64_t endHeight, string name, string domain, string page_link) {
          find_if(this->list.begin(), this->list.end(), [&key, &endHeight, &name, &domain, &page_link](std::unique_ptr<Validator> &v) {
              if (v->getPublicKey() == key) {
                v->setEndHeight(endHeight);
                v->setName(name);
                v->setDomain(domain);
                v->setPageLink(page_link);
                return true;
              }
              return false;
          });
        }

        ValidatorsState Validators::validate_expiration() {
          if((time(nullptr) - this->last_updated) >= this->timeout && this->status == ValidatorsState::Valid) {
            this->status = ValidatorsState::NeedsUpdate;
          }

          if((time(nullptr) - this->last_updated) >= this->timeout + this->timeout_grace_period && this->status == ValidatorsState::NeedsUpdate) {
            this->status = ValidatorsState::Expired;
          }

          return this->status;
        }
    }
}