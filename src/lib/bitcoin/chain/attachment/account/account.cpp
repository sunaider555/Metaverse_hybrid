/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#include <metaverse/bitcoin/chain/attachment/account/account.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

#ifdef MVS_DEBUG
#include <json/minijson_writer.hpp>
#endif

#include <metaverse/bitcoin/math/crypto.hpp>
#include <metaverse/bitcoin.hpp>
using namespace libbitcoin::wallet;

namespace libbitcoin {
namespace chain {

account_script::account_script()
{
}

void account_script::set_description(const std::string& description)
{
    description_ = description;
}

void account_script::set_address(const std::string& address)
{
    address_ = address;
}

void account_script::set_script(const data_chunk& script)
{
    script_ = script;
}

bool account_script::from_data(reader& source)
{
    description_ = source.read_string();
    address_ = source.read_string();
    const auto size = source.read_variable_uint_little_endian();
    script_ = source.read_data(size);
    return !address_.empty();
}

void account_script::to_data(writer& sink) const
{
    sink.write_string(description_);
    sink.write_string(address_);
    sink.write_variable_uint_little_endian(script_.size());
    sink.write_data(script_);
}

bool account_script::operator==(const account_script& other) const
{
    //return (description_ == other.description_) && (address_ == other.address_) && (script_ == other.script_);
    return address_ == other.address_;
}

uint64_t account_script::serialized_size() const
{
    return variable_string_size(description_) + variable_string_size(address_) + variable_data_chunk_size(script_);
}

account_multisig::account_multisig()
    : hd_index_(0), m_(0), n_(0)
{
}

account_multisig::account_multisig(
    uint32_t hd_index, uint8_t m, uint8_t n,
    std::vector<std::string>&& cosigner_pubkeys, std::string& pubkey)
    : hd_index_(hd_index)
    , m_(m)
    , n_(n)
    , cosigner_pubkeys_(std::move(cosigner_pubkeys))
    , pubkey_(pubkey)
{
}

void account_multisig::set_hd_index(uint32_t hd_index) {
    hd_index_ = hd_index;
}

uint32_t account_multisig::get_hd_index() const {
    return hd_index_;
}

void account_multisig::set_index(uint32_t index) {
    index_ = index;
}

uint32_t account_multisig::get_index() const {
    return index_;
}

void account_multisig::set_m(uint8_t m) {
    m_ = m;
}

uint8_t account_multisig::get_m() const {
    return m_;
}

void account_multisig::set_n(uint8_t n) {
    n_ = n;
}

uint8_t account_multisig::get_n() const {
    return n_;
}

const std::vector<std::string>& account_multisig::get_cosigner_pubkeys() const {
    return cosigner_pubkeys_;
}

void account_multisig::set_cosigner_pubkeys(std::vector<std::string>&& cosigner_pubkeys) {
    cosigner_pubkeys_ = std::move(cosigner_pubkeys);
    std::sort(cosigner_pubkeys_.begin(), cosigner_pubkeys_.end());
}

std::string account_multisig::get_pub_key() const {
    return pubkey_;
}

void account_multisig::set_pub_key(std::string& pubkey) {
    pubkey_ = pubkey;
}

std::string account_multisig::get_description() const {
    return description_;
}

void account_multisig::set_description(std::string& description) {
    description_ = description;
}

std::string account_multisig::get_address() const {
    return address_;
}

void account_multisig::set_address(std::string& address) {
    address_ = address;
}

bool account_multisig::from_data(reader& source)
{
    hd_index_ = source.read_4_bytes_little_endian();
    index_ = source.read_4_bytes_little_endian();
    m_ = source.read_byte();
    n_ = source.read_byte();
    pubkey_ = source.read_string();
    // read consigner pubkeys
    uint8_t size = source.read_byte();
    while (size--)
        cosigner_pubkeys_.push_back(source.read_string());

    description_ = source.read_string();
    address_ = source.read_string();

    return true;
}

void account_multisig::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(hd_index_);
    sink.write_4_bytes_little_endian(index_);
    sink.write_byte(m_);
    sink.write_byte(n_);
    sink.write_string(pubkey_);
    sink.write_byte(cosigner_pubkeys_.size());

    for (auto& each : cosigner_pubkeys_) {
        sink.write_string(each);
    }

    //sink.write_string(std::string("02b66fcb1064d827094685264aaa90d0126861688932eafbd1d1a4ba149de3308b"));
    sink.write_string(description_);
    sink.write_string(address_);
}

uint64_t account_multisig::serialized_size() const
{
    uint64_t size = 4 + 4 + 1 + 1 + (pubkey_.size() + 9) + 1; // hd_index,index,m,n,pubkey,pubkey number

    for (auto& each : cosigner_pubkeys_) {
        size += (each.size() + 9);
    }
    size += (description_.size() + 9);
    size += (address_.size() + 9);
    return size;
}

bool account_multisig::operator==(const account_multisig& other) const
{
    if (hd_index_ != other.hd_index_
            || m_ != other.m_ || n_ != other.n_
            || pubkey_ != other.pubkey_) {
        return false;
    }

    auto& other_pubkeys = other.cosigner_pubkeys_;
    if (cosigner_pubkeys_.size() != other_pubkeys.size()) {
        return false;
    }

    for (const auto& pubkey : cosigner_pubkeys_) {
        auto iter = std::find(other_pubkeys.begin(), other_pubkeys.end(), pubkey);
        if (iter == other_pubkeys.end()) {
            return false;
        }
    }

    return true;
}

void account_multisig::reset()
{
    hd_index_ = 0;
    index_ = 0;
    m_ = 0;
    n_ = 0;
    pubkey_ = "";
    cosigner_pubkeys_.clear();
    description_ = "";
    address_ = "";
}

#ifdef MVS_DEBUG
std::string account_multisig::to_string()
{
    std::ostringstream ss;

    ss << "\t hd_index = " << hd_index_ << "\n"
       << "\t index = " << index_ << "\n"
       << "\t m = " << m_ << "\n"
       << "\t n = " << n_ << "\n"
       << "\t pubkey = " << pubkey_ << "\n"
       << "\t description = " << description_ << "\n";
    for (auto& each : cosigner_pubkeys_)
        ss << "\t cosigner-pubkey = " << each << std::endl;
    return ss.str();
}
#endif

std::string account_multisig::get_multisig_script() const
{
    if (m_ == 0 && n_ == 0) {
        // not initialized
        return "";
    }

    std::ostringstream ss;
    ss << std::to_string(m_);
    for (auto& each : cosigner_pubkeys_)
        ss << " [ " << each << " ] ";
    ss << std::to_string(n_) << " checkmultisig";
    return ss.str();
}

account::account()
{
    reset();
}

account::account(
    const std::string& name, const std::string& mnemonic, const hash_digest& passwd,
    uint32_t hd_index, uint8_t priority, uint8_t status, uint8_t type)
    : name(name), mnemonic(mnemonic), passwd(passwd)
    , hd_index(hd_index), priority(priority), status(status), type(type)
{
}


bool account::is_valid() const
{
    return true;
}

void account::reset()
{
    this->name = "";
    this->mnemonic = "";
    //this->passwd = "";
    this->hd_index = 0;
    this->priority = account_priority::common_user; // 0 -- admin user  1 -- common user
    this->type = account_type::common;
    this->status = account_status::normal;
}

bool account::from_data_t(reader& source)
{
    reset();
    name = source.read_string();
    //mnemonic = source.read_string();

    // read encrypted mnemonic
    auto size = source.read_variable_uint_little_endian();
    data_chunk string_bytes = source.read_data(size);
    std::string result(string_bytes.begin(), string_bytes.end());
    mnemonic = result;

    passwd = source.read_hash();
    hd_index = source.read_4_bytes_little_endian();
    priority = source.read_byte();
    //status = source.read_2_bytes_little_endian();
    type = source.read_byte();
    status = source.read_byte();
    if (is_multisignature(type)) {
        //multisig.from_data(source);
        account_multisig multisig;
        uint32_t size = source.read_4_bytes_little_endian();
        while (size--) {
            multisig.reset();
            multisig.from_data(source);
            multisig_vec.push_back(multisig);
        }
    }
    if (is_script(type)) {
        uint32_t size = source.read_4_bytes_little_endian();
        while (size--) {
            account_script script;
            script.from_data(source);
            script_vec.push_back(script);
        }
    }
    return true;
}

void account::to_data_t(writer& sink) const
{
    sink.write_string(name);
    sink.write_string(mnemonic);
    sink.write_hash(passwd);
    sink.write_4_bytes_little_endian(hd_index);
    sink.write_byte(priority);
    //sink.write_2_bytes_little_endian(status);
    sink.write_byte(type);
    sink.write_byte(status);
    if (is_multisignature(type)) {
        //multisig.to_data(sink);
        sink.write_4_bytes_little_endian(multisig_vec.size());
        if (multisig_vec.size()) {
            for (auto& each : multisig_vec) {
                each.to_data(sink);
            }
        }
    }
    if (is_script(type)) {
        sink.write_4_bytes_little_endian(script_vec.size());
        for (auto& each : script_vec) {
            each.to_data(sink);
        }
    }
}

uint64_t account::serialized_size() const
{
    uint64_t size = name.size() + mnemonic.size() + passwd.size() + 4 + 1 + 2 + 2 * 9; // 2 string len
    if (is_multisignature(type)) {
        //size += multisig.serialized_size();
        size += 4; // vector size
        for (auto& each : multisig_vec)
            size += each.serialized_size();
    }
    if (is_script(type)) {
        size += 4; // vector size
        for (auto& each : script_vec)
            size += each.serialized_size();
    }
    return size;
}

account::operator bool() const
{
    return (name.empty() || mnemonic.empty());
}

bool account::operator==(const account& other) const
{
    return ((name == other.get_name())
            && (passwd == other.get_passwd())
            && (mnemonic == other.get_mnemonic()));
}


#ifdef MVS_DEBUG
std::string account::to_string()
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
       << "\t mnemonic = " << mnemonic << "\n"
       << "\t password = " << passwd.data() << "\n"
       << "\t hd_index = " << hd_index << "\n"
       << "\t priority = " << priority << "\n"
       << "\t type = " << type << "\n"
       << "\t status = " << status << "\n";
    if (is_multisignature(type)) {
        for (auto& each : multisig_vec)
            ss << "\t\t" << each.to_string();
    }
    return ss.str();
}
#endif

const std::string& account::get_name() const
{
    return name;
}

void account::set_name(const std::string& name)
{
    this->name = name;
}

const std::string& account::get_mnemonic() const
{
    return mnemonic; // for account == operator
}

const std::string& account::get_mnemonic(std::string& passphrase, std::string& decry_output) const
{
    decrypt_string(mnemonic, passphrase, decry_output);
    return decry_output;
}

void account::set_mnemonic(const std::string& mnemonic, std::string& passphrase)
{
    if (!mnemonic.size())
        throw std::logic_error{"mnemonic size is 0"};
    if (!passphrase.size())
        throw std::logic_error{"invalid password!"};
    std::string encry_output("");

    encrypt_string(mnemonic, passphrase, encry_output);
    this->mnemonic = encry_output;
}

void account::set_mnemonic(const std::string& mnemonic)
{
    this->mnemonic = mnemonic;
}

const hash_digest& account::get_passwd() const
{
    return passwd;
}

uint32_t account::get_hd_index() const
{
    return hd_index;
}

void account::set_hd_index(uint32_t hd_index)
{
    this->hd_index = hd_index;
}

uint8_t account::get_type() const
{
    return type;
}

void account::set_type(uint8_t type)
{
    this->type |= type;
}

uint8_t account::get_status() const
{
    return status;
}

void account::set_status(uint8_t status)
{
    this->status = status;
}

uint8_t account::get_priority() const
{
    return priority;
}

void account::set_priority(uint8_t priority)
{
    this->priority = priority;
}

const account_multisig::list& account::get_multisig_vec() const
{
    return multisig_vec;
}

void account::set_multisig_vec(account_multisig::list&& multisig_vec)
{
    this->multisig_vec = std::move(multisig_vec);
}

bool account::is_multisig_exist(const account_multisig& multisig)
{
    auto iter = std::find(multisig_vec.begin(), multisig_vec.end(), multisig);
    return iter != multisig_vec.end();
}

void account::set_multisig(const account_multisig& multisig)
{
    if (!is_multisig_exist(multisig)) {
        multisig_vec.push_back(multisig);
    }
}

void account::remove_multisig(const account_multisig& multisig)
{
    for (auto it = multisig_vec.begin(); it != multisig_vec.end();) {
        if (*it == multisig) {
            it = multisig_vec.erase(it);
            break;
        }
        else {
            ++it;
        }
    }
}

std::shared_ptr<account_multisig::list> account::get_multisig(const std::string& addr)
{
    auto acc_vec = std::make_shared<account_multisig::list>();
    for (auto& each : multisig_vec) {
        if (addr == each.get_address()) {
            acc_vec->push_back(each);
        }
    }
    return acc_vec;
}

void account::modify_multisig(const account_multisig& multisig)
{
    for (auto& each : multisig_vec) {
        if (each == multisig) {
            each = multisig;
            break;
        }
    }
}

const account_script::list& account::get_script_vec() const
{
    return script_vec;
}
void account::set_script_vec(account_script::list&& script)
{
    script_vec = std::move(script);
}
bool account::is_script_exist(const account_script& script)
{
    const auto iter = std::find(script_vec.begin(), script_vec.end(), script);
    return iter != script_vec.end();
}
void account::set_script(const account_script& script)
{
    if (!is_script_exist(script)) {
        script_vec.push_back(script);
    }
}
void account::modify_script(const account_script& script)
{
    for (auto& each : script_vec) {
        if (each == script) {
            each = script;
            break;
        }
    }
}
void account::remove_script(const account_script& script)
{
    for (auto it = script_vec.begin(); it != script_vec.end(); ++it) {
        if (*it == script) {
            it = script_vec.erase(it);
            break;
        }
    }
}
std::shared_ptr<account_script::list> account::get_script(const std::string& addr)
{
    auto acc_vec = std::make_shared<account_script::list>();
    for (auto& each : script_vec) {
        if (addr == each.get_address()) {
            acc_vec->push_back(each);
        }
    }
    return acc_vec;
}

} // namspace chain
} // namspace libbitcoin
