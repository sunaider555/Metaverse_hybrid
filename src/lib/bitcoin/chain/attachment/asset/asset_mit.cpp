/**
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
#include <metaverse/bitcoin/chain/attachment/asset/asset_mit.hpp>
#include <sstream>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/mgbubble/utility/Compare.hpp>

namespace libbitcoin {
namespace chain {

// use 1~127 to represent normal mit status type
// add plus 128 to them to make their status type in tracing state.
// status >128 means no content should be store
constexpr uint8_t MIT_STATUS_MASK = 0x7f;
constexpr uint8_t MIT_STATUS_SHORT_OFFSET = 0x80;

asset_mit::asset_mit()
{
    reset();
}

asset_mit::asset_mit(const std::string& symbol,
                     const std::string& address, const std::string& content)
    : symbol_(symbol)
    , address_(address)
    , content_(content)
    , status_(MIT_STATUS_NONE)
{
}

void asset_mit::reset()
{
    symbol_ = "";
    address_ = "";
    content_ = "";
    status_ = MIT_STATUS_NONE;
}

bool asset_mit::is_valid() const
{
    return !(symbol_.empty()
             || address_.empty()
             || is_invalid_status()
             || (calc_size() > get_max_serialized_size()));
}

bool asset_mit::operator< (const asset_mit& other) const
{
    return symbol_.compare(other.symbol_) < 0;
}

bool asset_mit::from_data_t(reader& source)
{
    reset();

    status_ = source.read_byte();
    symbol_ = source.read_string();
    address_ = source.read_string();
    if (is_register_status()) {
        content_ = source.read_string();
    }

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk asset_mit::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);
    // store status with offset, specify to store no content.
    sink.write_byte(get_status() + MIT_STATUS_SHORT_OFFSET);
    sink.write_string(symbol_);
    sink.write_string(address_);
    ostream.flush();
    return data;
}


void asset_mit::to_data_t(writer& sink) const
{
    sink.write_byte(status_);
    sink.write_string(symbol_);
    sink.write_string(address_);
    if (is_register_status()) {
        sink.write_string(content_);
    }
}

uint64_t asset_mit::get_max_serialized_size() const
{
    return is_register_status() ? ASSET_MIT_FIX_SIZE : ASSET_MIT_TRANSFER_FIX_SIZE;
}

uint64_t asset_mit::calc_size() const
{
    uint64_t len = (symbol_.size() + 1) + (address_.size() + 1) + ASSET_MIT_STATUS_FIX_SIZE;
    if (is_register_status()) {
        len += variable_string_size(content_);
    }
    return len;
}

uint64_t asset_mit::serialized_size() const
{
    return std::min(calc_size(), get_max_serialized_size());
}

std::string asset_mit::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << get_status_name() << "\n";
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t address = " << address_ << "\n";
    if (is_register_status()) {
        ss << "\t content = " << content_ << "\n";
    }
    return ss.str();
}

const std::string& asset_mit::get_symbol() const
{
    return symbol_;
}

void asset_mit::set_symbol(const std::string& symbol)
{
    symbol_ = limit_size_string(symbol, ASSET_MIT_SYMBOL_FIX_SIZE);
}

const std::string& asset_mit::get_address() const
{
    return address_;
}

void asset_mit::set_address(const std::string& address)
{
    address_ = limit_size_string(address, ASSET_MIT_ADDRESS_FIX_SIZE);
}

const std::string& asset_mit::get_content() const
{
    return content_;
}

void asset_mit::set_content(const std::string& content)
{
    content_ = limit_size_string(content, ASSET_MIT_CONTENT_FIX_SIZE);
}

uint8_t asset_mit::get_status() const
{
    return status_ & MIT_STATUS_MASK;
}

void asset_mit::set_status(uint8_t status)
{
    status_ = status & MIT_STATUS_MASK;
}

std::string asset_mit::status_to_string(uint8_t status)
{
    if (status == MIT_STATUS_REGISTER) {
        return "registered";
    }
    else if (status == MIT_STATUS_TRANSFER) {
        return "transfered";
    }
    else {
        return "none";
    }
}

std::string asset_mit::get_status_name() const
{
    return status_to_string(get_status());
}

bool asset_mit::is_register_status() const
{
    return status_ == MIT_STATUS_REGISTER;
}

bool asset_mit::is_transfer_status() const
{
    return status_ == MIT_STATUS_TRANSFER;
}

bool asset_mit::is_invalid_status() const
{
    return status_ <= MIT_STATUS_NONE || status_ >= MIT_STATUS_MAX;
}

///////////////////////////////////////////////////
///////////// asset_mit_info //////////////////////
///////////////////////////////////////////////////
void asset_mit_info::reset()
{
    output_height = 0;
    timestamp = 0;
    to_did = "";
    mit.reset();
}

bool asset_mit_info::operator< (const asset_mit_info& other) const
{
    return mit < other.mit;
}

uint64_t asset_mit_info::serialized_size() const
{
    // output_height; timestamp; to_did; mit;
    return 4 + 4 + (to_did.size() +1) + mit.serialized_size();
}

asset_mit_info asset_mit_info::factory_from_data(reader& source)
{
    asset_mit_info instance;
    instance.reset();

    instance.output_height = source.read_4_bytes_little_endian();
    instance.timestamp = source.read_4_bytes_little_endian();
    instance.to_did = source.read_string();
    instance.mit = asset_mit::factory_from_data(source);

    auto result = static_cast<bool>(source);
    if (!result) {
        instance.reset();
    }

    return instance;
}

data_chunk asset_mit_info::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_did);
    sink.write_data(mit.to_data());

    ostream.flush();
    return data;
}

data_chunk asset_mit_info::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_did);
    sink.write_data(mit.to_short_data());

    ostream.flush();
    return data;
}

} // namspace chain
} // namspace libbitcoin
