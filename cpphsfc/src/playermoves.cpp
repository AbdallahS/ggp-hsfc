#include <iterator>
#include <boost/utility.hpp>
#include <hsfc/playermoves.h>


namespace HSFC
{

/*****************************************************************************************
 * Implementation of the main/default PlayerMoves functions
 *****************************************************************************************/

PlayerMoves::PlayerMoves() : viewPlayers(this)
{ }

PlayerMoves::PlayerMoves(const PlayerMoves& other) : 
    viewPlayers(this), playermoves_(other.playermoves_), players_(other.players_)
{ }

PlayerMoves& PlayerMoves::operator=(const PlayerMoves& other)
{
    viewPlayers.pms_ = other.viewPlayers.pms_;
    playermoves_ = other.playermoves_;
    players_ = other.players_;
    return *this;
}

PlayerMoves::iterator PlayerMoves::begin() 
{
    return playermoves_.begin();
}

PlayerMoves::iterator PlayerMoves::end() 
{
    return playermoves_.end();
}

PlayerMoves::const_iterator PlayerMoves::begin() const 
{
    return playermoves_.begin();
}

PlayerMoves::const_iterator PlayerMoves::end() const
{
    return playermoves_.end();
}

PlayerMoves::iterator PlayerMoves::insert(const PlayerMove& pm)
{
    players_.insert(pm.first);
    return playermoves_.insert(pm);
}

PlayerMoves::iterator PlayerMoves::insert(PlayerMoves::const_iterator posn, const PlayerMove& pm)
{
    players_.insert(pm.first);
    return playermoves_.insert(posn, pm);
}

void PlayerMoves::clear()
{ 
    players_.clear();
    playermoves_.clear();
}

bool PlayerMoves::empty() const 
{ 
    return playermoves_.empty(); 
}

PlayerMoves::size_type PlayerMoves::size() const 
{ 
    return playermoves_.size(); 
}

/***************************************************************************************
 * 
 ***************************************************************************************/

PlayerMoves::ViewPlayers::ViewPlayers(PlayerMoves* pms) : pms_(pms)
{ }

PlayerMoves::ViewPlayers::iterator PlayerMoves::ViewPlayers::begin() 
{
    return pms_->players_.begin();
}

PlayerMoves::ViewPlayers::iterator PlayerMoves::ViewPlayers::end() 
{
    return pms_->players_.end();
}

PlayerMoves::ViewPlayers::iterator PlayerMoves::ViewPlayers::begin() const
{
    return pms_->players_.begin();
}

PlayerMoves::ViewPlayers::iterator PlayerMoves::ViewPlayers::end() const
{
    return pms_->players_.end();
}

PlayerMoves::ViewPlayers::iterator PlayerMoves::ViewPlayers::find(Player& p)
{
    return pms_->players_.find(p);
}

PlayerMoves::ViewPlayers::const_iterator PlayerMoves::ViewPlayers::find(Player& p) const
{
    return pms_->players_.find(p);
}

bool PlayerMoves::ViewPlayers::empty() const
{
    return pms_->players_.empty();
}

PlayerMoves::ViewPlayers::size_type PlayerMoves::ViewPlayers::size() const
{
    return pms_->players_.size();
}


/***************************************************************************************
 * 
 ***************************************************************************************/

PlayerMoves::ViewMovesByPlayer::ViewMovesByPlayer(PlayerMoves& pms, const Player& p) 
{
    boost::tie(begin_, end_) = pms.playermoves_.equal_range(p);
}

PlayerMoves::ViewMovesByPlayer::iterator PlayerMoves::ViewMovesByPlayer::begin() 
{
    return boost::make_transform_iterator(begin_, PlayerMoves::PlayerMoveToMove());
}

PlayerMoves::ViewMovesByPlayer::iterator PlayerMoves::ViewMovesByPlayer::end() 
{
    return boost::make_transform_iterator(end_, PlayerMoves::PlayerMoveToMove());
}

PlayerMoves::ViewMovesByPlayer::const_iterator PlayerMoves::ViewMovesByPlayer::begin() const 
{
    return boost::make_transform_iterator(begin_, PlayerMoves::PlayerMoveToMove());
}

PlayerMoves::ViewMovesByPlayer::const_iterator PlayerMoves::ViewMovesByPlayer::end() const
{
    return boost::make_transform_iterator(end_, PlayerMoves::PlayerMoveToMove());
}

bool PlayerMoves::ViewMovesByPlayer::empty() const
{
    return (begin_ == end_);
}

PlayerMoves::ViewMovesByPlayer::size_type PlayerMoves::ViewMovesByPlayer::size() const
{
    return (std::distance(begin_, end_));
}


}; /* namespace HSFC */
