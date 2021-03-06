/****************************************************************
 * Nineboard tictactoe seems to be causing problems, especially with
 * PortableX and throwing an error about the set of legals not having
 * moves for all players.
 *
 ****************************************************************/


//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

using namespace HSFC;

extern std::string g_nineboardttt;
extern std::string g_nineboardttt2;
extern std::string g_ttt;




/****************************************************************
 * Suport functions
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
 * Testing that PortableState works across games. Load 2 games.
 * For a state in game 1 playout (so it is in a terminal state).
 * Then serialise and deserialise this into a state in game 2
 * and check that it is in a terminal state.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_initial_state_across_game)
{
    Game game1(g_nineboardttt);
    Game game2(g_nineboardttt);
    const State& state1a = game1.initState();
    State state1b(game1);
    state1b.playout();
    BOOST_CHECK(!state1a.isTerminal());
    BOOST_CHECK(state1b.isTerminal());

    boost::shared_ptr<PortableState> pstate1a(new PortableState(state1a));
    boost::shared_ptr<PortableState> pstate1b(new PortableState(state1b));
    boost::shared_ptr<PortableState> pstate2a;
    boost::shared_ptr<PortableState> pstate2b;

    State state2a(game1, *pstate1a);

    // Transfer the initial state (pstate1a -> pstate2a)
    std::ostringstream oserialstream1;
    boost::archive::text_oarchive oa1(oserialstream1);
    oa1 << pstate1a;
    std::string serialised1(oserialstream1.str());

    std::istringstream iserialstream1(serialised1);
    boost::archive::text_iarchive ia1(iserialstream1);
    ia1 >> pstate2a;

    // Transfer the terminal state (pstate1b -> pstate2b)
    std::ostringstream oserialstream2;
    boost::archive::text_oarchive oa2(oserialstream2);
    oa2 << pstate1b;
    std::string serialised2(oserialstream2.str());

    std::istringstream iserialstream2(serialised2);
    boost::archive::text_iarchive ia2(iserialstream2);
    ia2 >> pstate2b;

    // Check the the serialization works
    BOOST_CHECK(*pstate1a == *pstate2a);
    BOOST_CHECK(*pstate1b == *pstate2b);

    State state2b(game2, *pstate1b);

    // Check that the terminal status is the same
    BOOST_CHECK(!state2a.isTerminal());
    BOOST_CHECK(state2b.isTerminal());

}


/****************************************************************
 * Nine board tictictoe GDL. Note: it has already been converted
 * using gadelac so that we don't need to rely on gadelac being
 * present for the unit test to work.
 ****************************************************************/

std::string g_nineboardttt=" \n\
;;;; RULES \n\
 \n\
(init (control xPlayer)) \n\
(role oPlayer) \n\
(role xPlayer) \n\
(index 1) \n\
(index 2) \n\
(index 3) \n\
(<= (legal xPlayer noop) (true (control oPlayer))) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) firstMove (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) (true (currentBoard ?i ?j)) (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) currentBoardClosed (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer noop) (true (control xPlayer))) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) firstMove (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) (true (currentBoard ?i ?j)) (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) currentBoardClosed (emptyCell ?i ?j ?k ?l)) \n\
(<= (next (mark ?i ?j ?k ?l ?mark)) (does ?player (play ?i ?j ?k ?l ?mark))) \n\
(<= (next (mark ?i ?j ?k ?l ?mark)) (true (mark ?i ?j ?k ?l ?mark))) \n\
(<= (next (currentBoard ?k ?l)) (does ?player (play ?i ?j ?k ?l ?mark))) \n\
(<= (next (control xPlayer)) (true (control oPlayer))) \n\
(<= (next (control oPlayer)) (true (control xPlayer))) \n\
(<= (goal xPlayer 0) (not (line x)) (not (line o)) open) \n\
(<= (goal xPlayer 100) (line x)) \n\
(<= (goal xPlayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal xPlayer 0) (line o)) \n\
(<= (goal oPlayer 0) (not (line x)) (not (line o)) open) \n\
(<= (goal oPlayer 100) (line o)) \n\
(<= (goal oPlayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal oPlayer 0) (line x)) \n\
(<= terminal (line x)) \n\
(<= terminal (line o)) \n\
(<= terminal (not open)) \n\
(<= (row ?i ?j ?k ?mark) (true (mark ?i ?j ?k 1 ?mark)) (true (mark ?i ?j ?k 2 ?mark)) (true (mark ?i ?j ?k 3 ?mark))) \n\
(<= (col ?i ?j ?k ?mark) (true (mark ?i ?j 1 ?k ?mark)) (true (mark ?i ?j 2 ?k ?mark)) (true (mark ?i ?j 3 ?k ?mark))) \n\
(<= (diag ?i ?j ?mark) (true (mark ?i ?j 1 1 ?mark)) (true (mark ?i ?j 2 2 ?mark)) (true (mark ?i ?j 3 3 ?mark))) \n\
(<= (diag ?i ?j ?mark) (true (mark ?i ?j 1 3 ?mark)) (true (mark ?i ?j 2 2 ?mark)) (true (mark ?i ?j 3 1 ?mark))) \n\
(<= (line ?mark) (index ?i) (index ?j) (index ?k) (row ?i ?j ?k ?mark)) \n\
(<= (line ?mark) (index ?i) (index ?j) (index ?k) (col ?i ?j ?k ?mark)) \n\
(<= (line ?mark) (index ?i) (index ?j) (diag ?i ?j ?mark)) \n\
(<= (emptyCell ?i ?j ?k ?l) (index ?i) (index ?j) (index ?k) (index ?l) (not (true (mark ?i ?j ?k ?l x))) (not (true (mark ?i ?j ?k ?l o)))) \n\
(<= open (emptyCell ?i ?j ?k ?l)) \n\
(<= currentBoardClosed (true (currentBoard ?i ?j)) (not (emptyCell ?i ?j 1 1)) (not (emptyCell ?i ?j 1 2)) (not (emptyCell ?i ?j 1 3)) (not (emptyCell ?i ?j 2 1)) (not (emptyCell ?i ?j 2 2)) (not (emptyCell ?i ?j 2 3)) (not (emptyCell ?i ?j 3 1)) (not (emptyCell ?i ?j 3 2)) (not (emptyCell ?i ?j 3 3))) \n\
(<= firstMove (not (true (currentBoard 1 1))) (not (true (currentBoard 1 2))) (not (true (currentBoard 1 3))) (not (true (currentBoard 2 1))) (not (true (currentBoard 2 2))) (not (true (currentBoard 2 3))) (not (true (currentBoard 3 1))) (not (true (currentBoard 3 2))) (not (true (currentBoard 3 3)))) \n\
 \n\
;;;; STRATS \n\
 \n\
(strat 0 true/1) \n\
(strat 1 index/1) \n\
(strat 2 emptyCell/4) \n\
(strat 3 currentBoardClosed/0) \n\
(strat 4 firstMove/0) \n\
(strat 5 legal/2) \n\
(strat 6 col/4) \n\
(strat 7 diag/3) \n\
(strat 8 row/4) \n\
(strat 9 line/1) \n\
(strat 10 open/0) \n\
(strat 11 terminal/0) \n\
(strat 12 does/2) \n\
(strat 13 goal/2) \n\
(strat 14 init/1) \n\
(strat 15 next/1) \n\
(strat 16 role/1) \n\
 \n\
;;;; PATHS \n\
 \n\
(arg terminal/0) \n\
(arg currentBoardClosed/0) \n\
(arg firstMove/0) \n\
(arg open/0) \n\
(arg does/2 0 oPlayer/0) \n\
(arg does/2 0 xPlayer/0) \n\
(arg does/2 1 noop/0) \n\
(arg does/2 1 play/5 0 1/0) \n\
(arg does/2 1 play/5 0 2/0) \n\
(arg does/2 1 play/5 0 3/0) \n\
(arg does/2 1 play/5 1 1/0) \n\
(arg does/2 1 play/5 1 2/0) \n\
(arg does/2 1 play/5 1 3/0) \n\
(arg does/2 1 play/5 2 1/0) \n\
(arg does/2 1 play/5 2 2/0) \n\
(arg does/2 1 play/5 2 3/0) \n\
(arg does/2 1 play/5 3 1/0) \n\
(arg does/2 1 play/5 3 2/0) \n\
(arg does/2 1 play/5 3 3/0) \n\
(arg does/2 1 play/5 4 o/0) \n\
(arg does/2 1 play/5 4 x/0) \n\
(arg goal/2 0 oPlayer/0) \n\
(arg goal/2 0 xPlayer/0) \n\
(arg goal/2 1 0/0) \n\
(arg goal/2 1 100/0) \n\
(arg goal/2 1 50/0) \n\
(arg init/1 0 control/1 0 xPlayer/0) \n\
(arg legal/2 0 oPlayer/0) \n\
(arg legal/2 0 xPlayer/0) \n\
(arg legal/2 1 noop/0) \n\
(arg legal/2 1 play/5 0 1/0) \n\
(arg legal/2 1 play/5 0 2/0) \n\
(arg legal/2 1 play/5 0 3/0) \n\
(arg legal/2 1 play/5 1 1/0) \n\
(arg legal/2 1 play/5 1 2/0) \n\
(arg legal/2 1 play/5 1 3/0) \n\
(arg legal/2 1 play/5 2 1/0) \n\
(arg legal/2 1 play/5 2 2/0) \n\
(arg legal/2 1 play/5 2 3/0) \n\
(arg legal/2 1 play/5 3 1/0) \n\
(arg legal/2 1 play/5 3 2/0) \n\
(arg legal/2 1 play/5 3 3/0) \n\
(arg legal/2 1 play/5 4 o/0) \n\
(arg legal/2 1 play/5 4 x/0) \n\
(arg next/1 0 control/1 0 oPlayer/0) \n\
(arg next/1 0 control/1 0 xPlayer/0) \n\
(arg next/1 0 currentBoard/2 0 1/0) \n\
(arg next/1 0 currentBoard/2 0 2/0) \n\
(arg next/1 0 currentBoard/2 0 3/0) \n\
(arg next/1 0 currentBoard/2 1 1/0) \n\
(arg next/1 0 currentBoard/2 1 2/0) \n\
(arg next/1 0 currentBoard/2 1 3/0) \n\
(arg next/1 0 mark/5 0 1/0) \n\
(arg next/1 0 mark/5 0 2/0) \n\
(arg next/1 0 mark/5 0 3/0) \n\
(arg next/1 0 mark/5 1 1/0) \n\
(arg next/1 0 mark/5 1 2/0) \n\
(arg next/1 0 mark/5 1 3/0) \n\
(arg next/1 0 mark/5 2 1/0) \n\
(arg next/1 0 mark/5 2 2/0) \n\
(arg next/1 0 mark/5 2 3/0) \n\
(arg next/1 0 mark/5 3 1/0) \n\
(arg next/1 0 mark/5 3 2/0) \n\
(arg next/1 0 mark/5 3 3/0) \n\
(arg next/1 0 mark/5 4 o/0) \n\
(arg next/1 0 mark/5 4 x/0) \n\
(arg role/1 0 oPlayer/0) \n\
(arg role/1 0 xPlayer/0) \n\
(arg true/1 0 control/1 0 oPlayer/0) \n\
(arg true/1 0 control/1 0 xPlayer/0) \n\
(arg true/1 0 currentBoard/2 0 1/0) \n\
(arg true/1 0 currentBoard/2 0 2/0) \n\
(arg true/1 0 currentBoard/2 0 3/0) \n\
(arg true/1 0 currentBoard/2 1 1/0) \n\
(arg true/1 0 currentBoard/2 1 2/0) \n\
(arg true/1 0 currentBoard/2 1 3/0) \n\
(arg true/1 0 mark/5 0 1/0) \n\
(arg true/1 0 mark/5 0 2/0) \n\
(arg true/1 0 mark/5 0 3/0) \n\
(arg true/1 0 mark/5 1 1/0) \n\
(arg true/1 0 mark/5 1 2/0) \n\
(arg true/1 0 mark/5 1 3/0) \n\
(arg true/1 0 mark/5 2 1/0) \n\
(arg true/1 0 mark/5 2 2/0) \n\
(arg true/1 0 mark/5 2 3/0) \n\
(arg true/1 0 mark/5 3 1/0) \n\
(arg true/1 0 mark/5 3 2/0) \n\
(arg true/1 0 mark/5 3 3/0) \n\
(arg true/1 0 mark/5 4 o/0) \n\
(arg true/1 0 mark/5 4 x/0) \n\
(arg col/4 0 1/0) \n\
(arg col/4 0 2/0) \n\
(arg col/4 0 3/0) \n\
(arg col/4 1 1/0) \n\
(arg col/4 1 2/0) \n\
(arg col/4 1 3/0) \n\
(arg col/4 2 1/0) \n\
(arg col/4 2 2/0) \n\
(arg col/4 2 3/0) \n\
(arg col/4 3 o/0) \n\
(arg col/4 3 x/0) \n\
(arg diag/3 0 1/0) \n\
(arg diag/3 0 2/0) \n\
(arg diag/3 0 3/0) \n\
(arg diag/3 1 1/0) \n\
(arg diag/3 1 2/0) \n\
(arg diag/3 1 3/0) \n\
(arg diag/3 2 o/0) \n\
(arg diag/3 2 x/0) \n\
(arg emptyCell/4 0 1/0) \n\
(arg emptyCell/4 0 2/0) \n\
(arg emptyCell/4 0 3/0) \n\
(arg emptyCell/4 1 1/0) \n\
(arg emptyCell/4 1 2/0) \n\
(arg emptyCell/4 1 3/0) \n\
(arg emptyCell/4 2 1/0) \n\
(arg emptyCell/4 2 2/0) \n\
(arg emptyCell/4 2 3/0) \n\
(arg emptyCell/4 3 1/0) \n\
(arg emptyCell/4 3 2/0) \n\
(arg emptyCell/4 3 3/0) \n\
(arg index/1 0 1/0) \n\
(arg index/1 0 2/0) \n\
(arg index/1 0 3/0) \n\
(arg line/1 0 o/0) \n\
(arg line/1 0 x/0) \n\
(arg row/4 0 1/0) \n\
(arg row/4 0 2/0) \n\
(arg row/4 0 3/0) \n\
(arg row/4 1 1/0) \n\
(arg row/4 1 2/0) \n\
(arg row/4 1 3/0) \n\
(arg row/4 2 1/0) \n\
(arg row/4 2 2/0) \n\
(arg row/4 2 3/0) \n\
(arg row/4 3 o/0) \n\
(arg row/4 3 x/0) \n\
";

std::string g_nineboardttt2=" \n\
(init (control xPlayer)) \n\
(role oPlayer) \n\
(role xPlayer) \n\
(index 1) \n\
(index 2) \n\
(index 3) \n\
(<= (legal xPlayer noop) (true (control oPlayer))) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) firstMove (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) (true (currentBoard ?i ?j)) (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal xPlayer (play ?i ?j ?k ?l x)) (true (control xPlayer)) currentBoardClosed (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer noop) (true (control xPlayer))) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) firstMove (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) (true (currentBoard ?i ?j)) (emptyCell ?i ?j ?k ?l)) \n\
(<= (legal oPlayer (play ?i ?j ?k ?l o)) (true (control oPlayer)) currentBoardClosed (emptyCell ?i ?j ?k ?l)) \n\
(<= (next (mark ?i ?j ?k ?l ?mark)) (does ?player (play ?i ?j ?k ?l ?mark))) \n\
(<= (next (mark ?i ?j ?k ?l ?mark)) (true (mark ?i ?j ?k ?l ?mark))) \n\
(<= (next (currentBoard ?k ?l)) (does ?player (play ?i ?j ?k ?l ?mark))) \n\
(<= (next (control xPlayer)) (true (control oPlayer))) \n\
(<= (next (control oPlayer)) (true (control xPlayer))) \n\
(<= (goal xPlayer 0) (not (line x)) (not (line o)) open) \n\
(<= (goal xPlayer 100) (line x)) \n\
(<= (goal xPlayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal xPlayer 0) (line o)) \n\
(<= (goal oPlayer 0) (not (line x)) (not (line o)) open) \n\
(<= (goal oPlayer 100) (line o)) \n\
(<= (goal oPlayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal oPlayer 0) (line x)) \n\
(<= terminal (line x)) \n\
(<= terminal (line o)) \n\
(<= terminal (not open)) \n\
(<= (row ?i ?j ?k ?mark) (true (mark ?i ?j ?k 1 ?mark)) (true (mark ?i ?j ?k 2 ?mark)) (true (mark ?i ?j ?k 3 ?mark))) \n\
(<= (col ?i ?j ?k ?mark) (true (mark ?i ?j 1 ?k ?mark)) (true (mark ?i ?j 2 ?k ?mark)) (true (mark ?i ?j 3 ?k ?mark))) \n\
(<= (diag ?i ?j ?mark) (true (mark ?i ?j 1 1 ?mark)) (true (mark ?i ?j 2 2 ?mark)) (true (mark ?i ?j 3 3 ?mark))) \n\
(<= (diag ?i ?j ?mark) (true (mark ?i ?j 1 3 ?mark)) (true (mark ?i ?j 2 2 ?mark)) (true (mark ?i ?j 3 1 ?mark))) \n\
(<= (line ?mark) (index ?i) (index ?j) (index ?k) (row ?i ?j ?k ?mark)) \n\
(<= (line ?mark) (index ?i) (index ?j) (index ?k) (col ?i ?j ?k ?mark)) \n\
(<= (line ?mark) (index ?i) (index ?j) (diag ?i ?j ?mark)) \n\
(<= (emptyCell ?i ?j ?k ?l) (index ?i) (index ?j) (index ?k) (index ?l) (not (true (mark ?i ?j ?k ?l x))) (not (true (mark ?i ?j ?k ?l o)))) \n\
(<= open (emptyCell ?i ?j ?k ?l)) \n\
(<= currentBoardClosed (true (currentBoard ?i ?j)) (not (emptyCell ?i ?j 1 1)) (not (emptyCell ?i ?j 1 2)) (not (emptyCell ?i ?j 1 3)) (not (emptyCell ?i ?j 2 1)) (not (emptyCell ?i ?j 2 2)) (not (emptyCell ?i ?j 2 3)) (not (emptyCell ?i ?j 3 1)) (not (emptyCell ?i ?j 3 2)) (not (emptyCell ?i ?j 3 3))) \n\
(<= firstMove (not (true (currentBoard 1 1))) (not (true (currentBoard 1 2))) (not (true (currentBoard 1 3))) (not (true (currentBoard 2 1))) (not (true (currentBoard 2 2))) (not (true (currentBoard 2 3))) (not (true (currentBoard 3 1))) (not (true (currentBoard 3 2))) (not (true (currentBoard 3 3)))) \n\
";

std::string g_ttt=" \n\
;;;; RULES \n\
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
;;;; STRATS \n\
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
;;;; PATHS \n\
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
