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

// Declare the tictactoe gdl. Defined at the end of the file.
extern const char* g_tictactoe;

/****************************************************************
 * Suport functions
 ****************************************************************/

// Count the number of moves that are legal for the given player
unsigned int count_player_moves(const std::vector<PlayerMove> moves, 
                                const std::string& player)
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

BOOST_AUTO_TEST_CASE(send_terminal_state_across_game1)
{
    Game game1(g_tictactoe);
    State state1(game1);
    state1.playout();
    BOOST_CHECK(state1.isTerminal());
    boost::shared_ptr<PortableState> pstate1(new PortableState(state1));

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pstate1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);


    Game game2(g_tictactoe);
    boost::shared_ptr<PortableState> pstate2;
    ia >> pstate2;
    State state2(game2, *pstate2);
    BOOST_CHECK(state2.isTerminal());
}


BOOST_AUTO_TEST_CASE(send_state_across_game1)
{
    Game game1(g_tictactoe);
    State state1(game1);
    BOOST_CHECK(!state1.isTerminal());
    boost::shared_ptr<PortableState> pstate1(new PortableState(state1));

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pstate1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);


    Game game2(g_tictactoe);
    boost::shared_ptr<PortableState> pstate2;
    ia >> pstate2;
    State state2(game2, *pstate2);
    BOOST_CHECK(!state2.isTerminal());
}

/****************************************************************
 * Testing that converting back and forth between state and
 * portable state preserves the number of joint moves.
 ****************************************************************/
BOOST_AUTO_TEST_CASE(send_state_across_game2)
{
    Game game1(g_tictactoe);
    State state1(game1);
    State state1b(game1);
    PortableState pstate1(state1);
    PortableState pstate1b(state1b);
    State state2(game1, pstate1);

    BOOST_CHECK(state1.isTerminal() == state2.isTerminal());
    BOOST_CHECK(pstate1 == pstate1b);

    std::vector<PlayerMove> legals1;
    std::vector<PlayerMove> legals2;
    state1.legals(std::back_inserter(legals1));
    state2.legals(std::back_inserter(legals2));
    BOOST_CHECK(legals1.size() == legals2.size());

    BOOST_CHECK(state1.joints().size() == state2.joints().size());
}

/****************************************************************
 * Testing that the Players are the same across 2 instances
 * of the same game.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_players_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    std::vector<Player> players1;
    std::vector<Player> players2;
    std::vector<Player> playerst;
    std::vector<PortablePlayer> pplayers1;
    std::vector<PortablePlayer> pplayerst;
    boost::unordered_set<std::string> playernames1;
    boost::unordered_set<std::string> playernames2;
    boost::unordered_set<std::string> playernamest;

    // Setup the games and get the player and player names.
    players1 = game1.players();
    players2 = game2.players();
    BOOST_CHECK_EQUAL(players1.size(), players2.size());
    BOOST_FOREACH(const Player& p, players1)
    {
        playernames1.insert(p.tostring());
    }
    BOOST_FOREACH(const Player& p, players2)
    {
        playernames2.insert(p.tostring());
    }
    BOOST_CHECK(playernames1 == playernames2);

    // Now do the important stuff.
    // Serialize a vector of PortableStates
    std::copy(players1.begin(), players1.end(),
              std::inserter(pplayers1, pplayers1.begin()));

    BOOST_CHECK_EQUAL(players1.size(), pplayers1.size());

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pplayers1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pplayerst;
    BOOST_CHECK_EQUAL(pplayers1.size(), pplayerst.size());

    // Deserialize vector of PortableStates

    std::transform(pplayerst.begin(), pplayerst.end(),
                   std::inserter(playerst, playerst.begin()),
                   FromPortable(game2));

    BOOST_CHECK_EQUAL(pplayerst.size(), playerst.size());

    BOOST_FOREACH(const Player& p, playerst)
    {
        playernamest.insert(p.tostring());
    }

    BOOST_CHECK(playernames2 == playernamest);
}


/****************************************************************
 * Testing that the Moves are the same across 2 instances
 * of the same game.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_moves_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    State state1(game1);
    State state2(game2);
    BOOST_CHECK(state1.isTerminal() == state2.isTerminal());

    std::vector<PlayerMove> playermoves1;
    std::vector<PlayerMove> playermoves2;
    std::vector<Move> moves1;
    std::vector<Move> moves2;
    std::vector<PortableMove> pmoves1;
    std::vector<PortableMove> pmoves2;
    boost::unordered_set<std::string> movenames1;
    boost::unordered_set<std::string> movenames2;
    boost::unordered_set<std::string> movenamest;

    // Setup the games and get the move and move names.
    state1.legals(std::back_inserter(playermoves1));
    state2.legals(std::back_inserter(playermoves2));
    BOOST_FOREACH(const PlayerMove& pm, playermoves1)
    {
        moves1.push_back(pm.second);
        movenames1.insert(pm.second.tostring());
    }
    BOOST_FOREACH(const PlayerMove& pm, playermoves2)
    {
        movenames2.insert(pm.second.tostring());
    }
    BOOST_CHECK(movenames1 == movenames2);

    // Serialize a vector of portables
    std::copy(moves1.begin(), moves1.end(),
              std::inserter(pmoves1, pmoves1.begin()));
    BOOST_CHECK_EQUAL(moves1.size(), pmoves1.size());

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pmoves1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);

    ia >> pmoves2;
    BOOST_CHECK_EQUAL(pmoves1.size(), pmoves2.size());

    // Deserialize vector of PortableStates
    std::transform(pmoves2.begin(), pmoves2.end(),
                   std::inserter(moves2, moves2.begin()),
                   FromPortable(game2));

    BOOST_CHECK_EQUAL(pmoves2.size(), moves2.size());

    BOOST_FOREACH(const Move& p, moves2)
    {
        movenamest.insert(p.tostring());
    }

    BOOST_CHECK(movenamest == movenames2);
}

/****************************************************************
 * Testing that PlayerMoves are the same across 2 instances
 * of the same game.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_playermoves_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    State state1(game1);
    State state2(game2);
    std::vector<PlayerMove> playermoves1;
    std::vector<PlayerMove> playermoves2;
    std::vector<PlayerMove> playermovest;
    std::vector<PortablePlayerMove> pplayermoves1;
    std::vector<PortablePlayerMove> pplayermoves2;
    std::vector<PortablePlayerMove> pplayermovest;

    // Setup the games and get the move and move names.
    state1.legals(std::back_inserter(playermoves1));
    state2.legals(std::back_inserter(playermoves2));

    std::copy(playermoves1.begin(), playermoves1.end(),
              std::inserter(pplayermoves1, pplayermoves1.begin()));
    std::copy(playermoves2.begin(), playermoves2.end(),
              std::inserter(pplayermoves2, pplayermoves2.begin()));

    BOOST_CHECK_EQUAL(playermoves1.size(), pplayermoves1.size());
    BOOST_CHECK_EQUAL(playermoves2.size(), pplayermoves2.size());
    BOOST_CHECK_EQUAL(pplayermoves1.size(), pplayermoves2.size());

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pplayermoves1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pplayermovest;
    BOOST_CHECK(pplayermoves1 == pplayermovest);

    std::transform(pplayermovest.begin(), pplayermovest.end(),
                   std::inserter(playermovest, playermovest.begin()),
                   FromPortable(game2));

    BOOST_CHECK_EQUAL(playermoves1.size(), playermovest.size());
}

/****************************************************************
 * Testing that PlayerGoals can be serialized.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_playergoals_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    State state1(game1);
    State state2(game2);
    std::vector<PlayerGoal> playergoals1;
    std::vector<PlayerGoal> playergoalst;
    std::vector<PortablePlayerGoal> pplayergoals1;
    std::vector<PortablePlayerGoal> pplayergoalst;

    // Setup the games get the playergoals after a playout
    state1.playout(playergoals1);
    BOOST_CHECK_EQUAL(playergoals1.size(), game1.numPlayers());

    std::copy(playergoals1.begin(), playergoals1.end(),
              std::inserter(pplayergoals1, pplayergoals1.begin()));

    BOOST_CHECK_EQUAL(playergoals1.size(), pplayergoals1.size());

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pplayergoals1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pplayergoalst;
    BOOST_CHECK(pplayergoals1 == pplayergoalst);

    std::transform(pplayergoalst.begin(), pplayergoalst.end(),
                   std::inserter(playergoalst, playergoalst.begin()),
                   FromPortable(game2));
    BOOST_CHECK_EQUAL(pplayergoalst.size(), playergoalst.size());
}



/****************************************************************
 ****************************************************************/

/****************************************************************
 * Testing that PortableJointMove are the same across 2 instances
 * of the same game.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_jointmove_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    State state1(game1);
    State state2(game2);
    std::vector<JointMove> jointmoves1;
    std::vector<JointMove> jointmoves2;
    std::vector<JointMove> jointmovest;
    std::vector<PortableJointMove> pjointmoves1;
    std::vector<PortableJointMove> pjointmoves2;
    std::vector<PortableJointMove> pjointmovest;

    // Setup the games and get the move and move names.
    jointmoves1 = state1.joints();
    jointmoves2 = state2.joints();

    std::transform(jointmoves1.begin(), jointmoves1.end(),
                   std::inserter(pjointmoves1, pjointmoves1.begin()),
                   ToPortable());
    std::transform(jointmoves2.begin(), jointmoves2.end(),
                   std::inserter(pjointmoves2, pjointmoves2.begin()),
                   ToPortable());

    BOOST_CHECK_EQUAL(jointmoves1.size(), pjointmoves1.size());
    BOOST_CHECK_EQUAL(jointmoves2.size(), pjointmoves2.size());
    BOOST_CHECK(pjointmoves1 == pjointmoves2);

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pjointmoves1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pjointmovest;
    BOOST_CHECK(pjointmoves1 == pjointmovest);

    std::transform(pjointmovest.begin(), pjointmovest.end(),
                   std::back_inserter(jointmovest),
                   FromPortable(game2));
    BOOST_CHECK_EQUAL(jointmoves1.size(), jointmovest.size());
}

/****************************************************************
 * Testing that JointGoals can be serialized.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_jointgoals_across_games)
{
    Game game1(g_tictactoe);
    Game game2(g_tictactoe);
    State state1(game1);
    State state2(game2);
    std::vector<PlayerGoal> playergoals1;
    std::vector<PlayerGoal> playergoalst;
    std::vector<PortablePlayerGoal> pplayergoals1;
    std::vector<PortablePlayerGoal> pplayergoalst;

    // Setup the games get the playergoals after a playout
    state1.playout(playergoals1);
    BOOST_CHECK_EQUAL(playergoals1.size(), game1.numPlayers());

    std::copy(playergoals1.begin(), playergoals1.end(),
              std::inserter(pplayergoals1, pplayergoals1.begin()));

    BOOST_CHECK_EQUAL(playergoals1.size(), pplayergoals1.size());

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pplayergoals1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pplayergoalst;
    BOOST_CHECK(pplayergoals1 == pplayergoalst);

    std::transform(pplayergoalst.begin(), pplayergoalst.end(),
                   std::inserter(playergoalst, playergoalst.begin()),
                   FromPortable(game2));
    BOOST_CHECK_EQUAL(pplayergoalst.size(), playergoalst.size());
}

/****************************************************************
 * Testing that PortableStates generated from different State objects
 * that correspond to the same state (ie., the fluents should be the
 * same).
 ****************************************************************/

PlayerMove get_playermove(const State& state,
                          const std::string& player,
                          const std::string& move)
{
    std::vector<PlayerMove> legs;
    state.legals(std::back_inserter(legs));

    BOOST_FOREACH(const PlayerMove& pm, legs)
    {
        std::string ply = pm.first.tostring();
        std::string mv = pm.second.tostring();
        if (ply == player && mv == move) return pm;
    }
    BOOST_CHECK(false);
    throw std::string("To prevent clang compiler warning");
}

BOOST_AUTO_TEST_CASE(generate_equiv_states)
{
    Game game1(g_tictactoe);
    State state1(game1);
    State state2(game1);
    JointMove jm;
    BOOST_CHECK(true);


    // Player the moves (x,1,1), (o,3,3), (x,3,1), (o,1,3)

    jm.insert(get_playermove(state1, "xplayer", "(mark 1 1)"));
    jm.insert(get_playermove(state1, "oplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state1.play(jm);
    jm.clear();

    jm.insert(get_playermove(state1, "oplayer", "(mark 3 3)"));
    jm.insert(get_playermove(state1, "xplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state1.play(jm);
    jm.clear();

    jm.insert(get_playermove(state1, "xplayer", "(mark 3 1)"));
    jm.insert(get_playermove(state1, "oplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state1.play(jm);
    jm.clear();

    jm.insert(get_playermove(state1, "oplayer", "(mark 1 3)"));
    jm.insert(get_playermove(state1, "xplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state1.play(jm);
    jm.clear();

    // Player the moves (x,3,1), (o,1,3), (x,1,1), (o,3,3)

    jm.insert(get_playermove(state2, "xplayer", "(mark 3 1)"));
    jm.insert(get_playermove(state2, "oplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state2.play(jm);
    jm.clear();

    jm.insert(get_playermove(state2, "oplayer", "(mark 1 3)"));
    jm.insert(get_playermove(state2, "xplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state2.play(jm);
    jm.clear();

    jm.insert(get_playermove(state2, "xplayer", "(mark 1 1)"));
    jm.insert(get_playermove(state2, "oplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state2.play(jm);
    jm.clear();

    jm.insert(get_playermove(state2, "oplayer", "(mark 3 3)"));
    jm.insert(get_playermove(state2, "xplayer", "noop"));
    BOOST_CHECK_EQUAL(jm.size(),2);
    state2.play(jm);
    jm.clear();

    PortableState pstate1(state1);
    PortableState pstate2(state2);

//    std::cout << "STATE1: " << std::endl << state1 << std::endl;
//    std::cout << "STATE2: " << std::endl << state2 << std::endl;

    // We want to now check that the pstates have the same fluents
    BOOST_CHECK(pstate1 == pstate2);
}

/****************************************************************
 * The GDL variables
 ****************************************************************/

const char* g_tictactoe = " \n\
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
