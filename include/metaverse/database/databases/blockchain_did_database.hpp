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
#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/result/transaction_result.hpp>
#include <metaverse/database/primitives/slab_hash_table.hpp>
#include <metaverse/database/primitives/slab_manager.hpp>
#include <metaverse/bitcoin/chain/attachment/did/blockchain_did.hpp>

namespace libbitcoin {
namespace database {

/// This enables lookups of transactions by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This is so we can quickly reconstruct blocks given a list of tx indexes
/// belonging to that block. These are stored with the block.
class BCD_API blockchain_did_database
{
public:
    /// Construct the database.
    blockchain_did_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~blockchain_did_database();

    /// Initialize a new transaction database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    std::shared_ptr<chain::blockchain_did> get(const hash_digest& hash) const;

    ///
    std::shared_ptr<std::vector<chain::blockchain_did> > get_history_dids(const hash_digest& hash) const;
    ///
    std::shared_ptr<std::vector<chain::blockchain_did> > get_blockchain_dids() const;

    /// 
    std::shared_ptr<chain::blockchain_did> get_register_history(const std::string & did_symbol) const;
    ///
    uint64_t get_register_height(const std::string & did_symbol) const;

    std::shared_ptr<std::vector<chain::blockchain_did> > getdids_from_address_history(
        const std::string &address, const uint64_t& fromheight = 0
        ,const uint64_t & toheight = max_uint64 ) const;

    void store(const hash_digest& hash, const chain::blockchain_did& sp_detail);

    /// Delete a transaction from database.
    void remove(const hash_digest& hash);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

    //pop back did_detail
    std::shared_ptr<chain::blockchain_did> pop_did_transfer(const hash_digest &hash);
protected:
    /// update address status(current or old), default old
     std::shared_ptr<chain::blockchain_did> update_address_status(const hash_digest& hash,uint32_t status = chain::blockchain_did::address_history);
private:
    typedef slab_hash_table<hash_digest> slab_map;

    // Hash table used for looking up txs by hash.
    memory_map lookup_file_;
    slab_hash_table_header lookup_header_;
    slab_manager lookup_manager_;
    slab_map lookup_map_;
};

} // namespace database
} // namespace libbitcoin


