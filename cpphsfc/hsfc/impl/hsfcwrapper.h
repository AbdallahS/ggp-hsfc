/*****************************************************************************************
 * This is a thin wrapper around the HSFC:
 * 1) wrapper class is in the HSFC namespace.
 * 2) is const correct
 * 3) adds a few extra useful functions
 *****************************************************************************************/

#ifndef HSFC_WRAPPER_H
#define HSFC_WRAPPER_H

#include <string>
#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <hsfc/impl/hsfcEngine.h>

// Typedefs added to reduce porting effort from previous libhsfc
typedef hsfcParameters hsfcGDLParameters;
typedef hsfcEngine hsfcGDLManager;

namespace HSFC
{


class HSFCManager
{


private:
    boost::scoped_ptr<hsfcGDLManager> internal_;

    // Because the hsfcGDLManager doesn't provide an easy way to get
    // the role names we need to jump through some hoops. Running
    // PopulatePlayerNamesFromLegalMoves() is also important because
    // the internals are not properly initialised until a state
    // has been created.
    std::vector<std::string> playernames_;
    void PopulatePlayerNamesFromLegalMoves();

    boost::scoped_ptr<hsfcGDLParameters> params_;

public:
    HSFCManager();

    /* Functions from hsfcGDLManager with const fixes */
    hsfcState* CreateGameState();
    void FreeGameState(hsfcState* GameState);
    void CopyGameState(hsfcState& Destination, const hsfcState& Source);
    void SetInitialGameState(hsfcState& GameState);
    void GetLegalMoves(const hsfcState& GameState,
                       std::vector<hsfcLegalMove>& LegalMove) const;
    void DoMove(hsfcState& GameState, const std::vector<hsfcLegalMove>& LegalMove);
    bool IsTerminal(const hsfcState& GameState) const;
    void GetGoalValues(const hsfcState& GameState, std::vector<int>& GoalValue) const;
    void PlayOut(hsfcState& GameState, std::vector<int>& GoalValue);

    /* Additional functions - note: capitalised first letters for class consistency. */
    unsigned int NumPlayers() const;
    std::ostream& PrintPlayer(std::ostream& os, unsigned int roleid) const;
    std::ostream& PrintMove(std::ostream& os, const hsfcLegalMove& legalmove) const;

    // These are debugging functions. Don't use them except for debugging code.
    std::ostream& PrintState(std::ostream& os, const hsfcState& GameState) const;
    void DisplayState(const hsfcState& GameState, bool rigids) const;

    // NOTE: the change of behaviour of the Initialise function compared to the underlying
    // HSFC. Passing a string is now assumed to be a GDL description. To pass a filename
    // use boost::filesystem::path.
    void Initialise(const std::string& gdldescription,
                    const hsfcGDLParameters& Parameters);
    void Initialise(const boost::filesystem::path& gdlfilename,
                    const hsfcGDLParameters& Parameters);


    /*******************************************************************************
     * A semi-portable representation of a game state. Should be portable across any
     * HSFC instance that is loaded with the exact same GDL file.
     *******************************************************************************/

    void GetStateData(const hsfcState& state, std::set<std::pair<int,int> >& relationset,
                      int& round, int& currentstep) const;

    void SetStateData(const std::set<std::pair<int,int> >& relationset, int round,
                      int currentstep, hsfcState& state);

    void PrintFluent(const hsfcTuple& fluent, std::string& text) const;
    void GetFluents(const hsfcState& state, std::vector<hsfcTuple>& fluents) const;
};

};

#endif /* HSFC_WRAPPER_H */
