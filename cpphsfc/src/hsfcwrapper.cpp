#include <cstdlib>
#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/foreach.hpp>
#include <boost/variant/get.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem/fstream.hpp>

#include <hsfc/impl/hsfcwrapper.h>
#include <hsfc/hsfcexception.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of HSFCManager
 *****************************************************************************************/

HSFCManager::HSFCManager() :
    internal_(new hsfcGDLManager()), params_(new hsfcGDLParameters())
{  }

/*****************************************************************************************
 * Internal extra functions.
 * Note: must only be called after Initialise()
 *****************************************************************************************/

void HSFCManager::PopulatePlayerNamesFromLegalMoves()
{
    hsfcState* tmpstate;
    try
    {
        tmpstate = this->CreateGameState();
        this->SetInitialGameState(*tmpstate);
        playernames_.assign(this->NumPlayers(), std::string());

        std::vector<hsfcLegalMove> legalmoves;
        this->GetLegalMoves(*tmpstate, legalmoves);
        if (legalmoves.empty())
        {
            throw HSFCInternalError()
                << ErrorMsgInfo("HSFC internal error: initial state contains no legal moves");
        }
        BOOST_FOREACH(hsfcLegalMove& lm, legalmoves)
        {
            Term term;
            internal_->GetMoveText(lm);
            parse_sexpr(lm.Text,term);
            delete[] lm.Text;

            if (SubTerms* ts = boost::get<SubTerms>(&term))
            {
                if (ts->children_.size() != 3)
                    throw HSFCInternalError()
                        << ErrorMsgInfo("HSFC internal error: move_.Text term not arity 3");
                if (boost::get<std::string>(ts->children_[0]) != "does")
                    throw HSFCInternalError()
                        << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
                playernames_[lm.RoleIndex] = boost::get<std::string>(ts->children_[1]);
            }
            else
            {
                throw HSFCInternalError()
                    << ErrorMsgInfo("HSFC internal error: move_.Text term is not a 'does' relation");
            }
        }
        BOOST_FOREACH(const std::string& s, playernames_)
        {
            if (s.empty())
                throw HSFCInternalError()
                    << ErrorMsgInfo("HSFC internal error: Failed to find names for roles");
        }
    } catch (HSFCException& e)
    {
        this->FreeGameState(tmpstate);
        throw HSFCInternalError() << ErrorMsgInfo(e.what());
    } catch (...)
    {
        this->FreeGameState(tmpstate);
        throw;
    }

}

/*****************************************************************************************
 * Functions that match the hsfcGDLManager
 *****************************************************************************************/

hsfcState* HSFCManager::CreateGameState()
{
    hsfcState* state;

    if (!internal_->CreateGameState(&state))
    {
        throw HSFCInternalError() << ErrorMsgInfo("Failed to create HSFC game state");
    }
    return state;
}

void HSFCManager::FreeGameState(hsfcState* GameState)
{
    internal_->FreeGameState(GameState);
}

void HSFCManager::CopyGameState(hsfcState& Destination, const hsfcState& Source)
{
    hsfcState& tmp = const_cast<hsfcState&>(Source);
    internal_->CopyGameState(&Destination, &tmp);
}

void HSFCManager::SetInitialGameState(hsfcState& GameState)
{
    internal_->SetInitialGameState(&GameState);
}

void HSFCManager::GetLegalMoves(const hsfcState& GameState,
                                std::vector<hsfcLegalMove>& LegalMove) const
{
    std::vector<std::vector<hsfcLegalMove> > tmp;
    internal_->GetLegalMoves(const_cast<hsfcState*>(&GameState), tmp);

    BOOST_FOREACH(std::vector<hsfcLegalMove>& v, tmp)
    {
        BOOST_FOREACH(hsfcLegalMove& lm, v)
        {
            LegalMove.push_back(lm);
        }
    }
}

void HSFCManager::DoMove(hsfcState& GameState, const std::vector<hsfcLegalMove>& LegalMove)
{
    internal_->DoMove(&GameState, const_cast<std::vector<hsfcLegalMove>&>(LegalMove));
}

bool HSFCManager::IsTerminal(const hsfcState& GameState) const
{
    return internal_->IsTerminal(const_cast<hsfcState*>(&GameState));
}

void HSFCManager::GetGoalValues(const hsfcState& GameState,
                                std::vector<int>& GoalValue) const
{
    internal_->GetGoalValues(const_cast<hsfcState*>(&GameState), GoalValue);
}

void HSFCManager::PlayOut(hsfcState& GameState, std::vector<int>& GoalValue)
{
    internal_->PlayOut(&GameState, GoalValue);
}

void HSFCManager::DisplayState(const hsfcState& GameState, bool rigids) const
{
    internal_->PrintState(const_cast<hsfcState*>(&GameState), rigids);
}

std::ostream& HSFCManager::PrintState(std::ostream& os, const hsfcState& GameState) const
{
    throw HSFCException()
        << ErrorMsgInfo("HSFCManager::PrintState() not supported for HSFC2");
}


/*****************************************************************************************
 * Initialise function behaviour has been modified. Added a version to load a GDL
 * description from a string. Because the underlying libhsfc doesn't provide a function
 * to do this I hack it by writing to a temporary file.
 *****************************************************************************************/

void HSFCManager::Initialise(const boost::filesystem::path& gdlfile,
                             const hsfcGDLParameters& parameters)
{
    namespace bfs=boost::filesystem;
    // Check that the file exists
    if (!bfs::is_regular_file(gdlfile))
    {
        std::ostringstream ss;
        ss << "File does not exist: " << gdlfile.string();
        throw HSFCValueError() << ErrorMsgInfo(ss.str());
    }

    bfs::ifstream ts(gdlfile);
    std::string gdlstr;
    ts.seekg(0, std::ios::end);
    gdlstr.reserve(ts.tellg());
    ts.seekg(0, std::ios::beg);
    gdlstr.assign((std::istreambuf_iterator<char>(ts)),
                  std::istreambuf_iterator<char>());
    ts.close();
    Initialise(gdlstr, parameters);
}

void HSFCManager::Initialise(const std::string& gdldescription,
                             const hsfcGDLParameters& parameters)
{
    std::string tmpgdl = gdl_keywords_to_lowercase(gdldescription);
    params_.reset(new hsfcGDLParameters(parameters));
    if (!internal_->Initialise(&tmpgdl, params_.get()))
    {
        std::ostringstream ss;
        ss << "Failed to initialise the HSFC engine.";
        throw HSFCInternalError() << ErrorMsgInfo(ss.str());
    }
    PopulatePlayerNamesFromLegalMoves();
}




/*****************************************************************************************
 * Extra functions
 *****************************************************************************************/

unsigned int HSFCManager::NumPlayers() const
{
    return (unsigned int) internal_->NumRoles;
}

std::ostream& HSFCManager::PrintPlayer(std::ostream& os, unsigned int roleid) const
{
    if (roleid >= this->NumPlayers())
        throw HSFCInternalError() << ErrorMsgInfo("HSFC internal error: not a valid roleid");
    return os << playernames_[roleid];
}

/*****************************************************************************************
 * To extract the Move text
 *****************************************************************************************/


struct gdl_move_visitor : public boost::static_visitor<std::ostream&>
{
    std::ostream& os_;
    gdl_move_visitor(std::ostream& os) : os_(os){}
    std::ostream& operator()(const std::string& name) { return os_ << name; }
    std::ostream& operator()(const SubTerms& ts){ return generate_sexpr(ts, os_); }
};

std::ostream& HSFCManager::PrintMove(std::ostream& os, const hsfcLegalMove& legalmove) const
{
    try
    {
        Term term;
        internal_->GetMoveText(const_cast<hsfcLegalMove&>(legalmove));
        parse_sexpr(legalmove.Text,term);

        if (SubTerms* ts = boost::get<SubTerms>(&term))
        {
            if (ts->children_.size() != 3)
                throw HSFCInternalError()
                    << ErrorMsgInfo("HSFC internal error: move_.Text term not arity 3");
            if (boost::get<std::string>(ts->children_[0]) != "does")
                throw HSFCInternalError()
                    << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
            gdl_move_visitor gmv(os);
            boost::apply_visitor(gmv, ts->children_[2]);
            return os;
        }
        else
        {
            throw HSFCInternalError()
                << ErrorMsgInfo("HSFC internal error: move_.Text term is not a 'does' relation");
        }
    } catch (HSFCException& e)
    {
        throw HSFCInternalError() << ErrorMsgInfo(e.what());
    }
}


void HSFCManager::GetStateData(const hsfcState& state,
                               std::set<std::pair<int,int> >& relationset,
                               int& round, int& currentstep) const
{
    hsfcStateManager* sm = const_cast<hsfcStateManager*>(internal_->StateManager);
    hsfcState& ts = const_cast<hsfcState&>(state);
    round = ts.Round;
    currentstep = ts.CurrentStep;

//    sm->PrintRelations(&ts, true);

    for (int i=1; i < sm->NumRelationLists; ++i)
    {
        hsfcRelationSchema* tmp = sm->Schema->RelationSchema[i];
//        std::cerr << " RER: " << tmp << std::endl;
        if (sm->Schema->RelationSchema[i]->Rigidity != hsfcRigidityFull)
        {
            for (int j=0; j < ts.NumRelations[i]; ++j)
            {
                relationset.insert(std::make_pair(i, ts.RelationID[i][j]));
            }
        }
    }

}

void HSFCManager::SetStateData(const std::set<std::pair<int,int> >& relationset,
                               int round, int currentstep,
                               hsfcState& state)
{
    hsfcStateManager* sm = internal_->StateManager;
    sm->ResetState(&state);
    state.Round = round;
    state.CurrentStep = currentstep;
    typedef std::pair<int,int> ipair_t;
    BOOST_FOREACH(const ipair_t& pr, relationset)
    {
        hsfcTuple tuple;
        tuple.Index = pr.first;
        tuple.ID = pr.second;
        sm->AddRelation(&state, tuple);
    }

}

void HSFCManager::PrintFluent(const hsfcTuple& fluent, std::string& text) const {
  hsfcTuple& tmp = const_cast<hsfcTuple&>(fluent);
  internal_->GetMoveText(tmp, text);
}

void HSFCManager::GetFluents(const hsfcState& state, std::vector<hsfcTuple>& fluents) const {
  hsfcState& tmp = const_cast<hsfcState&>(state);
  internal_->GetStateFluents(&tmp, fluents);
}

}; /* namespace HSFC */
