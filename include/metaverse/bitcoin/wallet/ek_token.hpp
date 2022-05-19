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
#ifndef MVS_WALLET_EK_TOKEN_HPP
#define MVS_WALLET_EK_TOKEN_HPP

#include <iostream>
#include <string>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/wallet/encrypted_keys.hpp>

namespace libbitcoin {
namespace wallet {

/**
 * Serialization helper to convert between base58 string and bip38 token.
 */
class BC_API ek_token
{
public:
    /// Constructors.
    ek_token();
    ek_token(const std::string& encoded);
    ek_token(const encrypted_token& key);
    ek_token(const ek_token& other);

    /// Operators.
    bool operator<(const ek_token& other) const;
    bool operator==(const ek_token& other) const;
    bool operator!=(const ek_token& other) const;
    ek_token& operator=(const ek_token& other);
    friend std::istream& operator>>(std::istream& in, ek_token& to);
    friend std::ostream& operator<<(std::ostream& out, const ek_token& of);

    /// Cast operators.
    operator const bool() const;
    operator const encrypted_token&() const;

    /// Serializer.
    std::string encoded() const;

    /// Accessors.
    const encrypted_token& token() const;

private:
    /// Factories.
    static ek_token from_string(const std::string& encoded);

    /// Members.
    /// These should be const, apart from the need to implement assignment.
    bool valid_;
    encrypted_token token_;
};

} // namespace wallet
} // namespace libbitcoin

#endif
