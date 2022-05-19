/**
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
#ifndef MVS_CHAIN_EVALUATION_CONTEXT_HPP
#define MVS_CHAIN_EVALUATION_CONTEXT_HPP

#include <algorithm>
#include <cstdint>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include "conditional_stack.hpp"

namespace libbitcoin {
namespace chain {

class evaluation_context
{
public:
    data_chunk pop_stack();

    operation::stack::const_iterator code_begin;
    uint64_t operation_counter;
    data_stack stack;
    data_stack alternate;
    conditional_stack conditional;
    uint32_t flags;
};

} // namspace chain
} // namspace libbitcoin

#endif
