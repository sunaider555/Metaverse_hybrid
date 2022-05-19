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
#include <metaverse/bitcoin/chain/output.hpp>
#include <cctype>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/bitcoin/wallet/payment_address.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>

namespace libbitcoin {
namespace chain {


output::output()
{
    reset();
}

output::output(output&& other)
: output(other.value, std::move(other.script), std::move(other.attach_data))
{
}
output::output(const output& other)
:output(other.value, other.script, other.attach_data)
{
}

output::output(uint64_t&& value, chain::script&& script, attachment&& attach_data)
: value(std::move(value)), script(std::move(script))
,attach_data(std::move(attach_data))
{
}

output::output(const uint64_t& value, const chain::script& script, const attachment& attach_data)
: value(value), script(script),attach_data(attach_data)
{
}

output& output::operator=(output&& other)
{
    value = std::move(other.value);
    script = std::move(other.script);
    attach_data = std::move(other.attach_data);
    return *this;
}

output& output::operator=(const output& other)
{
    value = other.value;
    script = other.script;
    attach_data = other.attach_data;
    return *this;
}

bool output::is_valid() const
{
    return (value != 0) || script.is_valid()
        || attach_data.is_valid(); // added for asset issue/transfer
}

bool output::is_null() const
{
    return !is_valid();
}

std::string output::get_script_address() const
{
    auto payment_address = wallet::payment_address::extract(script);
    return payment_address.encoded();
}

code output::check_attachment_address(bc::blockchain::block_chain_impl& chain) const
{
    bool is_asset = false;
    bool is_did = false;
    std::string attachment_address;
    if (is_asset_issue() || is_asset_secondaryissue() || is_asset_mit()) {
        attachment_address = get_asset_address();
        is_asset = true;
    } else if (is_asset_cert()) {
        attachment_address = get_asset_cert_address();
        is_asset = true;
    } else if (is_did_register() || is_did_transfer()) {
        attachment_address = get_did_address();
        is_did = true;
    }
    if (is_asset || is_did) {
        auto script_address = get_script_address();
        if (attachment_address != script_address) {
            log::debug("output::check_attachment_address")
                << (is_asset ? "asset" : "did")
                << " attachment address " << attachment_address
                << " is not equal to script address " << script_address;
            if (is_asset) {
                return error::asset_address_not_match;
            }
            if (is_did) {
                return error::did_address_not_match;
            }
        }
    }
    return error::success;
}

void output::reset()
{
    value = 0;
    script.reset();
    attach_data.reset(); // added for asset issue/transfer
}

bool output::from_data_t(reader& source)
{
    reset();

    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
        result = script.from_data(source, true,
            script::parse_mode::raw_data_fallback);

    /* begin added for asset issue/transfer */
    if (result)
        result = attach_data.from_data(source);
    /* end added for asset issue/transfer */

    if (!result)
        reset();

    return result;
}

void output::to_data_t(writer& sink) const
{
    sink.write_8_bytes_little_endian(value);
    script.to_data(sink, true);
    /* begin added for asset issue/transfer */
    attach_data.to_data(sink);
    /* end added for asset issue/transfer */
}

uint64_t output::serialized_size() const
{
    return 8 + script.serialized_size(true)
        + attach_data.serialized_size(); // added for asset issue/transfer
}

std::string output::to_string(uint32_t flags) const
{
    flags = chain::get_script_context();

    std::ostringstream ss;

    ss << "\tvalue = " << value << "\n"
        << "\t" << script.to_string(flags) << "\n"
        << "\t" << attach_data.to_string() << "\n"; // added for asset issue/transfer

    return ss.str();
}

uint64_t output::get_asset_amount() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_maximum_supply();
        }
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            auto trans_info = boost::get<asset_transfer>(asset_info.get_data());
            return trans_info.get_quantity();
        }
    }
    return 0;
}

bool output::is_asset_transfer() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        return (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_did_transfer() const
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        return (did_info.get_status() == DID_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_asset_issue() const
{
    if(attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if(asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return !detail_info.is_asset_secondaryissue();
        }
    }
    return false;
}

bool output::is_asset_secondaryissue() const
{
    if(attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if(asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.is_asset_secondaryissue();
        }
    }
    return false;
}

bool output::is_asset_mit() const
{
    return (attach_data.get_type() == ASSET_MIT_TYPE);
}

std::string output::get_asset_mit_symbol() const
{
    if (is_asset_mit()) {
        auto mit_info = boost::get<asset_mit>(attach_data.get_attach());
        return mit_info.get_symbol();
    }
    return std::string("");
}

bool output::is_asset_mit_register() const
{
    if (is_asset_mit()) {
        auto asset_info = boost::get<asset_mit>(attach_data.get_attach());
        if (asset_info.is_register_status()) {
            return true;
        }
    }
    return false;
}

bool output::is_asset_mit_transfer() const
{
    if (is_asset_mit()) {
        auto asset_info = boost::get<asset_mit>(attach_data.get_attach());
        if (asset_info.is_transfer_status()) {
            return true;
        }
    }
    return false;
}

bool output::is_asset_cert() const
{
    return (attach_data.get_type() == ASSET_CERT_TYPE);
}

bool output::is_asset_cert_autoissue() const
{
    if (attach_data.get_type() == ASSET_CERT_TYPE) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        if (cert_info.get_status() == ASSET_CERT_AUTOISSUE_TYPE) {
            return true;
        }
    }
    return false;
}

bool output::is_asset_cert_issue() const
{
    if (attach_data.get_type() == ASSET_CERT_TYPE) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        if (cert_info.get_status() == ASSET_CERT_ISSUE_TYPE) {
            return true;
        }
    }
    return false;
}

bool output::is_asset_cert_transfer() const
{
    if (attach_data.get_type() == ASSET_CERT_TYPE) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        if (cert_info.get_status() == ASSET_CERT_TRANSFER_TYPE) {
            return true;
        }
    }
    return false;
}

bool output::is_asset() const
{
    return (attach_data.get_type() == ASSET_TYPE);
}

bool output::is_did() const
{
    return (attach_data.get_type() == DID_TYPE);
}

bool output::is_etp() const
{
    return (attach_data.get_type() == ETP_TYPE);
}

bool output::is_etp_award() const
{
    return (attach_data.get_type() == ETP_AWARD_TYPE);
}

bool output::is_message() const
{
    return (attach_data.get_type() == MESSAGE_TYPE);
}

std::string output::get_asset_symbol() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (is_asset()) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_symbol();
        }
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            auto trans_info = boost::get<asset_transfer>(asset_info.get_data());
            return trans_info.get_symbol();
        }
    }
    else if (is_asset_mit()) {
        auto asset_info = boost::get<asset_mit>(attach_data.get_attach());
        return asset_info.get_symbol();
    }
    else if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_asset_issuer() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (is_asset()) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_issuer();
        }
    }
    else if (is_asset_mit()) {
        BITCOIN_ASSERT(false);
    }
    return std::string("");
}

std::string output::get_asset_address() const // for validate_transaction.cpp to verify asset address
{
    if (is_asset()) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_address();
        }
    }
    else if (is_asset_mit()) {
        auto asset_info = boost::get<asset_mit>(attach_data.get_attach());
        return asset_info.get_address();
    }
    return std::string("");
}

asset_mit output::get_asset_mit() const
{
    if (is_asset_mit()) {
        return boost::get<asset_mit>(attach_data.get_attach());
    }
    log::error("output::get_asset_mit") << "Asset type is not an mit.";
    return asset_mit();
}

asset_cert output::get_asset_cert() const
{
    if (is_asset_cert()) {
        return boost::get<asset_cert>(attach_data.get_attach());
    }
    log::error("output::get_asset_cert") << "Asset type is not an asset_cert.";
    return asset_cert();
}

std::string output::get_asset_cert_symbol() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_asset_cert_owner() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_owner();
    }
    return std::string("");
}

std::string output::get_asset_cert_address() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_address();
    }

    return std::string("");
}

asset_cert_type output::get_asset_cert_type() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_type();
    }
    return asset_cert_ns::none;
}

bool output::is_did_register() const
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        return (did_info.get_status() ==  DID_DETAIL_TYPE);
    }
    return false;
}

std::string output::get_did_symbol() const // for validate_transaction.cpp to calculate did transfer amount
{
    if (attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        auto detail_info = boost::get<did_detail>(did_info.get_data());
        return detail_info.get_symbol();

    }
    return std::string("");
}

std::string output::get_did_address() const // for validate_transaction.cpp to calculate did transfer amount
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        auto detail_info = boost::get<did_detail>(did_info.get_data());
        return detail_info.get_address();

    }
    return std::string("");
}

did output::get_did() const
{
    if(attach_data.get_type() == DID_TYPE) {
        return boost::get<did>(attach_data.get_attach());
    }
    return did();
}

asset_transfer output::get_asset_transfer() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            return boost::get<asset_transfer>(asset_info.get_data());
        }
    }
    log::error("output::get_asset_transfer") << "Asset type is not asset_transfer_TYPE.";
    return asset_transfer();
}

asset_detail output::get_asset_detail() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            return boost::get<asset_detail>(asset_info.get_data());
        }
    }
    log::error("output::get_asset_detail") << "Asset type is not ASSET_DETAIL_TYPE.";
    return asset_detail();
}

const data_chunk& output::get_attenuation_model_param() const
{
    BITCOIN_ASSERT(operation::is_pay_key_hash_with_attenuation_model_pattern(script.operations));
    return operation::get_model_param_from_pay_key_hash_with_attenuation_model(script.operations);
}

uint32_t output::get_lock_sequence(uint32_t default_value) const
{
    if (!operation::is_pay_key_hash_with_sequence_lock_pattern(script.operations)) {
        return default_value;
    }
    return operation::get_lock_sequence_from_pay_key_hash_with_sequence_lock(script.operations);
}

uint32_t output::get_lock_heights_sequence(uint32_t default_value) const
{
    if (!operation::is_pay_key_hash_with_sequence_lock_pattern(script.operations)) {
        return default_value;
    }
    auto raw_value = operation::get_lock_sequence_from_pay_key_hash_with_sequence_lock(script.operations);
    return get_relative_locktime_locked_heights(raw_value);
}

uint32_t output::get_lock_seconds_sequence(uint32_t default_value) const
{
    if (!operation::is_pay_key_hash_with_sequence_lock_pattern(script.operations)) {
        return default_value;
    }
    auto raw_value = operation::get_lock_sequence_from_pay_key_hash_with_sequence_lock(script.operations);
    return get_relative_locktime_locked_seconds(raw_value);
}

} // namspace chain
} // namspace libbitcoin
