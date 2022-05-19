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

BOOST_AUTO_TEST_SUITE(network)
BOOST_AUTO_TEST_SUITE(fetch_stealth__invoke)

#if 0
define BX_FETCH_STEALTH_HEIGHT0_PREFIX10101010101_INFO \
"stealth\n" \
"{\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 024f4101000199071b753d68747470733a2f2f6370722e736d2f50647334476956\n" \
"        public_key_hash 65393bf10822868aba54b130914a9aab79246ea4\n" \
"        transaction_hash 2b8024d6d4c1c333ecd8d0d8af44aed676c8624cf1503340b37e9465cad415f5\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 026f7072657475726e2e6e696e6a61202d20707573682073746420747873202877\n" \
"        public_key_hash 6f0d2ae4b13864e3c7b40d300f49a4a90756cd19\n" \
"        transaction_hash 02854c79fb685e2f9551b7c0e5e0074429ecc73aebfc5df558714cdfe6e07467\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 02a9b4d9e2115b3a6d18651f2d5e920a9c5c8b4f54a7882cd75a1b1a93752347a5\n" \
"        public_key_hash efe0fe844abff9370ef3289db0c502eeaf94e712\n" \
"        transaction_hash f6cc0174f09f6583ec42817ba1172030609f418d78598e9bd7b8152be4100f55\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 024f41010001021b753d68747470733a2f2f6370722e736d2f6f647454475a3434\n" \
"        public_key_hash 4b76082816365293791d3083064396a4e2ab1346\n" \
"        transaction_hash 1623d01142f80a2900e7b323ae5a36d864e0bceee85f2018bc601d1721bb82fd\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 02466100000000013ade3ea849d1e743370b2825222e9dd7756c340c5ee524981e\n" \
"        public_key_hash 3d53ff9a82cc6f7e2c794e93615fce0d92fdb7c5\n" \
"        transaction_hash 5dbbcb33be54d2d685a910d36976285c8bd46c294cb87e6e1cd8ec9fe288823b\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 0206c1bb73ed03bcb8bfb55f271e5a1a5c393dc326b98486310615c4e266cf40da\n" \
"        public_key_hash 46287595049649cbc05ea0c263ce59d83999cfc5\n" \
"        transaction_hash 271dca78c3b0a0e0358d6c8b721e2dc365159a7684a4130aaf0b91d711cb820a\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 024afedf40aef29fd5e2796b8f0e1a434d261e3d791839ce06ea7f77d451d45a38\n" \
"        public_key_hash 82813e03530bcccc6b867e7d9dd7b54d2c71b4d8\n" \
"        transaction_hash d50937b02ba1269b3f7c77f4a15fbbfff7080431a42f2ea63545a533ddee925f\n" \
"    }\n" \
"    match\n" \
"    {\n" \
"        ephemeral_public_key 024661000000000e235bed1e6b0ecaea8348469bb84815ccef13fdc77a58934f41\n" \
"        public_key_hash 3d53ff9a82cc6f7e2c794e93615fce0d92fdb7c5\n" \
"        transaction_hash 3f1a14eadb4ef34038b68333b0945aff44cb9dc3aecc95e42a3c3cf55178b25e\n" \
"    }\n" \
"}\n"
#endif

// requires libbitcoin-server.
BOOST_AUTO_TEST_CASE(fetch_stealth__invoke_mainnet_height_0__okay)
{
    BX_DECLARE_NETWORK_COMMAND(fetch_stealth);

    // We need to set a high filter becuase the reference server restricts response size.
    command.set_filter_argument({ "1010101010101010" });
    command.set_server_url_setting({ BX_MAINNET_SERVER });
    BX_REQUIRE_OKAY(command.invoke(output, error));
}

// requires libbitcoin-server.
BOOST_AUTO_TEST_CASE(fetch_stealth__invoke_mainnet_height_400000__okay)
{
    BX_DECLARE_NETWORK_COMMAND(fetch_stealth);
    command.set_height_option(400000);

    // We need to set a high filter becuase the reference server restricts response size.
    command.set_filter_argument({ "1010101010101010" });
    command.set_server_url_setting({ BX_MAINNET_SERVER });
    BX_REQUIRE_OKAY(command.invoke(output, error));
}

////// requires libbitcoin-server.
////// This test is fragile, will eventually break (due to expansion of the returned result).
////BOOST_AUTO_TEST_CASE(fetch_stealth__invoke_mainnet_height_0_prefix_10101010101__okay_output)
////{
////    BX_DECLARE_NETWORK_COMMAND(fetch_stealth);
////    command.set_format_option({ "info" });
////    command.set_height_option(0);
////    command.set_filter_argument({ "10101010101" });
////    command.set_server_url_setting({ BX_MAINNET_SERVER });
////    BX_REQUIRE_OKAY(command.invoke(output, error));
////    BX_REQUIRE_OUTPUT(BX_FETCH_STEALTH_HEIGHT0_PREFIX10101010101_INFO);
////}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
