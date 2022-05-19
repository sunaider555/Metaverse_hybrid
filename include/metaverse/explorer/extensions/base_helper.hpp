/**
 * Copyright (c) 2016-2018 mvs developers
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
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
#pragma once

#include <metaverse/bitcoin.hpp>
#include <metaverse/bitcoin/chain/attachment/attachment.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account_address.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>

namespace libbitcoin {
namespace blockchain {
class block_chain_impl;
}
}

namespace libbitcoin {
namespace explorer {
namespace commands{

/// NOTICE: this type is not equal to attachment_type and business_kind
/// attachment_type : the collapsed type of tx output attachment, **recorded on blockchain**
/// business_kind   : the expanded type of attachment, mainly used for database/history query
/// for example :
/// attachment_type           |  business_kind
/// -------------------------------------------------------------------
/// attachment_etp           --> etp
/// attachment_etp_award     --> etp_award
/// attachment_asset         --> asset_issue | asset_transfer
/// attachment_message       --> message
/// attachment_did           --> did_register   |  did_transfer
/// attachment_asset_cert    --> asset_cert
/// attachment_asset_mit     --> asset_mit
/// -------------------------------------------------------------------
/// utxo_attach_type is only used in explorer module
/// utxo_attach_type will be used to generate attachment with attachment_type and content
/// for example :
/// utxo_attach_type::asset_issue    --> attachment_asset of asset_detail
///     auto asset_detail = asset(ASSET_DETAIL_TYPE, asset_detail);
///     attachment(ASSET_TYPE, ATTACH_INIT_VERSION, asset_detail);
/// utxo_attach_type::asset_transfer --> attachment_asset of asset_transfer
///     auto asset_transfer = asset(ASSET_TRANSFERABLE_TYPE, asset_transfer);
///     attachment(ASSET_TYPE, ATTACH_INIT_VERSION, asset_transfer);
/// NOTICE: createrawtx / createmultisigtx --type option is using these values.
/// DO NOT CHANGE EXIST ITEMS!!!
enum class utxo_attach_type : uint32_t
{
    etp = 0,
    deposit = 1,
    asset_issue = 2,
    asset_transfer = 3,
    asset_attenuation_transfer = 4,
    asset_locked_transfer = 5,
    message = 6,
    asset_cert = 7,
    asset_secondaryissue = 8,
    did_register = 9,
    did_transfer = 10,
    asset_cert_issue = 11,
    asset_cert_transfer = 12,
    asset_cert_autoissue = 13,
    asset_mit = 14,
    asset_mit_transfer = 15,
    invalid = 0xffffffff
};

extern utxo_attach_type get_utxo_attach_type(const chain::output&);

struct address_asset_record
{
    std::string prikey;
    std::string addr;
    uint64_t    amount{0}; // spendable etp amount
    std::string symbol;
    uint64_t    asset_amount{0}; // spendable asset amount
    chain::asset_cert_type asset_cert{chain::asset_cert_ns::none};
    utxo_attach_type type{utxo_attach_type::invalid};
    chain::output_point output;
    chain::script script;
    uint32_t hd_index{0}; // only used for multisig tx
    uint32_t sequence{max_input_sequence};
};

struct receiver_record
{
    typedef std::vector<receiver_record> list;

    std::string target;
    std::string symbol;
    uint64_t    amount{0}; // etp value
    uint64_t    asset_amount{0};
    chain::asset_cert_type asset_cert{chain::asset_cert_ns::none};

    utxo_attach_type type{utxo_attach_type::invalid};
    chain::attachment attach_elem;  // used for MESSAGE_TYPE, used for information transfer etc.
    chain::input_point input_point{null_hash, max_uint32};
    bool is_lock_seq_{false};

    receiver_record()
        : target()
        , symbol()
        , amount(0)
        , asset_amount(0)
        , asset_cert(chain::asset_cert_ns::none)
        , type(utxo_attach_type::invalid)
        , attach_elem()
        , input_point{null_hash, max_uint32}
        , is_lock_seq_(false)
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t asset_amount_,
        utxo_attach_type type_, const chain::attachment& attach_elem_, bool is_lock_seq)
        : receiver_record(target_, symbol_, amount_, asset_amount_,
            chain::asset_cert_ns::none, type_, attach_elem_,
            chain::input_point(null_hash, max_uint32), is_lock_seq)
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t asset_amount_,
        utxo_attach_type type_, const chain::attachment& attach_elem_ = chain::attachment(),
        const chain::input_point& input_point_ = {null_hash, max_uint32},
        bool is_lock_seq = false)
        : receiver_record(target_, symbol_, amount_, asset_amount_,
            chain::asset_cert_ns::none, type_, attach_elem_, input_point_, is_lock_seq)
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t asset_amount_, chain::asset_cert_type asset_cert_,
        utxo_attach_type type_, const chain::attachment& attach_elem_ = chain::attachment(),
        const chain::input_point& input_point_ = {null_hash, max_uint32},
        bool is_lock_seq = false)
        : target(target_)
        , symbol(symbol_)
        , amount(amount_)
        , asset_amount(asset_amount_)
        , asset_cert(asset_cert_)
        , type(type_)
        , attach_elem(attach_elem_)
        , input_point(input_point_)
        , is_lock_seq_(is_lock_seq)
    {}

    bool is_empty() const;
};

struct utxo_balance {
    typedef std::vector<utxo_balance> list;
    std::string output_hash;
    uint32_t output_index;
    uint64_t output_height;
    uint64_t unspent_balance;
    uint64_t frozen_balance;
};

struct balances {
    uint64_t total_received;
    uint64_t confirmed_balance;
    uint64_t unspent_balance;
    uint64_t frozen_balance;
};

struct locked_balance {
    typedef std::vector<locked_balance> list;
    std::string address;
    uint64_t locked_value;
    uint64_t locked_height;
    uint64_t expiration_height;
    uint64_t lock_at_height;
    bool is_time_locked;

    bool operator< (const locked_balance& other) const {
        return expiration_height < other.expiration_height;
    }
};

struct deposited_balance {
    deposited_balance(const std::string& address_, const std::string& tx_hash_,
        uint64_t deposited_, uint64_t expiration_)
        : address(address_)
        , tx_hash(tx_hash_)
        , balance(0)
        , bonus(0)
        , deposited_height(deposited_)
        , expiration_height(expiration_)
    {}

    std::string address;
    std::string tx_hash;
    std::string bonus_hash;
    uint64_t balance;
    uint64_t bonus;
    uint64_t deposited_height;
    uint64_t expiration_height;

    // for sort
    bool operator< (const deposited_balance& other) const {
        return expiration_height < other.expiration_height;
    }

    typedef std::vector<deposited_balance> list;
};

// helper function
void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance);

void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<utxo_balance::list> sh_vec);

void sync_fetch_deposited_balance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<deposited_balance::list> sh_vec);

void sync_fetch_asset_balance(const std::string& address, bool sum_all,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<chain::asset_balances::list> sh_asset_vec);

void sync_fetch_asset_deposited_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<chain::asset_deposited_balance::list> sh_asset_vec);

std::shared_ptr<chain::asset_balances::list> sync_fetch_asset_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain);

std::shared_ptr<chain::asset_deposited_balance::list> sync_fetch_asset_deposited_view(
    const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain);

void sync_fetch_locked_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<locked_balance::list> sp_vec,
    const std::string& asset_symbol,
    uint64_t expiration = 0);

void sync_fetch_asset_cert_balance(const std::string& address, const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<chain::asset_cert::list> sh_vec,
    chain::asset_cert_type cert_type=chain::asset_cert_ns::none);

std::string get_random_payment_address(std::shared_ptr<std::vector<chain::account_address>>,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address(const std::string& did_or_address,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address(const std::string& did_or_address,
    chain::attachment& attach, bool is_from,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address_from_did(const std::string& did,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_fee_dividend_address(bc::blockchain::block_chain_impl& blockchain);

bool is_ETH_Address(const std::string& address);
void check_asset_symbol(const std::string& symbol, bool check_sensitive=false);
void check_mit_symbol(const std::string& symbol, bool check_sensitive=false);
void check_did_symbol(const std::string& symbol, bool check_sensitive=false);
void check_message(const std::string& message, bool check_sensitive=false);
void check_mining_subsidy_param(const std::string& param);
chain::asset_cert_type check_issue_cert(bc::blockchain::block_chain_impl& blockchain,
    const std::string& account, const std::string& symbol, const std::string& cert_name);
chain::asset_cert_type check_cert_type_name(const std::string& cert_type_name, bool all=false);

class BCX_API base_transfer_common
{
public:
    using exclude_range_t = std::pair<uint64_t, uint64_t>;
    enum filter : uint8_t {
        FILTER_ETP = 1 << 0,
        FILTER_ASSET = 1 << 1,
        FILTER_ASSETCERT = 1 << 2,
        FILTER_DID = 1 << 3,
        FILTER_IDENTIFIABLE_ASSET = 1 << 4,
        FILTER_ALL = 0xff,
        // if specify 'from_' address,
        // then get these types' unspent only from 'from_' address
        FILTER_PAYFROM = FILTER_ETP | FILTER_ASSET,
        FILTER_ALL_BUT_PAYFROM = FILTER_ALL & ~FILTER_PAYFROM
    };

    base_transfer_common(
        bc::blockchain::block_chain_impl& blockchain,
        receiver_record::list&& receiver_list, uint64_t fee,
        std::string&& symbol, std::string&& from, std::string&& change,
        uint32_t locktime = 0, uint32_t sequence = bc::max_input_sequence,
        exclude_range_t exclude_etp_range = {0, 0})
        : blockchain_{blockchain}
        , symbol_{std::move(symbol)}
        , from_{std::move(from)}
        , mychange_{std::move(change)}
        , payment_etp_{fee}
        , receiver_list_{std::move(receiver_list)}
        , locktime_(locktime)
        , sequence_(sequence)
        , exclude_etp_range_(exclude_etp_range)
    {
    };

    virtual ~base_transfer_common()
    {
        receiver_list_.clear();
        from_list_.clear();
    };

    static const uint64_t maximum_fee{10000000000};
    static const uint64_t minimum_fee{10000};
    static const uint64_t tx_limit{677};

    virtual bool get_spendable_output(chain::output&, const chain::history&, uint64_t height) const;
    virtual chain::operation::stack get_script_operations(const receiver_record& record) const;
    virtual void sync_fetchutxo(
            const std::string& prikey, const std::string& addr, filter filter = FILTER_ALL, const chain::history::list& spec_rows={});
    virtual chain::attachment populate_output_attachment(const receiver_record& record);
    virtual void sum_payments();
    virtual void sum_payment_amount();
    virtual void populate_change();
    virtual void populate_tx_outputs();
    virtual void populate_unspent_list() = 0;
    virtual void sign_tx_inputs();
    virtual void send_tx();

    void populate_tx_header();

    // common functions, single responsibility.
    static void check_fee_in_valid_range(uint64_t fee);
    void check_receiver_list_not_empty() const;
    bool is_payment_satisfied(filter filter = FILTER_ALL) const;
    void check_payment_satisfied(filter filter = FILTER_ALL) const;
    void check_model_param_initial(std::string& param, uint64_t amount);

    static chain::operation::stack get_pay_key_hash_with_attenuation_model_operations(
            const std::string& model_param, const receiver_record& record);

    static chain::operation::stack get_pay_key_hash_with_lock_height_operations(
            uint16_t lock_cycle, const receiver_record& record);

    void populate_etp_change(const std::string& address = std::string(""));
    void populate_asset_change(const std::string& address = std::string(""));
    void populate_tx_inputs();
    void check_tx();
    void exec();

    std::string get_mychange_address(filter filter) const;

    tx_type& get_transaction() { return tx_; }
    const tx_type& get_transaction() const { return tx_; }

    // in secondary issue, locked asset can also verify threshold condition
    virtual bool is_locked_asset_as_payment() const {return false;}

    virtual bool filter_out_address(const std::string& address) const;

    virtual std::string get_sign_tx_multisig_script(const address_asset_record& from) const;

    void set_did_verify_attachment(const receiver_record& record, chain::attachment& attach);

    virtual bool include_input_script() const { return false; }

protected:
    bc::blockchain::block_chain_impl& blockchain_;
    tx_type                           tx_; // target transaction
    std::string                       symbol_;
    std::string                       from_;
    std::string                       mychange_;
    uint64_t                          tx_item_idx_{0};
    uint64_t                          payment_etp_{0};
    uint64_t                          payment_asset_{0};
    uint64_t                          unspent_etp_{0};
    uint64_t                          unspent_asset_{0};
    std::vector<chain::asset_cert_type>      payment_asset_cert_;
    std::vector<chain::asset_cert_type>      unspent_asset_cert_;
    uint8_t                           payment_did_{0};
    uint8_t                           unspent_did_{0};
    uint8_t                           payment_mit_{0};
    uint8_t                           unspent_mit_{0};
    std::vector<receiver_record>      receiver_list_;
    std::vector<address_asset_record> from_list_;
    uint32_t                          locktime_;
    uint32_t                          sequence_;
    exclude_range_t                   exclude_etp_range_;
};

class BCX_API base_transfer_helper : public base_transfer_common
{
public:
    base_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee,
        std::string&& symbol = std::string(""),
        std::string&& change = std::string(""),
        uint32_t locktime = 0,
        uint32_t sequence = bc::max_input_sequence,
        exclude_range_t exclude_etp_range = {0, 0})
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), std::move(from),
            std::move(change), locktime, sequence, exclude_etp_range)
        , cmd_{cmd}
        , name_{std::move(name)}
        , passwd_{std::move(passwd)}
    {}

    ~base_transfer_helper()
    {}

    void populate_unspent_list() override;

protected:
    command&                          cmd_;
    std::string                       name_;
    std::string                       passwd_;
};

class BCX_API base_multisig_transfer_helper : public base_transfer_helper
{
public:
    base_multisig_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee, std::string&& symbol,
        chain::account_multisig&& multisig_from, uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        , multisig_(std::move(multisig_from))
    {}

    ~base_multisig_transfer_helper()
    {}

    bool filter_out_address(const std::string& address) const override;

    std::string get_sign_tx_multisig_script(const address_asset_record& from) const override;

    void send_tx() override;

protected:
    // for multisig address
    chain::account_multisig multisig_;
};

class BCX_API base_transaction_constructor : public base_transfer_common
{
public:
    base_transaction_constructor(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, receiver_record::list&& receiver_list,
        std::string&& symbol, std::string&& change,
        std::string&& message, uint64_t fee, uint32_t locktime = 0,
        bool include_input_script = false)
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), "", std::move(change), locktime)
        , type_{type}
        , message_{std::move(message)}
        , from_vec_{std::move(from_vec)}
        , include_input_script_(include_input_script)
    {}

    virtual ~base_transaction_constructor()
    {
        from_vec_.clear();
    };

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    // no operation in exec
    void sign_tx_inputs() override {}
    void send_tx() override {}

    bool include_input_script() const override { return include_input_script_; }

protected:
    utxo_attach_type                  type_{utxo_attach_type::invalid};
    std::string                       message_;
    std::vector<std::string>          from_vec_; // from address vector
    bool include_input_script_; // set input's script for offline sign
};

class BCX_API sending_etp : public base_transfer_helper
{
public:
    sending_etp(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        std::string&& change, uint64_t fee, uint32_t locktime = 0,
        exclude_range_t exclude_etp_range = {0, 0})
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, "", std::move(change), locktime,
            bc::max_input_sequence, exclude_etp_range)
    {}

    ~sending_etp(){}
};

class BCX_API lock_sending : public base_transfer_helper
{
public:
    lock_sending(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        std::string&& change, uint64_t fee, uint32_t sequence)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, "", std::move(change), 0, sequence)
    {}

    ~lock_sending(){}

    chain::operation::stack get_script_operations(const receiver_record& record) const override;
};

class BCX_API sending_multisig_tx : public base_multisig_transfer_helper
{
public:
    sending_multisig_tx(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list, uint64_t fee,
        chain::account_multisig& multisig, std::string&& symbol = std::string(""),
        uint32_t locktime = 0)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig), locktime)
    {}

    ~sending_multisig_tx(){}

    void populate_change() override;
};

class BCX_API issuing_asset : public base_transfer_helper
{
public:
    issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param, std::string&& mining_sussidy_param,
        receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t fee_percentage_to_miner,
        uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        , attenuation_model_param_{std::move(model_param)}
        , mining_sussidy_param_{std::move(mining_sussidy_param)}
        , fee_percentage_to_miner_(fee_percentage_to_miner)
    {}

    ~issuing_asset(){}

    void sum_payments() override;
    void sum_payment_amount() override;
    chain::attachment populate_output_attachment(const receiver_record& record) override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::shared_ptr<chain::asset_detail> unissued_asset_;
    std::string domain_cert_address_;
    std::string attenuation_model_param_;
    std::string mining_sussidy_param_;
    uint32_t fee_percentage_to_miner_;
};

class BCX_API secondary_issuing_asset : public base_transfer_helper
{
public:
    secondary_issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list,
        uint64_t fee, uint64_t volume, uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        , volume_(volume)
        , attenuation_model_param_{std::move(model_param)}
    {}

    ~secondary_issuing_asset(){}

    void sum_payment_amount() override;
    void populate_change() override;
    chain::attachment populate_output_attachment(const receiver_record& record) override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

    uint64_t get_volume() { return volume_; };

    bool is_locked_asset_as_payment() const override {return true;}

private:
    uint64_t volume_{0};
    std::shared_ptr<chain::asset_detail> issued_asset_;
    std::string target_address_;
    std::string attenuation_model_param_;
};

class BCX_API sending_asset : public base_transfer_helper
{
public:
        sending_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee,
        std::string&& message, std::string&& change,
        uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee,
            std::move(symbol), std::move(change), locktime)
        , attenuation_model_param_{std::move(model_param)}
        , message_{std::move(message)}
    {}

    ~sending_asset()
    {}

    void sum_payment_amount() override;
    void populate_change() override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::string attenuation_model_param_;
    std::string message_;
};

class BCX_API registering_did : public base_multisig_transfer_helper
{
public:
    registering_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t fee_percentage_to_miner,
        chain::account_multisig&& multisig, uint32_t locktime = 0)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig), locktime)
        , fee_percentage_to_miner_(fee_percentage_to_miner)
    {}

    ~registering_did()
    {}

    void sum_payment_amount() override;

private:
    uint32_t fee_percentage_to_miner_;
};

class BCX_API sending_multisig_did : public base_transfer_helper
{
public:
    sending_multisig_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        chain::account_multisig&& multisig, chain::account_multisig&& multisigto,
        uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        , fromfee(feefrom)
        , multisig_from_(std::move(multisig))
        , multisig_to_(std::move(multisigto))
    {}

    ~sending_multisig_did()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    std::string get_sign_tx_multisig_script(const address_asset_record& from) const override;

    // no operation in exec
    void send_tx() override {}

private:
    std::string fromfee;
    chain::account_multisig multisig_from_;
    chain::account_multisig multisig_to_;
};

class BCX_API sending_did : public base_transfer_helper
{
public:
    sending_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        ,fromfee(feefrom)
    {}

    ~sending_did()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

private:
    std::string fromfee;
};

class BCX_API transferring_asset_cert : public base_multisig_transfer_helper
{
public:
    transferring_asset_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        chain::account_multisig&& multisig_from, uint32_t locktime = 0)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig_from), locktime)
    {}

    ~transferring_asset_cert()
    {}
};

class BCX_API issuing_asset_cert : public base_transfer_helper
{
public:
    issuing_asset_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
    {}

    ~issuing_asset_cert()
    {}

    void sum_payment_amount() override;
};

class BCX_API registering_mit : public base_transfer_helper
{
public:
    registering_mit(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, std::map<std::string, std::string>&& mit_map,
        receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t locktime = 0)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol), "", locktime)
        , mit_map_(mit_map)
    {}

    ~registering_mit()
    {}

    chain::attachment populate_output_attachment(const receiver_record& record) override;

private:
    std::map<std::string, std::string> mit_map_;
};

class BCX_API transferring_mit : public base_multisig_transfer_helper
{
public:
    transferring_mit(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        chain::account_multisig&& multisig_from, uint32_t locktime = 0)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol),
            std::move(multisig_from), locktime)
    {}

    ~transferring_mit()
    {}

    chain::attachment populate_output_attachment(const receiver_record& record) override;
};


} // commands
} // explorer
} // libbitcoin
