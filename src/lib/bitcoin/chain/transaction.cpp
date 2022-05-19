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
#include <metaverse/bitcoin/chain/transaction.hpp>

#include <numeric>
#include <sstream>
#include <utility>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/math/limits.hpp>
#include <metaverse/bitcoin/chain/input.hpp>
#include <metaverse/bitcoin/chain/output.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/consensus/witness.hpp>

namespace libbitcoin {
namespace chain {

// default constructors

transaction::transaction()
  : version(0), locktime(0), hash_(nullptr)
{
}

transaction::transaction(const transaction& other)
  : transaction(other.version, other.locktime, other.inputs, other.outputs)
{
}

transaction::transaction(uint32_t version, uint32_t locktime,
    const input::list& inputs, const output::list& outputs)
  : version(version),
    locktime(locktime),
    inputs(inputs),
    outputs(outputs),
    hash_(nullptr)
{
}

transaction::transaction(transaction&& other)
  : transaction(other.version, other.locktime,
        std::forward<input::list>(other.inputs),
        std::forward<output::list>(other.outputs))
{
}

transaction::transaction(uint32_t version, uint32_t locktime,
    input::list&& inputs, output::list&& outputs)
  : version(version),
    locktime(locktime),
    inputs(std::forward<input::list>(inputs)),
    outputs(std::forward<output::list>(outputs)),
    hash_(nullptr)
{
}

transaction& transaction::operator=(transaction&& other)
{
    version = other.version;
    locktime = other.locktime;
    inputs = std::move(other.inputs);
    outputs = std::move(other.outputs);
    return *this;
}

// TODO: eliminate blockchain transaction copies and then delete this.
transaction& transaction::operator=(const transaction& other)
{
    version = other.version;
    locktime = other.locktime;
    inputs = other.inputs;
    outputs = other.outputs;
    return *this;
}

bool transaction::is_valid() const
{
    return (version != 0) || (locktime != 0) || !inputs.empty() ||
        !outputs.empty();
}

void transaction::reset()
{
    version = 0;
    locktime = 0;
    inputs.clear();
    inputs.shrink_to_fit();
    outputs.clear();
    outputs.shrink_to_fit();

    mutex_.lock();
    hash_.reset();
    mutex_.unlock();
}

bool transaction::from_data_t(reader& source)
{
    reset();
    version = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
    {
        const auto tx_in_count = source.read_variable_uint_little_endian();
        result = source;

        if (result)
        {
            inputs.resize(tx_in_count);

            for (auto& input: inputs)
            {
                result = input.from_data(source);

                if (!result)
                    break;
            }
        }
    }

    if (result)
    {
        const auto tx_out_count = source.read_variable_uint_little_endian();
        result = source;

        if (result)
        {
            outputs.resize(tx_out_count);

            for (auto& output: outputs)
            {
                result = output.from_data(source);

                if (!result)
                    break;
            }
        }
    }

    if (result)
    {
        locktime = source.read_4_bytes_little_endian();
        result = source;
    }

    if (!result)
        reset();

    return result;
}

void transaction::to_data_t(writer& sink) const
{
    sink.write_4_bytes_little_endian(version);
    sink.write_variable_uint_little_endian(inputs.size());

    for (const auto& input: inputs)
        input.to_data(sink);

    sink.write_variable_uint_little_endian(outputs.size());

    for (const auto& output: outputs)
        output.to_data(sink);

    sink.write_4_bytes_little_endian(locktime);
}

uint64_t transaction::serialized_size() const
{
    uint64_t tx_size = 8;
    tx_size += variable_uint_size(inputs.size());
    for (const auto& input: inputs)
        tx_size += input.serialized_size();

    tx_size += variable_uint_size(outputs.size());
    for (const auto& output: outputs)
        tx_size += output.serialized_size();

    return tx_size;
}

std::string transaction::to_string(uint32_t flags) const
{
    flags = chain::get_script_context();

    std::ostringstream value;
    value << "Transaction:\n"
        << "\tversion = " << version << "\n"
        << "\tlocktime = " << locktime << "\n"
        << "Inputs:\n";

    for (const auto input: inputs)
        value << input.to_string(flags);

    value << "Outputs:\n";
    for (const auto output: outputs)
        value << output.to_string(flags);

    value << "\n";
    return value.str();
}

hash_digest transaction::hash() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (!hash_)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        hash_.reset(new hash_digest(bitcoin_hash(to_data())));
        mutex_.unlock_and_lock_upgrade();
        //---------------------------------------------------------------------
    }

    hash_digest hash = *hash_;
    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return hash;
}

hash_digest transaction::hash(uint32_t sighash_type) const
{
    auto serialized = to_data();
    extend_data(serialized, to_little_endian(sighash_type));
    return bitcoin_hash(serialized);
}

bool transaction::is_coinbase() const
{
    return (inputs.size() == 1) && inputs[0].previous_output.is_null();
}

bool transaction::is_pos_genesis_tx(bool is_testnet) const
{
    if (!is_coinbase() || outputs.size() != (1 + witness_cert_count)) {
        return false;
    }

    // check etp reward
    const auto & out = outputs[0];
    if (!out.is_etp() || out.value != pos_genesis_reward) {
        return false;
    }

    chain::script script;
    std::string foundation_address = get_foundation_address(is_testnet);
    wallet::payment_address pay_address(foundation_address);
    script.operations = chain::operation::to_pay_key_hash_pattern(pay_address.hash());
    const auto expected = script.to_data(false);

    const auto actual = out.script.to_data(false);
    if (!std::equal(expected.begin(), expected.end(), actual.begin())) {
        return false;
    }

    // check witness cert
    for (uint32_t i = 0; i < witness_cert_count; ++i) {
        const auto& out = outputs[i + 1];
        if (!out.is_asset_cert_autoissue()) {
            return false;
        }

        const auto cert = out.get_asset_cert();
        if (cert.get_address() != foundation_address || !cert.is_primary_witness()) {
            return false;
        }

        const auto actual = out.script.to_data(false);
        if (!std::equal(expected.begin(), expected.end(), actual.begin())) {
            return false;
        }
    }

    return true;
}

bool transaction::is_coinstake() const
{
    return (inputs.size() > 0)
        && (!inputs[0].previous_output.is_null())
        && (outputs.size() >= 2)
        && (outputs[0].is_null()) //the coin stake transaction is marked with the first output empty
        && (inputs[0].get_script_address() == outputs[1].get_script_address());
}

bool transaction::all_inputs_final() const
{
    const auto finalized = [](const input& input)
    {
        return input.is_final();
    };

    return std::all_of(inputs.begin(), inputs.end(), finalized);
}

bool transaction::is_final(uint64_t block_height, uint32_t block_time) const
{
    const auto max_locktime = [=]()
    {
        return locktime < locktime_threshold ?
            safe_unsigned<uint32_t>(block_height) : block_time;
    };

    return locktime == 0 || locktime < max_locktime() || all_inputs_final();
}

bool transaction::is_locktime_conflict() const
{
    auto locktime_set = locktime != 0;

    if (locktime_set)
        for (const auto& input: inputs)
            if (input.sequence < max_input_sequence)
                return false;

    return locktime_set;
}

uint64_t transaction::total_output_value() const
{
    const auto value = [](uint64_t total, const output& output)
    {
        return total + output.value;
    };

    return std::accumulate(outputs.begin(), outputs.end(), uint64_t(0), value);
}

uint64_t transaction::total_output_transfer_amount() const
{
    const auto value = [](uint64_t total, const output& output)
    {
        // asset issue and asset transfer can not co-exist in one transaction outputs.
        // asset secondary issue is from air, so not add its amount to pass amount check.
        if (output.is_asset_secondaryissue()) {
            return total;
        }
        return total + output.get_asset_amount();
    };
    return std::accumulate(outputs.begin(), outputs.end(), uint64_t(0), value);
}

uint64_t transaction::legacy_sigops_count(bool accurate) const
{
    uint64_t total_sigs = 0;
    for (const auto& input : inputs)
    {
        const auto& operations = input.script.operations;
        total_sigs += operation::count_script_sigops(operations, accurate);
    }

    for (const auto& output : outputs)
    {
        const auto& operations = output.script.operations;
        total_sigs += operation::count_script_sigops(operations, accurate);
    }

    return total_sigs;
}

uint64_t transaction::legacy_sigops_count(const transaction::list& txs, bool accurate)
{
    uint64_t total_sigs = 0;
    for (const auto& tx : txs) {
        total_sigs += tx.legacy_sigops_count(accurate);
    }
    return total_sigs;
}

bool transaction::has_asset_issue() const
{
    for (auto& elem: outputs) {
        if(elem.is_asset_issue())
            return true;
    }
    return false;
}

bool transaction::has_asset_secondary_issue() const
{
    for (auto& elem: outputs) {
        if(elem.is_asset_secondaryissue())
            return true;
    }
    return false;
}


bool transaction::has_asset_transfer() const
{
    for (auto& elem: outputs) {
        if(elem.is_asset_transfer()
            && elem.get_asset_amount()) // block #810376 has 0 asset transfer without input
            return true;
    }
    return false;
}

bool transaction::has_asset_cert() const
{
    for (auto& elem: outputs) {
        if(elem.is_asset_cert())
            return true;
    }
    return false;
}

bool transaction::has_asset_mit_transfer() const
{
    for (auto& elem: outputs) {
        if(elem.is_asset_mit_transfer())
            return true;
    }
    return false;
}

bool transaction::has_did_register() const
{
    for (auto& elem: outputs) {
        if(elem.is_did_register())
            return true;
    }
    return false;
}

bool transaction::has_did_transfer() const
{
    for (auto& elem: outputs) {
        if(elem.is_did_transfer())
            return true;
    }
    return false;
}


} // namspace chain
} // namspace libbitcoin
