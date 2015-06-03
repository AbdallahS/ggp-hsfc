#include <iterator>
#include <sstream>
#include <climits>
#include <boost/functional/hash.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

namespace HSFC
{

/*****************************************************************************************
 * PortableState
 *****************************************************************************************/

PortableState::PortableState() : round_(0), currentstep_(0)
{ }

PortableState::PortableState(const State& state)
{
    state.manager_->GetStateData(*state.state_, relationset_, round_, currentstep_);
}

PortableState::PortableState(const PortableState& other) :
round_(other.round_), currentstep_(other.currentstep_),
relationset_(other.relationset_)
{ }

PortableState& PortableState::operator=(const PortableState& other)
{
    round_ = other.round_;
    currentstep_ = other.currentstep_;
    relationset_ = other.relationset_;
    return *this;
}


bool PortableState::operator==(const PortableState& other) const
{
    return (round_ == other.round_ && currentstep_ == other.currentstep_ &&
            relationset_ == other.relationset_);
}

bool PortableState::operator!=(const PortableState& other) const
{
    return !(*this == other);
}

bool PortableState::operator<(const PortableState& other) const
{
    if (round_ != other.round_) return round_ < other.round_;
    if (currentstep_ != other.currentstep_) return currentstep_ < other.currentstep_;
    return relationset_ < other.relationset_;
}

std::size_t PortableState::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, round_);
    boost::hash_combine(seed, currentstep_);
    boost::hash_combine(seed, relationset_);
    return seed;
}

std::size_t hash_value(const PortableState& ps)
{
    return ps.hash_value();
}

/*****************************************************************************************
 * PortablePlayer
 *****************************************************************************************/
PortablePlayer::PortablePlayer() : roleid_(UINT_MAX)
{ }

PortablePlayer::PortablePlayer(const Player& player) : roleid_(player.roleid_)
{ }

PortablePlayer::PortablePlayer(const PortablePlayer& other) : roleid_(other.roleid_)
{ }

PortablePlayer& PortablePlayer::operator=(const PortablePlayer& other)
{
    roleid_ = other.roleid_;
    return *this;
}

bool PortablePlayer::operator==(const PortablePlayer& other) const
{
    return (roleid_ == other.roleid_);
}

bool PortablePlayer::operator!=(const PortablePlayer& other) const
{
    return (roleid_ != other.roleid_);
}

bool PortablePlayer::operator<(const PortablePlayer& other) const
{
    return (roleid_ < other.roleid_);
}

std::size_t PortablePlayer::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, roleid_);
    return seed;
}

std::size_t hash_value(const PortablePlayer& pp)
{
    return pp.hash_value();
}

/*****************************************************************************************
 * PortableMove
 *****************************************************************************************/
PortableMove::PortableMove() : RoleIndex_(-1), RelationIndex_(-1), ID_(-1)
{ }

PortableMove::PortableMove(const Move& move) :
    RoleIndex_(move.move_.RoleIndex), Text_(move.move_.Text),
    RelationIndex_(move.move_.Tuple.RelationIndex), ID_(move.move_.Tuple.ID)
{ }

PortableMove::PortableMove(const PortableMove& other) :
    RoleIndex_(other.RoleIndex_), Text_(other.Text_),
    RelationIndex_(other.RelationIndex_), ID_(other.ID_)
{ }

PortableMove& PortableMove::operator=(const PortableMove& other)
{
    RoleIndex_ = other.RoleIndex_;
    RelationIndex_ = other.RelationIndex_;
    ID_ = other.ID_;
    Text_ = other.Text_;
    return *this;
}

bool PortableMove::operator==(const PortableMove& other) const
{
    return (RoleIndex_ == other.RoleIndex_ && RelationIndex_ == other.RelationIndex_ &&
            ID_ == other.ID_ && Text_ == other.Text_);
}

bool PortableMove::operator!=(const PortableMove& other) const
{
    return (RoleIndex_ != other.RoleIndex_ || RelationIndex_ != other.RelationIndex_ ||
            ID_ != other.ID_ || Text_ != other.Text_);
}

bool PortableMove::operator<(const PortableMove& other) const
{
    if (RoleIndex_ != other.RoleIndex_) return RoleIndex_ < other.RoleIndex_;
    if (RelationIndex_ != other.RelationIndex_) return RelationIndex_ < other.RelationIndex_;
    if (ID_ != other.ID_) return ID_ < other.ID_;
    return Text_ < other.Text_;
}

std::size_t PortableMove::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, RoleIndex_);
    boost::hash_combine(seed, RelationIndex_);
    boost::hash_combine(seed, ID_);
    boost::hash_combine(seed, Text_);
    return seed;
}

std::size_t hash_value(const PortableMove& pm)
{
    return pm.hash_value();
}

}; /* namespace HSFC */
