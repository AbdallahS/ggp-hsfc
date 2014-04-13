//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

using namespace HSFC;

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
}

// Return the player 
Player get_player(const Game& game, const std::string& playername)
{
    std::vector<Player> players;
    game.players(players);
    BOOST_FOREACH(const Player& p, players)
    {
        if (p.tostring() == playername) return p;
    }
    BOOST_CHECK(false);
}

/****************************************************************
 * Testing that PortableState works across games. Load 2 games.
 * For a state in game 1 playout (so it is in a terminal state).
 * Then serialise and deserialise this into a state in game 2
 * and check that it is in a terminal state.
 ****************************************************************/

/****************************************************************
 * Testing that PortableState works across games. Load 2 games.
 * For a state in game 1 playout (so it is in a terminal state).
 * Then serialise and deserialise this into a state in game 2
 * and check that it is in a terminal state.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_state_across_games1)
{
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    State state1 = game1.initState();
    State state2 = game2.initState();
    BOOST_CHECK(!state1.isTerminal());
    BOOST_CHECK(!state2.isTerminal());

    std::vector<PlayerGoal> result;
    state1.playout(result);
    BOOST_CHECK(state1.isTerminal());

    boost::shared_ptr<PortableState> pstate1 = state1.CreatePortableState();
    boost::shared_ptr<PortableState> pstate2;

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pstate1;
    std::string serialised(oserialstream.str());
    
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pstate2;

    state2.LoadPortableState(*pstate2);
    BOOST_CHECK(state2.isTerminal());
}


BOOST_AUTO_TEST_CASE(send_state_across_games2)
{
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
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


    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    boost::shared_ptr<PortableState> pstate2;
    ia >> pstate2;
    State state2(game2, *pstate2);
    BOOST_CHECK(state2.isTerminal());
}


/****************************************************************
 ****************************************************************/


