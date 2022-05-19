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


/************************ issue *************************/
class issue: public command_extension
{
public:
    static const char* symbol(){ return "issue";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "Broadcast the asset whole network."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("SYMBOL", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
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
            "ACCOUNTNAME",
            value<std::string>(&auth_.name)->required(),
            BX_ACCOUNT_NAME
        )
        (
            "ACCOUNTAUTH",
            value<std::string>(&auth_.auth)->required(),
            BX_ACCOUNT_AUTH
        )
        (
            "SYMBOL",
            value<std::string>(&argument_.symbol)->required(),
            "The asset symbol, global uniqueness, only supports UPPER-CASE alphabet and dot(.)"
        )
        (
            "model,m",
            value<std::string>(&option_.attenuation_model_param)->default_value(""),
            BX_MST_OFFERING_CURVE
        )
        (
            "subsidy,s",
            value<std::string>(&option_.mining_subsidy_param)->default_value(""),
            "The block subsidy parameters for mining."
            " The format is \"initial:unit32,interval:unit32,base:float\"."
            " The subsidy of block is calcuted by: initial * pow(base, (block_height/interval))"
            " Default is empty which means mining is not allowed."
        )
        (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(10 * 100000000),
            "The fee of tx. minimum is 10 etp."
        )
        (
            "percentage,p",
            value<uint32_t>(&argument_.percentage)->default_value(20),
            "Percentage of fee send to miner. minimum is 20."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        std::string symbol;
        uint64_t fee;
        uint32_t percentage;
    } argument_;

    struct option
    {
        std::string attenuation_model_param;
        std::string mining_subsidy_param;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

