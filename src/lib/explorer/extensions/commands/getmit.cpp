/**
 * Copyright (c) 2016-2018 mvs developers
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/getmit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

/************************ getmit *************************/

console_result getmit::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    if (!argument_.symbol.empty()) {
        // check symbol
        check_mit_symbol(argument_.symbol);
    }

    if (option_.show_current) {
        if (argument_.symbol.empty()) {
            throw argument_legality_exception("MIT symbol not privided while displaying the current status of MIT!");
        }
    }

    if (option_.show_history) {
        if (argument_.symbol.empty()) {
            throw argument_legality_exception("MIT symbol not privided while tracing history!");
        }

        // page limit & page index paramenter check
        if (option_.index < 1) {
            throw argument_legality_exception{"page index parameter cannot be zero"};
        }

        if (option_.limit < 1) {
            throw argument_legality_exception{"page record limit parameter cannot be zero"};
        }

        if (option_.limit > 100) {
            throw argument_legality_exception{"page record limit cannot be bigger than 100."};
        }
    }

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    bool is_list = true;
    if (argument_.symbol.empty()) {
        auto sh_vec = blockchain.get_registered_mits();
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem : *sh_vec) {
            json_value.append(elem.mit.get_symbol());
        }

        if (get_api_version() <=2 ) {
            jv_output["mits"] = json_value;
        }
        else {
            jv_output = json_value;
        }
    }
    else {
        if (option_.show_history) {
            auto sh_vec = blockchain.get_mit_history(argument_.symbol, option_.limit, option_.index);
            for (auto& elem : *sh_vec) {
                Json::Value asset_data = json_helper.prop_list(elem);
                json_value.append(asset_data);
            }

            if (get_api_version() <=2 ) {
                jv_output["mits"] = json_value;
            }
            else {
                if(json_value.isNull())
                    json_value.resize(0);  

                jv_output = json_value;
            }
        }
        else {
            if (option_.show_current) {
                auto sh_vec = blockchain.get_mit_history(argument_.symbol, 1, 1);
                if (nullptr != sh_vec && sh_vec->size() > 0) {
                    auto last_iter = --(sh_vec->end());
                    auto& mit_info = *last_iter;
                    auto reg_mit = blockchain.get_registered_mit(argument_.symbol);
                    if (nullptr != reg_mit) {
                        mit_info.mit.set_content(reg_mit->mit.get_content());
                    }

                    json_value = json_helper.prop_list(mit_info, true);
                }
            }
            else {
                auto mit_info = blockchain.get_registered_mit(argument_.symbol);
                if (nullptr != mit_info) {
                    json_value = json_helper.prop_list(*mit_info);
                }
            }

            jv_output = json_value;
        }
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

