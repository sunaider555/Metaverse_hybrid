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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP
#define MVS_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/define.hpp>
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/simple_chain.hpp>
#include <metaverse/blockchain/transaction_pool.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/consensus/fts.hpp>
#include <metaverse/blockchain/profile.hpp>

#define  LOG_BLOCK_CHAIN_IMPL  "block_chain_impl"

namespace libbitcoin {
namespace blockchain {

typedef console_result operation_result;

class block_chain_impl;

// encap to safer write database
class block_chain_writer {
public:
    block_chain_writer(block_chain_impl& chain);
    ~block_chain_writer();
private:
    block_chain_impl& chain_;
};

/// The simple_chain interface portion of this class is not thread safe.
class BCB_API block_chain_impl
  : public block_chain, public simple_chain
{
    friend class block_chain_writer;
public:
    block_chain_impl(threadpool& pool,
        const blockchain::settings& chain_settings,
        const database::settings& database_settings);

    /// The database is closed on destruct, threads must be joined.
    ~block_chain_impl();

    /// This class is not copyable.
    block_chain_impl(const block_chain_impl&) = delete;
    void operator=(const block_chain_impl&) = delete;

    // Properties (thread safe).
    // ------------------------------------------------------------------------

    // Get a reference to the transaction pool.
    transaction_pool& pool();

    // Get a reference to the blockchain configuration settings.
    const settings& chain_settings() const;

    // block_chain start/stop (thread safe).
    // ------------------------------------------------------------------------

    /// Start or restart the blockchain.
    virtual bool start() override;

    /// Signal stop of current work, speeds shutdown with multiple threads.
    virtual bool stop() override;

    /// Close the blockchain, threads must first be joined, can be restarted.
    virtual bool close() override;

    // simple_chain (NOT THREAD SAFE).
    // ------------------------------------------------------------------------

    /// Return the first and last gaps in the blockchain, or false if none.
    bool get_gap_range(uint64_t& out_first, uint64_t& out_last) const override;

    /// Return the next chain gap at or after the specified start height.
    bool get_next_gap(uint64_t& out_height, uint64_t start_height) const override;

    /// Get the dificulty of a block at the given height.
    bool get_difficulty(u256& out_difficulty, uint64_t height) const override;

    /// Get the header of the block at the given height.
    bool get_header(chain::header& out_header, uint64_t height) const override;
    uint64_t get_transaction_count(uint64_t block_height) const;
    uint32_t get_block_timestamp(uint64_t height) const;

    bool get_signature(ec_signature& blocksig, uint64_t height) const override;
    bool get_public_key(ec_compressed& public_key, uint64_t height) const override;
    bool get_signature_and_public_key(ec_signature& blocksig, ec_compressed& public_key, uint64_t height) const override;

    /// Get the height of the block with the given hash.
    bool get_height(uint64_t& out_height, const hash_digest& block_hash) const override;

    /// Get height of latest block.
    bool get_last_height(uint64_t& out_height) const override;

    /// Get the hash digest of the transaction of the outpoint.
    bool get_outpoint_transaction(hash_digest& out_transaction,
        const chain::output_point& outpoint) const override;

    /// Get the transaction of the given hash and its block height.
    bool get_transaction(chain::transaction& out_transaction,
        uint64_t& out_block_height, const hash_digest& transaction_hash) const override;

    /// Import a block to the blockchain.
    bool import(chain::block::ptr block, uint64_t height) override;

    /// Append the block to the top of the chain.
    bool push(block_detail::ptr block) override;

    /// Remove blocks at or above the given height, returning them in order.
    bool pop_from(block_detail::list& out_blocks, uint64_t height) override;

    // block_chain queries (thread safe).
    // ------------------------------------------------------------------------

    /// Store a block to the blockchain, with indexing and validation.
    void store(message::block_message::ptr block,
        block_store_handler handler) override;

    /// fetch a block by height.
    void fetch_block(uint64_t height, block_fetch_handler handler) override;

    /// fetch a block by height.
    void fetch_block(const hash_digest& hash, block_fetch_handler handler) override;

    /// fetch block header by height.
    void fetch_block_header(uint64_t height,
        block_header_fetch_handler handler) override;

    /// fetch block header by hash.
    void fetch_block_header(const hash_digest& hash,
        block_header_fetch_handler handler) override;

    /// fetch a merkle block by height.
    void fetch_merkle_block(uint64_t height,
        merkle_block_fetch_handler handler) override;

    /// fetch a merkle block by height.
    void fetch_merkle_block(const hash_digest& hash,
        merkle_block_fetch_handler handler) override;

    /// fetch hashes of transactions for a block, by block height.
    void fetch_block_transaction_hashes(uint64_t height,
        transaction_hashes_fetch_handler handler) override;

    /// fetch hashes of transactions for a block, by block hash.
    void fetch_block_transaction_hashes(const hash_digest& hash,
        transaction_hashes_fetch_handler handler) override;

    /// fetch hashes of transactions for a block, by block height.
    void fetch_block_signature(uint64_t height,
                               block_signature_fetch_handler handler) override;

    /// fetch hashes of transactions for a block, by block hash.
    void fetch_block_signature(const hash_digest& hash,
                               block_signature_fetch_handler handler) override;

    void fetch_block_public_key(uint64_t height,
        block_public_key_fetch_handler handler) override;

    void fetch_block_public_key(const hash_digest& hash,
        block_public_key_fetch_handler handler) override;

    /// fetch a block locator relative to the current top and threshold.
    void fetch_block_locator(block_locator_fetch_handler handler) override;

    /// fetch the set of block hashes indicated by the block locator.
    void fetch_locator_block_hashes(const message::get_blocks& locator,
        const hash_digest& threshold, size_t limit,
        locator_block_hashes_fetch_handler handler) override;

    /// fetch the set of block headers indicated by the block locator.
    void fetch_locator_block_headers(const message::get_headers& locator,
        const hash_digest& threshold, size_t limit,
        locator_block_headers_fetch_handler handler) override;

    /// fetch height of block by hash.
    void fetch_block_height(const hash_digest& hash,
        block_height_fetch_handler handler) override;

    /// fetch height of latest block.
    void fetch_last_height(last_height_fetch_handler handler) override;

    /// fetch transaction by hash.
    void fetch_transaction(const hash_digest& hash,
        transaction_fetch_handler handler) override;

    /// fetch height and offset within block of transaction by hash.
    void fetch_transaction_index(const hash_digest& hash,
        transaction_index_fetch_handler handler) override;

    /// fetch spend of an output point.
    void fetch_spend(const chain::output_point& outpoint,
        spend_fetch_handler handler) override;

    /// fetch outputs, values and spends for an address.
    void fetch_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height, history_fetch_handler handler) override;

    bool fetch_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height, chain::history_compact::list& history);


    chain::history::list get_address_history(const wallet::payment_address& addr, bool add_memory_pool = false);


    /// fetch stealth results.
    void fetch_stealth(const binary& filter, uint64_t from_height,
        stealth_fetch_handler handler) override;

    /// filter out block hashes that exist in the store.
    virtual void filter_blocks(message::get_data::ptr message,
        result_handler handler) override;

    /// filter out block hashes that exist in the orphan pool.
    virtual void filter_orphans(message::get_data::ptr message,
        result_handler handler) override;

    /// filter out transaction hashes that exist in the store.
    virtual void filter_transactions(message::get_data::ptr message,
        result_handler handler) override;

    /// Subscribe to blockchain reorganizations.
    virtual void subscribe_reorganize(reorganize_handler handler) override;

    /// must be have enough etp locked in the address
    virtual bool check_pos_capability(
        uint64_t best_height,
        const wallet::payment_address& pay_addres) override;

    /// select pos utxo. target value
    virtual uint32_t select_utxo_for_staking(
        const u256& bits,
        uint64_t best_height,
        const wallet::payment_address& pay_addres,
        std::shared_ptr<chain::output_info::list> stake_outputs = nullptr,
        uint32_t max_count = max_uint32) override;

    bool check_pos_utxo_height_and_value(
        const u256& bits,
        const uint64_t& out_height,
        const uint64_t& curr_height,
        const uint64_t& value);

    bool check_pos_utxo_capability(
        const u256& bits,
        const uint64_t& height,
        const chain::transaction& tx,
        const uint32_t& out_index ,
        const uint64_t& out_height,
        bool strict=true
    );

    bool pos_exist_before(const uint64_t& height) const;

    virtual chain::header::ptr get_last_block_header(const chain::header& parent_header, uint32_t version) const;

    inline hash_digest get_hash(const std::string& str) const;
    inline short_hash get_short_hash(const std::string& str) const;

    std::shared_ptr<chain::transaction>  get_spends_output(const chain::input_point& input);

    // account related api
    std::shared_ptr<chain::account> is_account_passwd_valid(const std::string& name, const std::string& passwd);
    std::string is_account_lastwd_valid(const chain::account& acc, std::string& auth, const std::string& lastwd);
    void set_account_passwd(const std::string& name, const std::string& passwd);
    bool is_account_exist(const std::string& name);
    bool is_admin_account(const std::string& name);
    operation_result store_account(std::shared_ptr<chain::account> acc);
    std::shared_ptr<chain::account> get_account(const std::string& name);
    std::shared_ptr<std::vector<chain::account>> get_accounts();
    operation_result delete_account(const std::string& name);
    operation_result delete_account_address(const std::string& name);

    std::shared_ptr<chain::business_history::list> get_address_business_history(
        const std::string& addr, chain::business_kind kind, uint8_t confirmed);
    std::shared_ptr<chain::business_history::list> get_address_business_history(
        const std::string& addr, chain::business_kind kind, uint32_t time_begin, uint32_t time_end);
    std::shared_ptr<chain::business_history::list> get_address_business_history(const std::string& addr);

    // account asset api
    operation_result store_account_asset(const chain::asset_detail& detail, const std::string& name);
    operation_result store_account_asset(std::shared_ptr<chain::asset_detail> detail, const std::string& name);
    operation_result delete_account_asset(const std::string& name);
    std::shared_ptr<chain::business_address_asset::list> get_account_asset(
        const std::string& name, const std::string& asset_name, chain::business_kind kind);
    std::shared_ptr<chain::business_address_asset::list> get_account_asset(
        const std::string& name, const std::string& asset);
    std::shared_ptr<chain::business_address_asset::list> get_account_assets(const std::string& name);
    std::shared_ptr<chain::business_address_asset::list> get_account_assets(
        const std::string& name, chain::business_kind kind);
    uint64_t get_address_asset_volume(const std::string& address, const std::string& asset);
    uint64_t get_account_asset_volume(const std::string& account, const std::string& asset);
    uint64_t get_asset_volume(const std::string& asset);

    // asset api
    bool is_asset_exist(const std::string& asset_name, bool check_local_db=true);
    uint64_t get_asset_height(const std::string& asset_name) const ;
    std::shared_ptr<chain::asset_detail::list> get_local_assets();
    std::shared_ptr<chain::asset_detail::list> get_issued_assets(
        const std::string& symbol="", const std::string& address="");
    std::shared_ptr<chain::asset_detail> get_issued_asset(const std::string& symbol);
    std::shared_ptr<chain::blockchain_asset> get_issued_blockchain_asset(const std::string& symbol);
    std::shared_ptr<chain::business_address_asset::list> get_account_assets();
    std::shared_ptr<chain::business_address_asset::list> get_account_unissued_assets(const std::string& name);
    std::shared_ptr<chain::asset_detail> get_account_unissued_asset(
        const std::string& name, const std::string& symbol);

    std::shared_ptr<chain::blockchain_asset::list> get_asset_register_output(const std::string& symbol);
    // cert api
    std::shared_ptr<chain::asset_cert> get_asset_cert(const std::string& symbol, chain::asset_cert_type cert_type) const;
    bool is_asset_cert_exist(const std::string& symbol, chain::asset_cert_type cert_type);
    uint64_t get_asset_cert_height(const std::string& cert_symbol, chain::asset_cert_type cert_type);
    std::shared_ptr<chain::asset_cert::list> get_issued_asset_certs(
        const std::string& address = "", chain::asset_cert_type cert_type = asset_cert_ns::none);
    std::shared_ptr<chain::asset_cert> get_account_asset_cert(
        const std::string& account, const std::string& symbol, chain::asset_cert_type cert_type);
    std::shared_ptr<chain::business_address_asset_cert::list> get_account_asset_certs(
        const std::string& account, const std::string& symbol, chain::asset_cert_type cert_type);
    std::shared_ptr<chain::business_address_asset_cert::list> get_address_asset_certs(
        const std::string& address, const std::string& symbol, chain::asset_cert_type cert_type);

    // identifiable asset
    bool is_asset_mit_exist(const std::string& symbol);
    uint64_t get_asset_mit_height(const std::string& mit_symbol)const;
    std::shared_ptr<chain::asset_mit_info> get_registered_mit(const std::string& symbol);
    std::shared_ptr<chain::asset_mit_info::list> get_registered_mits();
    std::shared_ptr<chain::asset_mit_info::list> get_mit_history(const std::string& symbol,
        uint64_t limit = 0, uint64_t page_number = 0);
    std::shared_ptr<chain::asset_mit::list> get_account_mits(
        const std::string& account, const std::string& symbol="");

    // account did api
    bool is_did_exist(const std::string& symbol) const;
    uint64_t get_did_height(const std::string& symbol) const;
    bool is_address_registered_did(const std::string& address, uint64_t fork_index = max_uint64);
    bool is_account_owned_did(const std::string& account, const std::string& symbol);
    std::string get_did_from_address(const std::string& address, uint64_t fork_index = max_uint64);
    std::shared_ptr<chain::did_detail> get_registered_did(const std::string& symbol) const;
    std::shared_ptr<chain::did_detail::list> get_registered_dids();
    std::shared_ptr<chain::did_detail::list> get_account_dids(const std::string& account);

    //get history addresses from did symbol
    std::shared_ptr<chain::blockchain_did::list>  get_did_history_addresses(const std::string & symbol);

    std::shared_ptr<chain::business_history::list> get_account_business_history(
        const std::string& name, chain::business_kind kind, uint32_t time_begin, uint32_t time_end);
    std::shared_ptr<chain::business_history::list> get_address_business_history(
        const std::string& addr, const std::string& symbol, chain::business_kind kind, uint8_t confirmed);
    std::shared_ptr<chain::business_record::list> get_address_business_record(
        const std::string& addr, size_t from_height = 0, size_t limit = 0);
    std::shared_ptr<chain::business_record::list> get_address_business_record(
        const std::string& addr, uint64_t start, uint64_t end, const std::string& symbol);
    std::shared_ptr<chain::business_record::list> get_address_business_record(
        const std::string& address, const std::string& symbol, size_t start_height, size_t end_height,
        uint64_t limit, uint64_t page_number) const;
    std::shared_ptr<chain::account_address::list> get_addresses();

    // account message api
    std::shared_ptr<chain::business_address_message::list> get_account_messages(const std::string& name);

    // account adress related api
    operation_result store_account_address(std::shared_ptr<chain::account_address> address);
    std::shared_ptr<chain::account_address> get_account_address(const std::string& name, const std::string& address);
    std::shared_ptr<chain::account_address::list> get_account_addresses(const std::string& name);
    void uppercase_symbol(std::string& symbol);

    static bool is_valid_address(const std::string& address);
    static bool is_payment_address(const std::string& address);
    static bool is_stealth_address(const std::string& address);
    static bool is_script_address(const std::string& address);
    static bool is_blackhole_address(const std::string& address);

    void fired() override;
    organizer& get_organizer() override;
    bool get_transaction_consider_pool(
        chain::transaction& tx, uint64_t& tx_height, const hash_digest& hash, bool* is_in_pool=nullptr);
    bool get_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height, chain::history_compact::list& history);
    code validate_transaction(const chain::transaction& tx);
    code broadcast_transaction(const chain::transaction& tx);
    bool get_tx_inputs_etp_value (chain::transaction& tx, uint64_t& etp_val);
    void safe_store_account(chain::account& acc, std::vector<std::shared_ptr<chain::account_address>>& addresses);

    shared_mutex& get_mutex();
    bool is_sync_disabled() const;
    void set_sync_disabled(bool b);

    uint64_t get_height();
    uint64_t calc_number_of_blocks(uint64_t from, uint64_t to) const;
    uint64_t get_expiration_height(uint64_t from, uint64_t lock_height) const;

    std::pair<uint64_t, uint64_t> get_locked_balance(
        uint64_t epoch_height, const std::string& address);

    std::shared_ptr<std::vector<std::pair<std::string, data_chunk>>> get_witnesses_addr_pubkey(
        block_chain_impl& chain,
        uint64_t epoch_height,
        std::shared_ptr<std::vector<std::string>> excluded_addresses);

    std::shared_ptr<consensus::fts_stake_holder::ptr_list> get_witnesses_mars(
        uint64_t epoch_height,
        std::shared_ptr<std::vector<std::string>> excluded_addresses);

    bool can_use_dpos(uint64_t height) const;
    bool can_use_dpos_impl(
        uint64_t height,
        std::function<bool(chain::header&, uint64_t)> const &get_header_func) const;

    chain::header::ptr get_prev_block_header(
        uint64_t height, chain::block_version ver, bool same_version=true) const;
    chain::header::ptr get_prev_block_header_impl(
        uint64_t height, chain::block_version ver, bool same_version,
        std::function<bool(chain::header&, uint64_t)> const &get_header_func) const;

    bool check_max_successive_height(uint64_t last_height, chain::block_version version) const;
    bool check_max_successive_height_impl(
        uint64_t height, chain::block_version version,
        std::function<chain::header::ptr(uint64_t, chain::block_version, bool)> const &get_prev_header_func) const;

    uint32_t get_median_time_past(uint64_t height) const;
    bool is_utxo_spendable(const chain::transaction& tx, uint32_t index,
                           uint64_t tx_height, uint64_t latest_height) const;

    static bool is_valid_symbol(const std::string& symbol, uint32_t tx_version);
    static bool is_valid_did_symbol(const std::string& symbol,  bool check_sensitive = false);
    static bool is_valid_mit_symbol(const std::string& symbol,  bool check_sensitive = false);

    profile::ptr get_profile(const profile_context&) const;

    uint64_t get_witness_stake_mars(
        const std::string& address, uint64_t epoch_height);

    uint64_t get_witness_cert_mars(
        const std::string& address, uint64_t epoch_height);

    std::shared_ptr<chain::asset_cert::list> get_address_witness_certs(
        const std::string& address, uint64_t epoch_height);

    // use witness_cert database
    bool is_primary_witness_cert_actived(
        const std::string& symbol, uint64_t epoch_height);

    // use witness_cert database
    std::shared_ptr<chain::blockchain_cert::list> get_issued_witness_certs(
        const std::string& symbol = "",
        const std::string& address = "",
        uint64_t epoch_height = 0);

    // use witness_cert database
    std::shared_ptr<chain::blockchain_cert::list> get_issued_secondary_witness_certs(
        const std::string& primary_symbol = "",
        uint64_t epoch_height = 0);

    // use witness_cert database
    bool is_secondary_witness_cert_exists(
        const std::string& symbol,
        uint64_t expiration = 0);

    operation_result calc_and_store_witness_profile(uint64_t epoch_height);

private:
    typedef std::function<bool(database::handle)> perform_read_functor;

    template <typename Handler, typename... Args>
    bool finish_fetch(database::handle handle, Handler handler, Args&&... args)
    {
        if (!database_.is_read_valid(handle))
            return false;

        handler(std::forward<Args>(args)...);
        return true;
    }

    template <typename Handler, typename... Args>
    void stop_write(Handler handler, Args&&... args)
    {
        const auto result = database_.end_write();
        BITCOIN_ASSERT(result);
        handler(std::forward<Args>(args)...);
    }

    void stop_write();
    void start_write();
    void do_store(message::block_message::ptr block,
        block_store_handler handler);

    ////void fetch_ordered(perform_read_functor perform_read);
    ////void fetch_parallel(perform_read_functor perform_read);
    void fetch_serial(perform_read_functor perform_read);
    bool stopped() const;

    std::string get_asset_symbol_from_business_data(const chain::business_data& data) const;

private:
    std::atomic<bool> stopped_;
    std::atomic<bool> sync_disabled_;
    const settings& settings_;

    // These are thread safe.
    organizer organizer_;
    ////dispatcher read_dispatch_;
    ////dispatcher write_dispatch_;
    blockchain::transaction_pool transaction_pool_;

    // This is protected by mutex.
    database::data_base database_;
    shared_mutex mutex_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
