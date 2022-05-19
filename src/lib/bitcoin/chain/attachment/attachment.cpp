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
#include <metaverse/bitcoin/chain/attachment/attachment.hpp>
#include <metaverse/bitcoin/chain/attachment/variant_visitor.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

attachment::attachment()
{
    reset();
}

attachment::attachment(uint32_t type)
{
    reset();
    this->type = type;
}

attachment::attachment(const std::string& from_did, const std::string& to_did)
    : version(DID_ATTACH_VERIFY_VERSION)
    , type(0) //attachment_type::attach_none;
    , todid(to_did)
    , fromdid(from_did)
{
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, attach);
}

attachment::attachment(attachment&& other)
    : version(other.version)
    , type(other.type)
    , todid(std::move(other.todid))
    , fromdid(std::move(other.fromdid))
    , attach(std::move(other.attach))
{
}

attachment::attachment(const attachment& other)
    : version(other.version)
    , type(other.type)
    , todid(other.todid)
    , fromdid(other.fromdid)
    , attach(other.attach)
{
}

attachment& attachment::operator=(attachment&& other)
{
    version = other.version;
    type = other.type;
    todid = std::move(other.todid);
    fromdid = std::move(other.fromdid);
    attach = std::move(other.attach);
    return *this;
}

attachment& attachment::operator=(const attachment& other)
{
    version = other.version;
    type = other.type;
    todid = other.todid;
    fromdid = other.fromdid;
    attach = other.attach;
    return *this;
}

void attachment::reset()
{
    version = 0;
    type = 0; //attachment_type::attach_none;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, attach);
    todid = "";
    fromdid = "";
}

bool attachment::is_valid() const
{
    if (!is_valid_type()) {
        return false;
    }
    auto visitor = is_valid_visitor();
    return boost::apply_visitor(visitor, attach);
}

bool attachment::is_valid_type() const
{
    return ((ETP_TYPE == type)
        || (ASSET_TYPE == type)
        || (ASSET_CERT_TYPE == type)
        || (ASSET_MIT_TYPE == type)
        || (MESSAGE_TYPE == type)
        || (ETP_AWARD_TYPE == type)
        || (DID_TYPE == type));
}

bool attachment::from_data_t(reader& source)
{
    reset();

    version = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
        type = source.read_4_bytes_little_endian();

    if (result && version == DID_ATTACH_VERIFY_VERSION) {
            todid = source.read_string();
            fromdid = source.read_string();
    }

    result = static_cast<bool>(source);
    if (result && is_valid_type()) {
        switch(type) {
            case ETP_TYPE:
            {
                attach = etp();
                break;
            }
            case ETP_AWARD_TYPE:
            {
                attach = etp_award();
                break;
            }
            case ASSET_TYPE:
            {
                attach = asset();
                break;
            }
            case ASSET_CERT_TYPE:
            {
                attach = asset_cert();
                break;
            }
            case ASSET_MIT_TYPE:
            {
                attach = asset_mit();
                break;
            }
            case MESSAGE_TYPE:
            {
                attach = blockchain_message();
                break;
            }
            case DID_TYPE:
            {
                attach = did();
                break;
            }
        }

        auto visitor = from_data_visitor(source);
        result = boost::apply_visitor(visitor, attach);
    }
    else {
        // result = false;
        // reset();
    }

    return result;
}

void attachment::to_data_t(writer& sink) const
{
    sink.write_4_bytes_little_endian(version);
    sink.write_4_bytes_little_endian(type);
    if (version == DID_ATTACH_VERIFY_VERSION) {
        sink.write_string(todid);
        sink.write_string(fromdid);
    }
    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, attach);
}

uint64_t attachment::serialized_size() const
{
    uint64_t size = 0;
    if(version == DID_ATTACH_VERIFY_VERSION) {
        size = 4 + 4 + (todid.size() + 1) + (fromdid.size() + 1);
    }
    else {
        size = 4 + 4;
    }

    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, attach);

    return size;
}

std::string attachment::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version << "\n"
        << "\t type = " << type << "\n";
    if (version == DID_ATTACH_VERIFY_VERSION) {
        ss << "\t fromdid = " << fromdid << "\n"
            << "\t todid = " << todid << "\n";
    }
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, attach);

    return ss.str();
}

uint32_t attachment::get_version() const
{
    return version;
}
void attachment::set_version(uint32_t version)
{
     this->version = version;
}

uint32_t attachment::get_type() const
{
    return type;
}
void attachment::set_type(uint32_t type)
{
     this->type = type;
}

void attachment::set_null()
{
    reset();
    this->type = ATTACH_NULL_TYPE;
}

std::string attachment::get_to_did() const
{
    return todid;
}
void attachment::set_to_did(const std::string& did)
{
    this->todid = did;
}

std::string attachment::get_from_did() const
{
    return fromdid;
}
void attachment::set_from_did(const std::string& did)
{
     this->fromdid = did;
}

attachment::attachment_data_type& attachment::get_attach()
{
    return this->attach;
}
const attachment::attachment_data_type& attachment::get_attach() const
{
    return this->attach;
}

} // namspace chain
} // namspace libbitcoin
