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
#include <metaverse/explorer/config/header.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/utility.hpp>

using namespace po;
using namespace bc::config;

namespace libbitcoin {
namespace explorer {
namespace config {

header::header()
  : value_()
{
}

header::header(const std::string& hexcode)
{
    std::stringstream(hexcode) >> *this;
}

header::header(const chain::header& value)
  : value_(value)
{
}

header::header(const header& other)
  : header(other.value_)
{
}

header::operator const chain::header&() const
{
    return value_;
}

std::istream& operator>>(std::istream& input, header& argument)
{
    std::string hexcode;
    input >> hexcode;

    // header base16 is a private encoding in bx, used to pass between commands.
    if (!argument.value_.from_data(base16(hexcode), false))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(hexcode));
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const header& argument)
{
    const auto bytes = argument.value_.to_data(false);

    // header base16 is a private encoding in bx, used to pass between commands.
    output << base16(bytes);
    return output;
}

} // namespace explorer
} // namespace config
} // namespace libbitcoin
