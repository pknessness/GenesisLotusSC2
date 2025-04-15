// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov
//#undef byte
#define BACKWARD_HAS_BFD 1
#include "backward/backward.hpp"
#include "Bot.hpp"

#include <sc2api/sc2_coordinator.h>
#include <sc2api/sc2_gametypes.h>
#include <sc2utils/sc2_arg_parser.h>

#include <iostream>

//#define BUILD_FOR_LADDER

#ifdef BUILD_FOR_LADDER
namespace
{

struct Options
{
    Options(): GamePort(0), StartPort(0)
    {}

    int32_t GamePort;
    int32_t StartPort;
    std::string ServerAddress;
    std::string OpponentId;
};

void ParseArguments(int argc, char* argv[], Options* options_)
{
    sc2::ArgParser arg_parser(argv[0]);
    arg_parser.AddOptions(
        {
            {"-g", "--GamePort", "Port of client to connect to", false},
            {"-o", "--StartPort", "Starting server port", false},
            {"-l", "--LadderServer", "Ladder server address", false},
            {"-x", "--OpponentId", "PlayerId of opponent", false},
        });


    arg_parser.Parse(argc, argv);

    std::string GamePortStr;
    if (arg_parser.Get("GamePort", GamePortStr))
        options_->GamePort = atoi(GamePortStr.c_str());

    std::string StartPortStr;
    if (arg_parser.Get("StartPort", StartPortStr))
        options_->StartPort = atoi(StartPortStr.c_str());

    std::string OpponentId;
    if (arg_parser.Get("OpponentId", OpponentId))
        options_->OpponentId = OpponentId;

    arg_parser.Get("LadderServer", options_->ServerAddress);
}

}  // namespace

int main(int argc, char* argv[])
{
    backward::SignalHandling sh;

    Options Options;
    ParseArguments(argc, argv, &Options);

    printf("PARSED OPTIONS: GamePort:%d StartPort:%d OppID:%s Server:%s\n", Options.GamePort, Options.StartPort, Options.OpponentId.c_str(), Options.ServerAddress.c_str());

    sc2::Coordinator coordinator;
    Bot bot;

    // Add the custom bot, it will control the players.
    int num_agents = 2;
    coordinator.SetParticipants({
        CreateParticipant(sc2::Race::Protoss, &bot), });
    //bot.SetOpponentId(Options.OpponentId);

    // Request the feature layers to allow the use of the single drop micro
    sc2::FeatureLayerSettings settings;
    coordinator.SetFeatureLayers(settings);
    // Connect to the game client
    std::cout << "Connecting to port " << Options.GamePort << std::endl;
    coordinator.Connect(Options.GamePort);
    coordinator.SetupPorts(num_agents, Options.StartPort, false);
    // Set the unit selection policy
    // (if true, the selection will jump around everywhere so it can be harder to debug and doesn't allow a human to play at the same time)
    coordinator.SetRawAffectsSelection(true);
    // Join the already started game
    coordinator.JoinGame();
    coordinator.SetTimeoutMS(120000);	// 2 min
    std::cout << "Successfully joined game" << std::endl;
    // Step forward the game simulation.
    while (coordinator.Update()) {
    }

    return 0;
}

#else

int main(int argc, char* argv[])
{
    //backward::SignalHandling sh;
    srand(clock());
    sc2::Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    Bot bot;
    sc2::Difficulty diff = sc2::Difficulty::Easy;
    sc2::Race race = (sc2::Race)(std::rand() % 4);  // Race::Random;
    sc2::AIBuild build = sc2::AIBuild::RandomBuild;
    coordinator.SetParticipants({ CreateParticipant(sc2::Race::Protoss, &bot), CreateComputer(race, diff, build) });

    

    coordinator.LaunchStarcraft();
    //std::string maps[6] = { "5_13/Oceanborn513AIE.SC2Map",  "5_13/Equilibrium513AIE.SC2Map",
    //                       "5_13/GoldenAura513AIE.SC2Map", "5_13/Gresvan513AIE.SC2Map",
    //                       "5_13/HardLead513AIE.SC2Map",   "5_13/SiteDelta513AIE.SC2Map" };

    std::string maps[6] = { "AbyssalReefAIE.SC2Map",  "AcropolisAIE.SC2Map",
                            "AutomatonAIE.SC2Map", "EphemeronAIE.SC2Map",
                            "InterloperAIE.SC2Map",   "ThunderbirdAIE.SC2Map" };

    int r = std::rand() % 6;
    printf("rand %d [%d %d %d %d %d %d] %d\n", r, std::rand(), std::rand(), std::rand(), std::rand(), std::rand(),
        std::rand(), RAND_MAX);

    coordinator.StartGame(maps[r]);
    while (coordinator.Update()) {
    }

    return 0;
}

#endif
