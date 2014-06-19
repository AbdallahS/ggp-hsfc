#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/variant/get.hpp>
#include <boost/functional/hash.hpp>
#include <boost/make_shared.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of HSFCException
 *****************************************************************************************/

char const* HSFCException::what() const throw ()
{
    if (const std::string* mi=boost::get_error_info<ErrorMsgInfo>(*this)) return mi->c_str();
    return "Unknown HSFCException";        
}


/*****************************************************************************************
 * Implementation of Player
 *****************************************************************************************/
Player::Player(boost::shared_ptr<const HSFCManager> manager, unsigned int roleid): 
    manager_(manager), roleid_(roleid)
{ }

Player::Player(const Player& other): manager_(other.manager_), roleid_(other.roleid_)
{ }

Player::Player(Game& game, const PortablePlayer& pp) : 
    manager_(game.manager_), roleid_(pp.roleid_)
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
    boost::hash_combine(seed, manager_.get());
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


/*********************************************************************************
 * Implementation of Move
 *********************************************************************************/

Move::Move(boost::shared_ptr<HSFCManager> manager, const hsfcLegalMove& move): 
    manager_(manager), move_(move)
{ }

Move::Move(const Move& other): manager_(other.manager_), move_(other.move_)
{ }

Move::Move(Game& game, const PortableMove& pm) : manager_(game.manager_)
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
    boost::hash_combine(seed, manager_.get());
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

std::ostream& operator<<(std::ostream& os, const JointMove& jmove)
{
    for(JointMove::const_iterator iter = jmove.begin(); iter != jmove.end(); iter++) {
        os << "(does " << iter->first << " " << iter->second << ") ";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const JointGoal& jgoal)
{
    for(JointGoal::const_iterator iter = jgoal.begin(); iter != jgoal.end(); iter++) {
        os << "(goal " << iter->first << " " << iter->second << ") ";
    }
    return os;
}

/*****************************************************************************************
 * Implementation of Game
 *****************************************************************************************/
Game::Game() 
{ 
    manager_ = boost::make_shared<HSFCManager>();
}

Game::Game(const Game& other) 
{ 
    manager_ = boost::make_shared<HSFCManager>();
    throw HSFCInternalError() 
        << ErrorMsgInfo("Internal error: Illegal use of Game::Game() copy constructor");
}

Game::Game(const std::string& gdldescription, bool usegadelac)
{
    manager_ = boost::make_shared<HSFCManager>();
    initialise(gdldescription, usegadelac);
}

Game::Game(const char* gdldescription, bool usegadelac)
{
    manager_ = boost::make_shared<HSFCManager>();
    initialise(std::string(gdldescription), usegadelac);
}

Game::Game(const boost::filesystem::path& gdlfile, bool usegadelac)
{
    manager_ = boost::make_shared<HSFCManager>();
    initialise(gdlfile, usegadelac);
}

void Game::initialise(const std::string& gdldescription, bool usegadelac)
{
    hsfcGDLParamaters params;
    initInternals(params);

    manager_->Initialise(gdldescription, params, usegadelac);
    initstate_.reset(new State(*this));
}

void Game::initialise(const boost::filesystem::path& gdlfile, bool usegadelac)
{
    hsfcGDLParamaters params;
    initInternals(params);

    manager_->Initialise(gdlfile, params, usegadelac);
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
    return manager_->NumPlayers();
}

std::vector<Player> Game::players() const {
    std::vector<Player> tmp;
    this->players(std::back_inserter(tmp));
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
 * NOTE: 20140612. From what I can tell from looking at the HSFC code a state isn't
 * in some sense valid until it has had the legal moves calculated from it. Or at least
 * you cannot run Play() from a state that has not had legal moves calculated. So as
 * a hack I will calculate (and throw away) the legal moves whenever I create a state and
 * whenever I run Play().
 *****************************************************************************************/
void State::initialize(){
    // Hack to calculate legal moves so that the state will now be in a good state.
    if (!isTerminal()) {
        std::vector<PlayerMove> legalMoves;
        legals(std::back_inserter(legalMoves));
    } else {
        goals();
    }
}

State::State(Game& game): manager_(game.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->SetInitialGameState(*state_);
    this->initialize();
}

State::State(Game& game, const PortableState& ps): manager_(game.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->SetStateData(ps.relationlist_, ps.round_, ps.currentstep_, *state_);
//  manager_->SetInitialGameState(*state_);
    this->initialize();
}


State::State(const State& other) : manager_(other.manager_), state_(NULL)
{
    state_ = manager_->CreateGameState();
    manager_->CopyGameState(*state_, *(other.state_));
    this->initialize();
}

State& State::operator=(const State& other)
{
    if (manager_ != other.manager_) 
        throw HSFCValueError() << ErrorMsgInfo("Cannot assign to a State from a different game");
    manager_->CopyGameState(*state_, *(other.state_));
    this->initialize();

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

boost::unordered_map<Player, std::vector<Move> > State::legals() const {
    std::vector<PlayerMove> legalMoves;
    legals(std::back_inserter(legalMoves));

    boost::unordered_map<Player, std::vector<Move> > result;
  
    for(std::vector<PlayerMove>::iterator pm = legalMoves.begin(); pm != legalMoves.end(); pm++) {
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
    const boost::unordered_map<Player, std::vector<Move> > inputs(legals());

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

JointGoal State::goals() const {
    JointGoal result;
    this->goals(std::inserter(result, result.begin()));
    return result;
}

void State::playout(std::vector<PlayerGoal>& results)
{
    this->playout(std::back_inserter(results));
}

JointGoal State::playout() {
    JointGoal result;
    this->playout(std::inserter(result, result.begin()));
    return result;
}


void State::play(const std::vector<PlayerMove>& moves)
{
    this->play(moves.begin(), moves.end());
}

void State::play(const JointMove& moves)
{
    this->play(moves.begin(), moves.end());
}

}; /* namespace HSFC */

