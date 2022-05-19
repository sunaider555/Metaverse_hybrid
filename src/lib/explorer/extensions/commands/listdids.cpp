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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listdids.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listdids *************************/

console_result listdids::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    // page limit & page index paramenter check
    if (argument_.index <= 0) {
        throw argument_legality_exception{"page index parameter cannot be zero"};
    }
    if (argument_.limit <= 0) {
        throw argument_legality_exception{"page record limit parameter cannot be zero"};
    }
    if (argument_.limit > 100) {
        throw argument_legality_exception{"page record limit cannot be bigger than 100."};
    }

    auto& blockchain = node.chain_impl();
    std::shared_ptr<chain::did_detail::list> sh_vec;
    if (auth_.name.empty()) {
        // no account -- list all dids in blockchain
        sh_vec = blockchain.get_registered_dids();
    }
    else {
        // list dids owned by the account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        sh_vec = blockchain.get_account_dids(auth_.name);
    }

    uint64_t limit = argument_.limit;
    uint64_t index = argument_.index;

    std::vector<chain::did_detail> result;
    uint64_t total_count = sh_vec-> size();
    uint64_t total_page = 0;
    if (total_count > 0) {
        std::sort(sh_vec->begin(), sh_vec->end());

        uint64_t start = 0, end = 0, tx_count = 0;
        if (index && limit) {
            total_page = (total_count % limit) ? (total_count / limit + 1) : (total_count / limit);
            index = index > total_page ? total_page : index;
            start = (index - 1) * limit;
            end = index * limit;
            tx_count = end >= total_count ? (total_count - start) : limit ;
        }
        else if (!index && !limit) { // all tx records
            start = 0;
            tx_count = total_count;
            index = 1;
            total_page = 1;
        }
        else {
            throw argument_legality_exception{"invalid limit or index parameter"};
        }

        if (start < total_count && tx_count > 0) {
            result.resize(tx_count);
            std::copy(sh_vec->begin() + start, sh_vec->begin() + start + tx_count, result.begin());
        }
    }

    Json::Value dids;
    // add blockchain dids
    for (auto& elem: result) {
        Json::Value did_data;
        did_data["symbol"] = elem.get_symbol();
        did_data["address"] = elem.get_address();
        did_data["status"] = "registered";
        dids.append(did_data);
    }

    // output
    if (dids.isNull()) {
        dids.resize(0);
    }

    jv_output["total_count"] = total_count;
    jv_output["total_page"] = total_page;
    jv_output["current_page"] = index;
    jv_output["dids"] = dids;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
