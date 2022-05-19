/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/explorer/config/script.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/utility.hpp>

using namespace po;

namespace libbitcoin {
namespace explorer {
namespace config {

script::script()
  : value_()
{
}

script::script(const std::string& mnemonic)
{
    std::stringstream(mnemonic) >> *this;
}

script::script(const chain::script& value)
  : value_(value)
{
}

script::script(const data_chunk& value)
{
    value_.from_data(value, false,
        chain::script::parse_mode::raw_data_fallback);
}

script::script(const std::vector<std::string>& tokens)
{
    const auto mnemonic = join(tokens);
    std::stringstream(mnemonic) >> *this;
}

script::script(const script& other)
  : script(other.value_)
{
}

const data_chunk script::to_data() const
{
    return value_.to_data(false);
}

const std::string script::to_string() const
{
    return value_.to_string(chain::get_script_context());
}

script::operator const chain::script&() const
{
    return value_;
}

std::istream& operator>>(std::istream& input, script& argument)
{
    std::istreambuf_iterator<char> end;
    std::string mnemonic(std::istreambuf_iterator<char>(input), end);
    boost::trim(mnemonic);

    // Test for invalid result sentinel.
    if (!argument.value_.from_string(mnemonic) && mnemonic.length() > 0)
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(mnemonic));
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const script& argument)
{
    output << argument.value_.to_string(chain::get_script_context());
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
