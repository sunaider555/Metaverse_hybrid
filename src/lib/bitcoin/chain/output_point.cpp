/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/chain/output_point.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <metaverse/bitcoin/math/limits.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/chain/point.hpp>

namespace libbitcoin {
namespace chain {

// Constructors.
//-----------------------------------------------------------------------------

output_point::output_point()
  : point{}
{
}

output_point::output_point(point&& value)
  : point(std::move(value))
{
}

output_point::output_point(const point& value)
  : point(value)
{
}

output_point::output_point(const output_point& other)
  : point(other)
{
}

output_point::output_point(output_point&& other)
  : point(std::move(other))
{
}

output_point::output_point(hash_digest&& hash, uint32_t index)
  : point({ std::move(hash), index })
{
}

output_point::output_point(const hash_digest& hash, uint32_t index)
  : point(hash, index)
{
}

// Operators.
//-----------------------------------------------------------------------------

output_point& output_point::operator=(point&& other)
{
    reset();
    point::operator=(std::move(other));
    return *this;
}

output_point& output_point::operator=(const point& other)
{
    reset();
    point::operator=(other);
    return *this;
}

output_point& output_point::operator=(output_point&& other)
{
    point::operator=(std::move(other));
    return *this;
}

output_point& output_point::operator=(const output_point& other)
{
    point::operator=(other);
    return *this;
}

bool output_point::operator==(const point& other) const
{
    return point::operator==(other);
}

bool output_point::operator!=(const point& other) const
{
    return point::operator!=(other);
}

bool output_point::operator==(const output_point& other) const
{
    return point::operator==(other);
}

bool output_point::operator!=(const output_point& other) const
{
    return !(*this == other);
}

// Deserialization.
//-----------------------------------------------------------------------------

output_point output_point::factory(const data_chunk& data)
{
    output_point instance;
    instance.from_data(data);
    return instance;
}

output_point output_point::factory(std::istream& stream)
{
    output_point instance;
    instance.from_data(stream);
    return instance;
}

output_point output_point::factory(reader& source)
{
    output_point instance;
    instance.from_data(source);
    return instance;
}

} // namespace chain
} // namespace libbitcoin
