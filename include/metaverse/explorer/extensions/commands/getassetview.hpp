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


#pragma once
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getassetview *************************/

class getassetview: public command_extension
{
public:
    static const char* symbol(){ return "getassetview";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "Show asset owners from MVS blockchain. "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SYMBOL", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
        (
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            "SYMBOL",
            value<std::string>(&argument_.symbol),
            "Asset symbol."
        )
        (
            "limit,l",
            value<uint64_t>(&argument_.limit)->default_value(100),
            "Asset count per page."
        )
        (
            "index,i",
            value<uint64_t>(&argument_.index)->default_value(1),
            "Page index."
        )
        (
            "deposit,d",
            value<bool>(&option_.is_deposit)->default_value(false)->zero_tokens(),
            "If specified, then only get related cert. Default is not specified."
        );

        return options;
    }


    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        std::string symbol;
        uint64_t limit;
        uint64_t index;
    } argument_;

    struct option
    {
         bool is_deposit;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

