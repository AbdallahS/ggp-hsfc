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

HSFCManager::HSFCManager() : internal_(new hsfcGDLManager())
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
        BOOST_FOREACH(hsfcLegalMove& lm, legalmoves)
        {
            Term term;
            parse_flat(lm.Text, term);
            if (term.children_.size() != 3)
                throw HSFCInternalError() 
                    << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
            if (boost::get<std::string>(term.children_[0]) != "does")
                throw HSFCInternalError() 
                    << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
            playernames_[lm.RoleIndex] = boost::get<std::string>(term.children_[1]);
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
 * Internal function
 * Preprocess using gadelac
 *
 * BUG FIX: The return value of std::system() seems to be OS specific (maybe 
 * compiler specific as well?). For example: 
 * http://www.cplusplus.com/reference/cstdlib/system/ suggested that the
 * the return value on successful completion would be the return code of gadelac, 
 * while conceding that this may be system specific. This is not the case for 
 * linux and from what I can tell POSIX in general. Instead you have to use the 
 * macro WEXITSTATUS() to extract the external programs return code from the 
 * std::system() return value. See: http://linux.die.net/man/3/system
 *
 * NOTE: This is exactly why I think this sort of thing is bad, you 
 * get behaviours that don't match what you expect due to small OS quirks. 
 * Gadelac should be a library and not an externally called executable.
 *
 *****************************************************************************************/

void HSFCManager::RunGadelac(const boost::filesystem::path& infile, 
                             const boost::filesystem::path& outfile,
                             const std::string& extra_options)
{            
    std::ostringstream ss;
    ss << "gadelac " << extra_options << " --backend gdl -o " 
       << outfile.native() << " " << infile.native();
    int result = std::system(ss.str().c_str());
    if (result == 0) return;
    if (result == -1) throw HSFCInternalError() << ErrorMsgInfo("Could not find executable: gadelac");
    int code = WEXITSTATUS(result);
    if (code == 1) throw HSFCValueError() << ErrorMsgInfo("Gadelac indentified invalid GDL");
    std::ostringstream sserr;
    sserr << "Gadelac failed to process GDL (code: " << code << "): " << ss.str();
    throw HSFCInternalError() << ErrorMsgInfo(sserr.str());
}

/*****************************************************************************************
 * Functions that match the hsfcGDLManager
 *****************************************************************************************/

hsfcState* HSFCManager::CreateGameState()
{
    hsfcState* state;  
    if (internal_->CreateGameState(&state))
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
    internal_->GetLegalMoves(const_cast<hsfcState*>(&GameState), LegalMove);
}

void HSFCManager::DoMove(hsfcState& GameState, const std::vector<hsfcLegalMove>& LegalMove)
{
    internal_->DoMove(&GameState, const_cast<std::vector<hsfcLegalMove>&>(LegalMove));
}

bool HSFCManager::IsTerminal(const hsfcState& GameState) const
{
    internal_->IsTerminal(const_cast<hsfcState*>(&GameState));
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

void HSFCManager::PrintState(const hsfcState& GameState) const
{
    internal_->PrintState(const_cast<hsfcState*>(&GameState));
}


/*****************************************************************************************
 * Initialise function behaviour has been modified. Added a version to load a GDL 
 * description from a string. Because the underlying libhsfc doesn't provide a function
 * to do this I hack it by writing to a temporary file.
 *****************************************************************************************/

void HSFCManager::Initialise(const boost::filesystem::path& gdlfile,
                             const hsfcGDLParamaters& Parameters,
                             bool usegadelac)
{
    namespace bfs=boost::filesystem;
    bfs::path gadfile;
    bfs::path infile;

    try
    {
        // Check that the file exists
        if (!bfs::is_regular_file(gdlfile))
        {
            std::ostringstream ss;
            ss << "File does not exist: " << gdlfile.native();        
            throw HSFCValueError() << ErrorMsgInfo(ss.str());
        }

        // If we need to use gadelac then use a temporary output file
        if (usegadelac)
        {
            gadfile = bfs::unique_path(); 
            RunGadelac(gdlfile, gadfile);            
            infile = gadfile;
        } 
        else
        {
            infile = gdlfile;
        }

        // needed to avoid non-const in hsfcGDLManager::Initialise
        std::string tmpstr(infile.native()); 
        hsfcGDLParamaters params(Parameters);

        int result = internal_->InitialiseFromFile(&tmpstr, params);
        if (result != 0)
        {
            std::ostringstream ss;
            ss << "Failed to initialise the HSFC engine. Error code: " << result;
            throw HSFCInternalError() << ErrorMsgInfo(ss.str());
        }  

        // Now jump through hoops to workout the names of the players.
        PopulatePlayerNamesFromLegalMoves();
        
        if (usegadelac) bfs::remove(gadfile);
    } catch (...)
    {
        if (usegadelac && bfs::is_regular_file(gadfile)) bfs::remove(gadfile);    
        throw;
    }
}

void HSFCManager::Initialise(const std::string& gdldescription, 
                             const hsfcGDLParamaters& Parameters,
                             bool usegadelac)
{
    namespace bfs=boost::filesystem;
    bfs::path tmppath = bfs::unique_path();
    try {        
        bfs::ofstream gdlfile(tmppath);
        if (gdlfile.fail()) throw HSFCInternalError() 
                                << ErrorMsgInfo("Failed to create temporary file");
        
        gdlfile << gdldescription;
        gdlfile.close();
        Initialise(tmppath, Parameters, usegadelac);
        bfs::remove(tmppath);    
    } catch (...)
    {
        if (bfs::is_regular_file(tmppath)) bfs::remove(tmppath);    
        throw;
    }
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
    std::ostream& operator()(const Term& t){ return generate_sexpr(t,os_); }
};

std::ostream& HSFCManager::PrintMove(std::ostream& os, const hsfcLegalMove& legalmove) const
{
    try
    {
        Term term;
        parse_flat(legalmove.Text, term);
        if (term.children_.size() != 3)
            throw HSFCInternalError() 
                << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
        if (boost::get<std::string>(term.children_[0]) != "does")
            throw HSFCInternalError() 
                << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
        gdl_move_visitor gmv(os);
        boost::apply_visitor(gmv, term.children_[2]);
        return os;
    } catch (HSFCException& e)
    {
        throw HSFCInternalError() << ErrorMsgInfo(e.what());
    }
}

void HSFCManager::GetStateData(const hsfcState& state, 
                               std::vector<std::pair<int,int> >& relationlist,
                               int& round, int& currentstep) const
{  
    hsfcStateManager* sm = const_cast<hsfcStateManager*>(internal_->StateManager);  
    hsfcState& ts = const_cast<hsfcState&>(state);
    round = ts.Round;
    currentstep = ts.CurrentStep;
    for (int i=0; i < sm->NumRelationLists; ++i)
    {
        if (sm->Schema->Relation[i]->Fact != hsfcFactPermanent)
        {
            for (int j=0; j < ts.NumRelations[i]; ++j)
            {
                relationlist.push_back(std::make_pair(i, ts.RelationID[i][j]));
            }
        }
    }  
}

void HSFCManager::SetStateData(const std::vector<std::pair<int,int> >& relationlist, 
                               int round, int currentstep,
                               hsfcState& state) 
{  
    hsfcStateManager* sm = internal_->StateManager;  
    sm->ResetState(&state);
    state.Round = round;
    state.CurrentStep = currentstep;
    typedef std::pair<int,int> ipair_t;
    BOOST_FOREACH(const ipair_t& pr, relationlist)
    {
        hsfcTuple tuple;
        tuple.RelationIndex = pr.first;
        tuple.ID = pr.second;
        sm->AddRelation(&state, &tuple);
    }
}


}; /* namespace HSFC */
