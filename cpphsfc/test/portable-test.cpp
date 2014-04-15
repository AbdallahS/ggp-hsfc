//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
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
 * NOTE: send_state_across_games1 uses the deprecated functions.
 *       send_state_across_games2 is the preferred method.
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
 * Testing that the Players are the same across 2 instances
 * of the same game.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_players_across_games)
{
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    std::vector<Player> players1;
    std::vector<Player> players2;
    std::vector<Player> playerst;
    std::vector<PortablePlayer> pplayers1;
    std::vector<PortablePlayer> pplayerst;
    boost::unordered_set<std::string> playernames1;
    boost::unordered_set<std::string> playernames2;
    boost::unordered_set<std::string> playernamest;

    // Setup the games and get the player and player names.
    game1.players(players1);    
    game2.players(players2);    
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
    convert_to_portable(players1, pplayers1);
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
    convert_from_portable(pplayerst, game2, playerst);
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
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    State state1(game1);
    State state2(game2);
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
    state1.legals(playermoves1);
    state2.legals(playermoves2);
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
    convert_to_portable(moves1, pmoves1);
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
    convert_from_portable(pmoves2, game2, moves2);
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
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    State state1(game1);
    State state2(game2);
    std::vector<PlayerMove> playermoves1;
    std::vector<PlayerMove> playermoves2;
    std::vector<PlayerMove> playermovest;
    std::vector<PortablePlayerMove> pplayermoves1;
    std::vector<PortablePlayerMove> pplayermoves2;
    std::vector<PortablePlayerMove> pplayermovest;

    // Setup the games and get the move and move names.
    state1.legals(playermoves1);
    state2.legals(playermoves2);

    convert_to_portable(playermoves1, pplayermoves1);
    convert_to_portable(playermoves2, pplayermoves2);

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

    convert_from_portable(pplayermovest, game2, playermovest);
    BOOST_CHECK_EQUAL(playermoves1.size(), playermovest.size());
}

/****************************************************************
 * Testing that PlayerGoals can be serialized.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_playergoals_across_games)
{
    Game game1(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));
    State state1(game1);
    State state2(game2);
    std::vector<PlayerGoal> playergoals1;
    std::vector<PlayerGoal> playergoalst;
    std::vector<PortablePlayerGoal> pplayergoals1;
    std::vector<PortablePlayerGoal> pplayergoalst;

    // Setup the games get the playergoals after a playout
    state1.playout(playergoals1);
    BOOST_CHECK_EQUAL(playergoals1.size(), game1.numPlayers()); 
    
    convert_to_portable(playergoals1, pplayergoals1);
    BOOST_CHECK_EQUAL(playergoals1.size(), pplayergoals1.size()); 

    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pplayergoals1;
    std::string serialised(oserialstream.str());
    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);
    ia >> pplayergoalst;
    BOOST_CHECK(pplayergoals1 == pplayergoalst);

    convert_from_portable(pplayergoalst, game2, playergoalst);
    BOOST_CHECK_EQUAL(pplayergoalst.size(), playergoalst.size());
}



/****************************************************************
 ****************************************************************/


