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
    throw std::string("To prevent clang compiler warning");
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
    throw std::string("To prevent clang compiler warning");
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
const char* g_gdl1 = " \n\n\
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
(arg does/2 0 oplayer/0) \n\
(arg does/2 0 xplayer/0) \n\
(arg does/2 1 mark/2 0 1/0) \n\
(arg does/2 1 mark/2 0 2/0) \n\
(arg does/2 1 mark/2 0 3/0) \n\
(arg does/2 1 mark/2 1 1/0) \n\
(arg does/2 1 mark/2 1 2/0) \n\
(arg does/2 1 mark/2 1 3/0) \n\
(arg does/2 1 noop/0) \n\
(arg goal/2 0 oplayer/0) \n\
(arg goal/2 0 xplayer/0) \n\
(arg goal/2 1 0/0) \n\
(arg goal/2 1 100/0) \n\
(arg goal/2 1 50/0) \n\
(arg init/1 0 cell/3 0 1/0) \n\
(arg init/1 0 cell/3 0 2/0) \n\
(arg init/1 0 cell/3 0 3/0) \n\
(arg init/1 0 cell/3 1 1/0) \n\
(arg init/1 0 cell/3 1 2/0) \n\
(arg init/1 0 cell/3 1 3/0) \n\
(arg init/1 0 cell/3 2 b/0) \n\
(arg init/1 0 control/1 0 xplayer/0) \n\
(arg legal/2 0 oplayer/0) \n\
(arg legal/2 0 xplayer/0) \n\
(arg legal/2 1 mark/2 0 1/0) \n\
(arg legal/2 1 mark/2 0 2/0) \n\
(arg legal/2 1 mark/2 0 3/0) \n\
(arg legal/2 1 mark/2 1 1/0) \n\
(arg legal/2 1 mark/2 1 2/0) \n\
(arg legal/2 1 mark/2 1 3/0) \n\
(arg legal/2 1 noop/0) \n\
(arg next/1 0 cell/3 0 1/0) \n\
(arg next/1 0 cell/3 0 2/0) \n\
(arg next/1 0 cell/3 0 3/0) \n\
(arg next/1 0 cell/3 1 1/0) \n\
(arg next/1 0 cell/3 1 2/0) \n\
(arg next/1 0 cell/3 1 3/0) \n\
(arg next/1 0 cell/3 2 b/0) \n\
(arg next/1 0 cell/3 2 o/0) \n\
(arg next/1 0 cell/3 2 x/0) \n\
(arg next/1 0 control/1 0 oplayer/0) \n\
(arg next/1 0 control/1 0 xplayer/0) \n\
(arg role/1 0 oplayer/0) \n\
(arg role/1 0 xplayer/0) \n\
(arg terminal/0) \n\
(arg true/1 0 cell/3 0 1/0) \n\
(arg true/1 0 cell/3 0 2/0) \n\
(arg true/1 0 cell/3 0 3/0) \n\
(arg true/1 0 cell/3 1 1/0) \n\
(arg true/1 0 cell/3 1 2/0) \n\
(arg true/1 0 cell/3 1 3/0) \n\
(arg true/1 0 cell/3 2 b/0) \n\
(arg true/1 0 cell/3 2 o/0) \n\
(arg true/1 0 cell/3 2 x/0) \n\
(arg true/1 0 control/1 0 oplayer/0) \n\
(arg true/1 0 control/1 0 xplayer/0) \n\
(arg column/2 0 1/0) \n\
(arg column/2 0 2/0) \n\
(arg column/2 0 3/0) \n\
(arg column/2 1 b/0) \n\
(arg column/2 1 o/0) \n\
(arg column/2 1 x/0) \n\
(arg diagonal/1 0 b/0) \n\
(arg diagonal/1 0 o/0) \n\
(arg diagonal/1 0 x/0) \n\
(arg line/1 0 b/0) \n\
(arg line/1 0 o/0) \n\
(arg line/1 0 x/0) \n\
(arg open/0) \n\
(arg row/2 0 1/0) \n\
(arg row/2 0 2/0) \n\
(arg row/2 0 3/0) \n\
(arg row/2 1 b/0) \n\
(arg row/2 1 o/0) \n\
(arg row/2 1 x/0) \n\
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



