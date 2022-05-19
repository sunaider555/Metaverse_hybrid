/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-explorer.
 *
 * libbitcoin-explorer is free software: you can redistribute it and/or
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

#include "command.hpp"

BX_USING_NAMESPACES()

BOOST_AUTO_TEST_SUITE(offline)
BOOST_AUTO_TEST_SUITE(ec_add__invoke)

// TODO: generate overflow condition

//BOOST_AUTO_TEST_CASE(ec_add__invoke__overflow__failure_error)
//{
//    BX_DECLARE_COMMAND(commands::ec_add);
//    command.set_point_argument({ "02ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" });
//    command.set_secret_argument({ "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" });
//    BX_REQUIRE_FAILURE(command.invoke(output, error));
//    BX_REQUIRE_ERROR(BX_EC_ADD_OUT_OF_RANGE "\n");
//}

BOOST_AUTO_TEST_CASE(ec_add__invoke__add_values__okay_output)
{
    BX_DECLARE_COMMAND(commands::ec_add);
    command.set_point_argument({ "021bab84e687e36514eeaf5a017c30d32c1f59dd4ea6629da7970ca374513dd006" });
    command.set_secret_argument({ "1bab84e687e36514eeaf5a017c30d32c1f59dd4ea6629da7970ca374513dd006" });
    BX_REQUIRE_OKAY(command.invoke(output, error));
    BX_REQUIRE_OUTPUT("0398dbb46779e5e82fc7422c874e2390f3e076500410481bd129927f78cfc455ac\n");
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
