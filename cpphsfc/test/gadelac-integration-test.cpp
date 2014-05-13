//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCGadelacIntegrationTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/function_output_iterator.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

using namespace HSFC;

/****************************************************************
 * General support functions
 ****************************************************************/

// Count the number of moves that are legal for the given player
unsigned int count_player_moves(const std::vector<PlayerMove> moves, const std::string& player)
{
    unsigned int count = 0;
    BOOST_FOREACH(const PlayerMove& pm, moves)
    {
        if (pm.first.tostring() == player) ++count;
    }
    return count;
}

// Pick the first available move for the given player
PlayerMove pick_first(const std::vector<PlayerMove> moves, const std::string& player)
{
    BOOST_FOREACH(const PlayerMove& pm, moves)
    {
        if (pm.first.tostring() == player) return pm;
    }
    BOOST_CHECK(false);
}

// Return the player 
Player get_player(const Game& game, const std::string& playername)
{
    std::vector<Player> players = game.players();
    BOOST_FOREACH(const Player& p, players)
    {
        if (p.tostring() == playername) return p;
    }
    BOOST_CHECK(false);
}

/****************************************************************
 * Tictactoe specific functions.
 ****************************************************************/

// Run a playout from any (non-terminal) tictactoe game state. 
// Someone either wins or it is a draw.
void tictactoe_playout_check(const State &state)
{
    if (state.isTerminal()) return;
    State tmpstate(state);
    std::vector<PlayerGoal> results;
    tmpstate.playout(results);
    bool haswinner = 
        (results[0].second == 100 && results[1].second == 0) || 
        (results[1].second == 100 && results[0].second == 0);
    if (!haswinner)
    {
        BOOST_CHECK(results[0].second == 50);
        BOOST_CHECK(results[1].second == 50);
    }   
}


/****************************************************************
 * Test of loading GDL from a string
 * A Gadelac produced string
 ****************************************************************/
const char* g_gdl1 = " \n\
;;;; RULES   \n\
 \n\
(role xplayer) \n\
(role oplayer) \n\
(init (cell 1 1 b)) \n\
(init (cell 1 2 b)) \n\
(init (cell 1 3 b)) \n\
(init (cell 2 1 b)) \n\
(init (cell 2 2 b)) \n\
(init (cell 2 3 b)) \n\
(init (cell 3 1 b)) \n\
(init (cell 3 2 b)) \n\
(init (cell 3 3 b)) \n\
(init (control xplayer)) \n\
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k)) \n\
(<= (next (control xplayer)) (true (control oplayer))) \n\
(<= (next (control oplayer)) (true (control xplayer))) \n\
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x))) \n\
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x))) \n\
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x))) \n\
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x))) \n\
(<= (line ?x) (row ?m ?x)) \n\
(<= (line ?x) (column ?m ?x)) \n\
(<= (line ?x) (diagonal ?x)) \n\
(<= open (true (cell ?m ?n b))) \n\
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w))) \n\
(<= (legal xplayer noop) (true (control oplayer))) \n\
(<= (legal oplayer noop) (true (control xplayer))) \n\
(<= (goal xplayer 100) (line x)) \n\
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal xplayer 0) (line o)) \n\
(<= (goal oplayer 100) (line o)) \n\
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal oplayer 0) (line x)) \n\
(<= terminal (line x)) \n\
(<= terminal (line o)) \n\
(<= terminal (not open)) \n\
 \n\
;;;; STRATS  \n\
 \n\
(strat does 0) \n\
(strat goal 1) \n\
(strat init 0) \n\
(strat legal 0) \n\
(strat next 0) \n\
(strat role 0) \n\
(strat terminal 1) \n\
(strat true 0) \n\
(strat cell 0) \n\
(strat column 0) \n\
(strat control 0) \n\
(strat diagonal 0) \n\
(strat line 0) \n\
(strat open 0) \n\
(strat row 0) \n\
 \n\
;;;; PATHS   \n\
 \n\
(arg does 0 oplayer) \n\
(arg does 0 xplayer) \n\
(arg does 1 mark 0 1) \n\
(arg does 1 mark 0 2) \n\
(arg does 1 mark 0 3) \n\
(arg does 1 mark 1 1) \n\
(arg does 1 mark 1 2) \n\
(arg does 1 mark 1 3) \n\
(arg does 1 noop) \n\
(arg goal 0 oplayer) \n\
(arg goal 0 xplayer) \n\
(arg goal 1 0) \n\
(arg goal 1 100) \n\
(arg goal 1 50) \n\
(arg init 0 cell 0 1) \n\
(arg init 0 cell 0 2) \n\
(arg init 0 cell 0 3) \n\
(arg init 0 cell 1 1) \n\
(arg init 0 cell 1 2) \n\
(arg init 0 cell 1 3) \n\
(arg init 0 cell 2 b) \n\
(arg init 0 control 0 xplayer) \n\
(arg legal 0 oplayer) \n\
(arg legal 0 xplayer) \n\
(arg legal 1 mark 0 1) \n\
(arg legal 1 mark 0 2) \n\
(arg legal 1 mark 0 3) \n\
(arg legal 1 mark 1 1) \n\
(arg legal 1 mark 1 2) \n\
(arg legal 1 mark 1 3) \n\
(arg legal 1 noop) \n\
(arg next 0 cell 0 1) \n\
(arg next 0 cell 0 2) \n\
(arg next 0 cell 0 3) \n\
(arg next 0 cell 1 1) \n\
(arg next 0 cell 1 2) \n\
(arg next 0 cell 1 3) \n\
(arg next 0 cell 2 b) \n\
(arg next 0 cell 2 o) \n\
(arg next 0 cell 2 x) \n\
(arg next 0 control 0 oplayer) \n\
(arg next 0 control 0 xplayer) \n\
(arg role 0 oplayer) \n\
(arg role 0 xplayer) \n\
(arg terminal ) \n\
(arg true 0 cell 0 1) \n\
(arg true 0 cell 0 2) \n\
(arg true 0 cell 0 3) \n\
(arg true 0 cell 1 1) \n\
(arg true 0 cell 1 2) \n\
(arg true 0 cell 1 3) \n\
(arg true 0 cell 2 b) \n\
(arg true 0 cell 2 o) \n\
(arg true 0 cell 2 x) \n\
(arg true 0 control 0 oplayer) \n\
(arg true 0 control 0 xplayer) \n\
(arg column 0 1) \n\
(arg column 0 2) \n\
(arg column 0 3) \n\
(arg column 1 b) \n\
(arg column 1 o) \n\
(arg column 1 x) \n\
(arg diagonal 0 b) \n\
(arg diagonal 0 o) \n\
(arg diagonal 0 x) \n\
(arg line 0 b) \n\
(arg line 0 o) \n\
(arg line 0 x) \n\
(arg open ) \n\
(arg row 0 1) \n\
(arg row 0 2) \n\
(arg row 0 3) \n\
(arg row 1 b) \n\
(arg row 1 o) \n\
(arg row 1 x) \n\
 \n\
;;;; DOMAINS \n\
 \n\
(domain_p does (set oplayer xplayer) (set mark noop)) \n\
(domain_p goal (set oplayer xplayer) (set 0 100 50)) \n\
(domain_p init (set cell control)) \n\
(domain_p legal (set oplayer xplayer) (set mark noop)) \n\
(domain_p next (set cell control)) \n\
(domain_p role (set oplayer xplayer)) \n\
(domain_p terminal ) \n\
(domain_p true (set cell control)) \n\
(domain_p column (set 1 2 3) (set b o x)) \n\
(domain_p diagonal (set b o x)) \n\
(domain_p line (set b o x)) \n\
(domain_p open ) \n\
(domain_p row (set 1 2 3) (set b o x)) \n\
 \n\
(domain_s 0 ) \n\
(domain_s 1 ) \n\
(domain_s 100 ) \n\
(domain_s 2 ) \n\
(domain_s 3 ) \n\
(domain_s 50 ) \n\
(domain_s b ) \n\
(domain_s cell (set 1 2 3) (set 1 2 3) (set b o x)) \n\
(domain_s control (set oplayer xplayer)) \n\
(domain_s mark (set 1 2 3) (set 1 2 3)) \n\
(domain_s noop ) \n\
(domain_s o ) \n\
(domain_s oplayer ) \n\
(domain_s x ) \n\
(domain_s xplayer ) \n\
";

/****************************************************************
 * A Non-gadelac gdl
 ****************************************************************/

const char* g_gdl2 = " \n\
(role xplayer) \n\
(role oplayer) \n\
(init (cell 1 1 b)) \n\
(init (cell 1 2 b)) \n\
(init (cell 1 3 b)) \n\
(init (cell 2 1 b)) \n\
(init (cell 2 2 b)) \n\
(init (cell 2 3 b)) \n\
(init (cell 3 1 b)) \n\
(init (cell 3 2 b)) \n\
(init (cell 3 3 b)) \n\
(init (control xplayer)) \n\
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k)) \n\
(<= (next (control xplayer)) (true (control oplayer))) \n\
(<= (next (control oplayer)) (true (control xplayer))) \n\
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x))) \n\
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x))) \n\
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x))) \n\
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x))) \n\
(<= (line ?x) (row ?m ?x)) \n\
(<= (line ?x) (column ?m ?x)) \n\
(<= (line ?x) (diagonal ?x)) \n\
(<= open (true (cell ?m ?n b))) \n\
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w))) \n\
(<= (legal xplayer noop) (true (control oplayer))) \n\
(<= (legal oplayer noop) (true (control xplayer))) \n\
(<= (goal xplayer 100) (line x)) \n\
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal xplayer 0) (line o)) \n\
(<= (goal oplayer 100) (line o)) \n\
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal oplayer 0) (line x)) \n\
(<= terminal (line x)) \n\
(<= terminal (line o)) \n\
(<= terminal (not open)) \n\
";


// Load a unordered_set of playernames (using tostring())
struct playername_loader
{
    playername_loader(boost::unordered_set<std::string>& m) : m_(m){};
    void operator()(const Player& o) { m_.insert(o.tostring()); }
    boost::unordered_set<std::string>& m_;
};

// Load a unordered_set of move texts (using tostring())
struct playermovename_loader
{
    playermovename_loader(boost::unordered_set<std::string>& m) : m_(m){};
    void operator()(const PlayerMove& o) const
    { 
        m_.insert(o.first.tostring() + o.second.tostring()); 
    }
    boost::unordered_set<std::string>& m_;
};


/****************************************************************
 * Simple test that it can run gadelac
 * Note: assumes gadelac is in the path!!!
 ****************************************************************/

BOOST_AUTO_TEST_CASE(game_construction)
{
    Game game1(g_gdl1);  
    std::cout << "Non-gadelac loaded ok" << std::endl;
    Game game2(g_gdl2, true);  
    std::cout << "Gadelac loaded ok" << std::endl;
}


/****************************************************************
 * Test the Game constructor
 ****************************************************************/



