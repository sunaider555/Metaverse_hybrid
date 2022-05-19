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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

BX_USING_NAMESPACES()

BOOST_AUTO_TEST_SUITE(network)
BOOST_AUTO_TEST_SUITE(fetch_balance__invoke)

#ifndef ENABLE_TESTNET

// These amounts may change at any time, making these particular tests fragile.

/**
// The 50BTC coinbase in the genesis block cannot be confirmed.
#define BX_FETCH_BALANCE_FIRST_ADDRESS_INFO \
"balance\n" \
"{\n" \
"    address 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa\n" \
"    confirmed 1538241483\n" \
"    received 6538241483\n" \
"    unspent 6538241483\n" \
"}\n"
*/

// Vector: github.com/spesmilo/sx/blob/master/README.md
#define BX_FETCH_BALANCE_SX_DEMO1_ADDRESS "134HfD2fdeBTohfx8YANxEpsYXsv5UoWyz"
#define BX_FETCH_BALANCE_SX_DEMO1_XML \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" \
"<balance><address>134HfD2fdeBTohfx8YANxEpsYXsv5UoWyz</address><confirmed>0</confirmed><received>100000</received><unspent>0</unspent></balance>\n"

// Vector: github.com/spesmilo/sx/blob/master/README.md
#define BX_FETCH_BALANCE_SX_DEMO2_ADDRESS "13Ft7SkreJY9D823NPm4t6D1cBqLYTJtAe"
#define BX_FETCH_BALANCE_SX_DEMO2_XML \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" \
"<balance><address>13Ft7SkreJY9D823NPm4t6D1cBqLYTJtAe</address><confirmed>90000</confirmed><received>90000</received><unspent>90000</unspent></balance>\n"

// This test disabled because people are always sending change to Satoshi.
//BOOST_AUTO_TEST_CASE(fetch_balance__invoke__mainnet_first_address_info__okay_output)
//{
//    BX_DECLARE_NETWORK_COMMAND(fetch_balance);
//    command.set_format_option({ "info" });
//    command.set_payment_address_argument({ { BX_FIRST_ADDRESS } });
//    BX_REQUIRE_OKAY(command.invoke(output, error));
//    BX_REQUIRE_OUTPUT(BX_FETCH_BALANCE_FIRST_ADDRESS_INFO);
//}

BOOST_AUTO_TEST_CASE(fetch_balance__invoke__mainnet_sx_demo1_xml__okay_output)
{
    BX_DECLARE_NETWORK_COMMAND(fetch_balance);
    command.set_format_option({ "xml" });
    command.set_payment_address_argument({ BX_FETCH_BALANCE_SX_DEMO1_ADDRESS });
    BX_REQUIRE_OKAY(command.invoke(output, error));
    BX_REQUIRE_OUTPUT(BX_FETCH_BALANCE_SX_DEMO1_XML);
}

BOOST_AUTO_TEST_CASE(fetch_balance__invoke__mainnet_sx_demo2_xml__okay_output)
{
    BX_DECLARE_NETWORK_COMMAND(fetch_balance);
    command.set_format_option({ "xml" });
    command.set_payment_address_argument({ BX_FETCH_BALANCE_SX_DEMO2_ADDRESS });
    BX_REQUIRE_OKAY(command.invoke(output, error));
    BX_REQUIRE_OUTPUT(BX_FETCH_BALANCE_SX_DEMO2_XML);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
