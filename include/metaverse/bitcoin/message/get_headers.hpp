/*
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
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
#ifndef MVS_MESSAGE_GET_HEADERS_HPP
#define MVS_MESSAGE_GET_HEADERS_HPP

#include <istream>
#include <memory>
#include <string>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/message/get_blocks.hpp>

namespace libbitcoin {
namespace message {

class BC_API get_headers
  : public get_blocks
{
public:
    typedef std::shared_ptr<get_headers> ptr;

    static get_headers factory_from_data(uint32_t version,
        const data_chunk& data);
    static get_headers factory_from_data(uint32_t version,
        std::istream& stream);
    static get_headers factory_from_data(uint32_t version, reader& source);

    get_headers();
    get_headers(const hash_list& start, const hash_digest& stop);
    get_headers(hash_list&& start, hash_digest&& stop);

    bool from_data(uint32_t version, const data_chunk& data) override;
    bool from_data(uint32_t version, std::istream& stream) override;
    bool from_data(uint32_t version, reader& source) override;

    static const std::string command;
    static const uint32_t version_minimum;
    static const uint32_t version_maximum;
};

} // end message
} // end libbitcoin

#endif
