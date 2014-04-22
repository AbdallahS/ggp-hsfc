#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/variant/get.hpp>
#include <boost/functional/hash.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of Player
 *****************************************************************************************/
Player::Player(const HSFCManager* manager, unsigned int roleid): manager_(manager), roleid_(roleid)
{ }

Player::Player(const Player& other): manager_(other.manager_), roleid_(other.roleid_)
{ }

Player::Player(Game& game, const PortablePlayer& pp) : 
    manager_(&game.manager_), roleid_(pp.roleid_)
{
    // Note: should probably check that the roleid is valid for this game.
}

std::string Player::tostring() const
{
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

bool Player::operator==(const Player& other) const
{
    return roleid_ == other.roleid_;
}

bool Player::operator!=(const Player& other) const
{
    return roleid_ != other.roleid_;
}

Player& Player::operator=(const Player& other)
{
    this->manager_ = other.manager_;
    this->roleid_ = other.roleid_;
    return *this;
}

std::size_t Player::hash_value() const
{
    // Note: since there is only 1 manager per game
    // we can use the pointer as a hash value.
    size_t seed = 0;
    boost::hash_combine(seed, roleid_);
    boost::hash_combine(seed, manager_);
    return seed;
}


std::size_t hash_value(const Player& player)
{
    return player.hash_value();
}


std::ostream& operator<<(std::ostream& os, const Player& player)
{
    return player.manager_->PrintPlayer(os, player.roleid_);
}


/*****************************************************************************************
 * Implementation of Move
 *****************************************************************************************/

Move::Move(HSFCManager* manager, const hsfcLegalMove& move): manager_(manager), move_(move)
{ }

Move::Move(const Move& other): manager_(other.manager_), move_(other.move_)
{ }

Move::Move(Game& game, const PortableMove& pm) : manager_(&game.manager_)
{
    move_.RoleIndex = pm.RoleIndex_;
    strcpy(move_.Text, pm.Text_.c_str());
    move_.Tuple.RelationIndex = pm.RelationIndex_;
    move_.Tuple.ID = pm.ID_;
    // Note: should probably check that all the values are legit.
}

std::string Move::tostring() const
{
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

bool operator==(const hsfcLegalMove& a, const hsfcLegalMove& b)
{
    return (a.RoleIndex == b.RoleIndex &&
            strcmp(a.Text, b.Text) == 0 &&
            a.Tuple.RelationIndex == b.Tuple.RelationIndex &&
            a.Tuple.ID == b.Tuple.ID);
}

bool operator!=(const hsfcLegalMove& a, const hsfcLegalMove& b)
{
    return !(a == b);
}

bool Move::operator==(const Move& other) const
{
    return (manager_ == other.manager_) && (move_ == other.move_);
}

bool Move::operator!=(const Move& other) const
{
    return (manager_ != other.manager_) || (move_ != other.move_);
}

Move& Move::operator=(const Move& other)
{
    manager_ = other.manager_;
    move_ = other.move_;
    return *this;
}

std::size_t Move::hash_value() const
{
    // Note: 1) since there is only 1 manager per game we can use the manager_
    //          pointer as a hash value.
    //       2) I think the move_.Text can be generated from the other data so
    //          we don't need to hash it. Maybe need to confirm this with Michael.
    size_t seed = 0;
    boost::hash_combine(seed, manager_);
    boost::hash_combine(seed, move_.RoleIndex);
    boost::hash_combine(seed, move_.Tuple.RelationIndex);
    boost::hash_combine(seed, move_.Tuple.ID);
    return seed;
}

std::size_t hash_value(const Move& move)
{
    return move.hash_value();
}

std::size_t hash_value(const JointMove& jmove) {
    size_t seed = 0;
    for (JointMove::const_iterator iter = jmove.begin(); iter != jmove.end(); iter++) {
        boost::hash_combine(seed, *iter);        
    }
    return seed;
}

std::ostream& operator<<(std::ostream& os, const Move& move)
{
    return move.manager_->PrintMove(os, move.move_);
}


/*****************************************************************************************
 * Implementation of Game
 *****************************************************************************************/
Game::Game() { }

Game::Game(const Game& other) { }

Game::Game(const std::string& gdldescription, bool usegadelac)
{
    initialise(gdldescription, usegadelac);
}

Game::Game(const char* gdldescription, bool usegadelac)
{
    initialise(std::string(gdldescription), usegadelac);
}

Game::Game(const boost::filesystem::path& gdlfile, bool usegadelac)
{
    initialise(gdlfile, usegadelac);
}

void Game::initialise(const std::string& gdldescription, bool usegadelac)
{
    if (usegadelac) throw HSFCException() << ErrorMsgInfo("GaDeLaC is not yet supported");
    hsfcGDLParamaters params;
    initInternals(params);

    manager_.Initialise(gdldescription, params);
    initstate_.reset(new State(*this));
}

void Game::initialise(const boost::filesystem::path& gdlfile, bool usegadelac)
{
    if (usegadelac) throw HSFCException() << ErrorMsgInfo("GaDeLaC is not yet supported");
    hsfcGDLParamaters params;
    initInternals(params);

    manager_.Initialise(gdlfile, params);
    initstate_.reset(new State(*this));
}


void Game::initInternals(hsfcGDLParamaters& params)
{
    // Note: not really sure what are sane options here so copying
    // from Michael's example code.
    params.ReadGDLOnly = false;      // Validate the GDL without creating the schema
    params.SchemaOnly = false;      // Validate the GDL & Schema without grounding the rules
    params.MaxRelationSize = 1000000;  // Max bytes per relation for high speed storage
    params.MaxReferenceSize = 1000000;  // Max bytes per lookup table for grounding
    params.OrderRules = true;      // Optimise the rule execution cost
}

const State& Game::initState() const
{
    return *initstate_;
}


unsigned int Game::numPlayers() const
{
    return manager_.NumPlayers();
}


void Game::players(std::vector<Player>& plyrs) const
{
    this->players(std::back_inserter(plyrs));
}

boost::shared_ptr<std::vector<Player> > Game::players() const
{
    boost::shared_ptr<std::vector<Player> >tmp(new std::vector<Player>());
    players(*tmp);
    return tmp;
}


bool Game::operator==(const Game& other) const
{
    // Note: because I disable the Game copy constructor I check
    // equality by checking the pointer. Maybe this is a bit dodgy.
    return this == &other;
}

bool Game::operator!=(const Game& other) const
{
    return this != &other;
}

/*****************************************************************************************
 * Implementation of State
 *****************************************************************************************/

State::State(Game& game): manager_(&game.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->SetInitialGameState(*state_);
}

State::State(Game& game, const PortableState& ps): manager_(&game.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->SetStateData(ps.relationlist_, ps.round_, ps.currentstep_, *state_);
//  manager_->SetInitialGameState(*state_);
}


State::State(const State& other) : manager_(other.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->CopyGameState(*state_, *(other.state_));
}

State& State::operator=(const State& other)
{
    BOOST_ASSERT_MSG(manager_ == other.manager_, "Cannot assign to States from different games");
    if (state_ != NULL)
        manager_->FreeGameState(state_);
    manager_->CopyGameState(*state_, *(other.state_));
    return *this;
}

State::~State()
{
    if (state_ != NULL)
        manager_->FreeGameState(state_);
}

bool State::isTerminal() const
{
    return manager_->IsTerminal(*state_);
}

void State::legals(std::vector<PlayerMove>& moves) const
{
    this->legals(std::back_inserter(moves));
}

boost::shared_ptr<std::vector<PlayerMove> > State::legals() const
{
    boost::shared_ptr<std::vector<PlayerMove> >tmp(new std::vector<PlayerMove>());
    legals(*tmp);
    return tmp;
}

boost::unordered_map<Player, std::vector<Move> > State::myLegals() const {
    const boost::shared_ptr<std::vector<PlayerMove> > legalMoves(legals());
    boost::unordered_map<Player, std::vector<Move> > result;
  
    for(std::vector<PlayerMove>::iterator pm = legalMoves->begin(); pm != legalMoves->end(); pm++) {
        boost::unordered_map<Player, std::vector<Move> >::iterator iter_moves(result.find(pm->first));
        if(iter_moves == result.end()) {
            result.emplace(pm->first, std::vector<Move>(1, pm->second));
        } else {
            iter_moves->second.push_back(pm->second);
        }
    }
    return result;
}

std::vector<JointMove> State::joints() const {
    const boost::unordered_map<Player, std::vector<Move> > inputs(myLegals());
    std::vector<JointMove> result (1, boost::unordered_map<Player, Move>());

    for (boost::unordered_map<Player, std::vector<Move> >::const_iterator iterInput = inputs.begin(); iterInput != inputs.end(); iterInput++) {
        std::vector<JointMove> new_result;
        for (std::vector<JointMove>::iterator iterResult = result.begin(); iterResult != result.end(); iterResult++) {
            for (std::vector<Move>::const_iterator iterMove = iterInput->second.begin(); iterMove != (*iterInput).second.end(); iterMove++) {
                JointMove tuple (*iterResult);
                tuple.emplace(iterInput->first, *iterMove);
                new_result.push_back(tuple);
            }
        }
        result = new_result;
    }
    return result;
}

void State::goals(std::vector<PlayerGoal>& results) const
{
    this->goals(std::back_inserter(results));
}

boost::shared_ptr<std::vector<PlayerGoal> > State::goals() const
{
    boost::shared_ptr<std::vector<PlayerGoal> >tmp(new std::vector<PlayerGoal>());
    goals(*tmp);
    return tmp;
}

JointGoal State::myGoals() const {
    JointGoal result;
    this->goals(std::inserter(result, result.begin()));
    return result;
}

void State::playout(std::vector<PlayerGoal>& results)
{
    this->playout(std::back_inserter(results));
}

boost::shared_ptr<std::vector<PlayerGoal> > State::playout()
{
    boost::shared_ptr<std::vector<PlayerGoal> >tmp(new std::vector<PlayerGoal>());
    playout(*tmp);
    return tmp;
}


void State::play(const std::vector<PlayerMove>& moves)
{
    this->play(moves.begin(), moves.end());
}

void State::play(const JointMove& moves)
{
    this->play(moves.begin(), moves.end());
}

boost::shared_ptr<PortableState> State::CreatePortableState() const
{
    boost::shared_ptr<PortableState> ps(new PortableState());
    ps->relationlist_.clear();
    manager_->GetStateData(*state_, ps->relationlist_, ps->round_, ps->currentstep_);
    return ps;
}

void State::LoadPortableState(const PortableState& ps)
{
    manager_->SetStateData(ps.relationlist_, ps.round_, ps.currentstep_, *state_);
}



}; /* namespace HSFC */

